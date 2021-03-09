/*  netgen - autorouting net generator for Polish Format map
 *  Copyright (C) 2005 Mariusz Dąbrowski 
 *  vim:set wm=0 si:
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "RoadIDGenerator.h"
#include "netgen.h"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <cstdio>

using namespace std;

RoadIDGenerator::RoadIDGenerator (istream& istr, 
				  MapData *md,
	  			  ConfigReader& cfg): 
	PFMStreamReader (istr),
	mapData (md),
	config (cfg)	
{
   currentRoadID = 1;
   restrictionID = 0x7FFFFFFF;
   loopCounter = 0;
   removedCounter = 0;
}

// czy przeprowadzac rozbior Data0= dla poline. W zalozeniu
// obiekty nie podlegajace obrobce netgenem (rzeczki itp) maja
// i tak kopiowane co do znaku linie z danymi
#define isWorthParsing(type) ((isRoutable (type) || (type)==TYPE_BACKGROUND || \
          (type)==config.numbersType || config.isRestrictionOrRoadSign(type)))

void RoadIDGenerator::startRoad (){
   first = true;
   previousPoint = new Point;
   pointInRoad = 0;
   length = 0;
   if (!config.isCheckOnly || config.checkNetIntegrity) {
      pl = new Polyline;
      pl->label = label;
      pl->type = objectType;
      pl->endLevel = endLevel;
      pl->dirIndicator = dirIndicator;
      pl->unknown = sectionBuf;
      pl->routeParam = routeParam;
      pl->restrParam = restrParam;
      pl->roadsignType = roadsignType;
      pl->roadsignPos = roadsignPos;
      pl->roadsignAngle = roadsignAngle;
      if (config.isRestrictionOrRoadSign(objectType)){
	 pl->roadID = restrictionID;
      } else {
	 pl->roadID = currentRoadID;
      }
      pl->markerKind = markerKind;

      if (pl->routeParam.valid){
	 if (pl->routeParam.speed == 0 && 
	     pl->routeParam.route == 0 && 
	     config.overrideNullSpeed==1){
	    pl->routeParam.speed = config.typeParameters[pl->type].speed;
	    pl->routeParam.route = config.typeParameters[pl->type].route;
	 }
      }
      else {	 
	 pl->routeParam = config.typeParameters[pl->type];
	 pl->routeParam.oneWay = 0;
	 pl->routeParam.valid = true;
      }
      if (pl->dirIndicator == 1){
  	pl->routeParam.oneWay = 1;
      }
      if (forceClass >= 0){
	 pl->routeParam.route = forceClass;
      }
      if (forceSpeed >= 0){
	 pl->routeParam.speed = forceSpeed;
      }
   }
}

void RoadIDGenerator::writeToMapAndOutput (Point pn, bool last){

   if (*previousPoint == pn){
      fprintf (stderr, _("Information:"
			 " Road %6d. Removed duplicated point %d.\n"),
	       currentRoadID, pointInRoad);
      removedCounter++;
   }
   else {

#ifdef NUMBERING_REPORT      
      // Uwaga obliczanie odległości ze względu na operacje
      // zmiennoprzecinkowe może istotnie spowalniać działanie
      // programu
      if (!first)
	length = length + sqrt (pn.sqrDistance (*previousPoint));
#endif      

      // Do listy drog mozna zapisac czy last, first czy inny
      // i wykorzystac pozniej te informacje do obliczania
      // liczby linii dochodzacych do punktu
     
      if (config.isRestrictionOrRoadSign(objectType)){
	 if (config.createNodeForRestriction){
   	    mapData->pointmap [pn].flags |= FLAG_RESTRICTION;
	 }
      }
      else {

	 mapData->pointmap [pn].roads.push_back (
	 Road (currentRoadID, pointInRoad, length, (first | last)));

   	 if (first) {
   	    mapData->pointmap [pn].flags |= FLAG_ROAD_BEGIN;
   	    mapData->pointmap [pn].flags |= FLAG_HAS_EXIT;
   	    if (dirIndicator != 1){
   	       mapData->pointmap [pn].flags |= FLAG_HAS_ENTRANCE;
   	    }
      	 }
	 else {
	    if (!last){
	       mapData->pointmap [pn].flags |= (FLAG_HAS_EXIT | FLAG_HAS_ENTRANCE);
	    } else {
	       mapData->pointmap [pn].flags |= FLAG_ROAD_END;
	       mapData->pointmap [pn].flags |= FLAG_HAS_ENTRANCE;
	       if (dirIndicator != 1){
		  mapData->pointmap [pn].flags |= FLAG_HAS_EXIT;
	       }
	    }
	 }
      }
      pointInRoad++;
      if (!config.isCheckOnly || config.checkNetIntegrity) {
	 pl->points.push_back(pn);
      }
      first = false;
   }
   *previousPoint = pn;		
}

void RoadIDGenerator::endRoad (){
   if (!config.isCheckOnly || config.checkNetIntegrity) {
      if (config.isRestrictionOrRoadSign(objectType)){
   	 mapData->lines.insert(make_pair(restrictionID, *pl));
      	 mapData->sections.push_back(MapSection(restrictionID));
      } else {
   	 mapData->lines.insert(make_pair(currentRoadID, *pl));
      	 mapData->sections.push_back(MapSection(currentRoadID));
      }
      delete pl;
   }

   if (config.isRestrictionOrRoadSign(objectType))
     restrictionID--;
   else
     currentRoadID++;

   delete previousPoint;
}

// funkcja znajduje węzły w pojedynczej linii. 
// zwraca true jeśli jest jeden węzeł składający się z dwóch niesąsiednich
// punktów oraz zapisuje do n1 i n2 numery punktów, które go tworzą

bool RoadIDGenerator::findNode (Points& pts, int &n1, int &n2){
   // tworzenie listy unikalnych punktów
   Pointmap pointmap;
   int pointNum = 0;
   for (PointsIter pn = pts.begin(); pn != pts.end (); pn++){   
      pointmap [*pn].roads.push_back (Road (0, pointNum));
      pointNum++;
   }
   // usuwanie punktów pojedynczych
   PointmapIter pm = pointmap.begin ();
   while (pm != pointmap.end ()){
      if (pm->second.roads.size() < 2){
	 PointmapIter pv = pm;
	 pm++;
	 pointmap.erase (pv);
      }
      else 
	pm++;
   }
   // pozostały tylko węzły
   if (pointmap.size () == 1){ // jeden węzeł
      if (pointmap.begin()->second.roads.size() == 2){ // zawiera dwa punkty
	 RoadlistIter ri = pointmap.begin()->second.roads.begin();
	 n1 = ri->node;
	 ri ++;
	 n2 = ri->node;
	 if (n2 - n1 > 1){
  	    return true;
	 }
      }
   }
   return false;
}

void RoadIDGenerator::outputSegment (PointsIter& pn, int segmentSize){
//   cerr << "Segment długości: " << segmentSize << endl;
   if (segmentSize > 1){
      startRoad ();
      for (int i = 0; i < segmentSize; i++){
	 bool last = (i == segmentSize - 1);
	 writeToMapAndOutput (*pn++, last);
      }	    
      endRoad ();
      pn--;
   }
}

// funkcja zwraca zaokrągloną w górę średnią arytmetyczną parametrów

int RoadIDGenerator::half (int m, int n){
   return (int) ceil ((double)(m+n) / 2);
}

//

void RoadIDGenerator::writeAllToMap (unsigned char flags){
   list<Points>::iterator da = datalist.begin ();
   while (da != datalist.end()){
      PointsIter pn;
      pn = da->begin();
      while (pn != da->end()){
	 mapData->pointmap [*pn].flags |= flags;
	 pn++;
      }
      da++;
   }
}

void RoadIDGenerator::checkMinAngle(Line* line){
   static double previousSlope;
   if (!line) {
     previousSlope = -20; // invalid, no previous line
     return;
   }

   double slope;
   if (line->vertical){			
      if (line->p.y < line->k.y){
         slope = M_PI/2;
      } else {
         slope = -M_PI/2;
      }
   } else {			   
      double coslat = cos (line->p.x/180*M_PI);			  
      double tgalfa = coslat * 
   	   (line->p.y - line->k.y)/
   	   (line->p.x - line->k.x);
      slope = atan (tgalfa);
   }
   if (line->p.x > line->k.x){
      if (line->p.y < line->k.y){
         slope += M_PI;
      } else {
         slope -= M_PI;
      }
   }

   if (previousSlope > -10.0){ // previous slope valid
      double angle = fabs (previousSlope - slope);
      if (angle > M_PI){
         angle = (2 * M_PI) - angle;
      }
      if (angle /M_PI*180 < config.minAngle){
         cerr << "Warning: at point " << line->p;
         cerr << " angle between segments: ";
         cerr << setprecision (1) << angle/M_PI*180;
         cerr << " degrees";
         cerr << setprecision(config.precision) << endl;
         mapData->acuteAngles.push_back(line->p);
   
      }  
   }	

   if (slope < 0)
     previousSlope = slope + M_PI;
   else 
     previousSlope = slope - M_PI;
}

void RoadIDGenerator::section (const string& sec){
   PFMStreamReader::section (sec);

   if (isPolyline){
      mapData->sections.push_back(sectionBuf);      
      sectionHeader = "[" + sec + "]\n";
      sectionBuf.clear();
      return;
   }  

   if (isImgID || isPolygon || isPoint){
      mapData->sections.push_back(sectionBuf);      
      sectionBuf = "[" + sec + "]\n";
      return;
   }

   if (insideImgID && isImgIDEnd){
      if (routing == -1)
	sectionBuf += "Routing=Y\n";      
      insideImgID = false;

      sectionBuf += "[" + sec + "]\n";
      mapData->sections.push_back(sectionBuf);
      sectionBuf.clear();
      return;
   }

   if (insidePoint && isPointEnd){      
      insidePoint = false;
      if  (!config.isRemoved(objectType) &&
	   (objectType != config.noCrossingType)){
	 sectionBuf += "[" + sec + "]\n";
	 mapData->sections.push_back(sectionBuf);
      }
      sectionBuf.clear();
      return;
   }

   if (insidePolygon && isPolygonEnd){
      insidePolygon = false;
      if (objectType == TYPE_BACKGROUND){
	 fprintf(stderr, "Info: background type 0x%x found (%s @ %d)\n", TYPE_BACKGROUND, label.c_str(), lineNo());
 	 writeAllToMap (FLAG_MAP_BOUND);
      } 
      sectionBuf += "[" + sec + "]\n";
      mapData->sections.push_back(sectionBuf);
      sectionBuf.clear();
      return;
   }

   if (insidePolyline && isPolylineEnd){
      // fake background as line:
      if (objectType == TYPE_BACKGROUND){
//       fprintf(stderr, "Info: fake background LINE type 0x%x found (%s @ %d)\n", TYPE_BACKGROUND, label.c_str(), lineNo());
         writeAllToMap (FLAG_MAP_BOUND);
      }

      insidePolyline = false;
      if (isRoutable(objectType) || (config.isRestrictionOrRoadSign(objectType))){
       	 list<Points>::iterator da = datalist.begin ();
       	 while (da != datalist.end()){
       	    int n1, n2;
       	    if ((da->size() >=3) && findNode (*da, n1, n2) && (!config.isRestrictionOrRoadSign(objectType))){
	       // Podział pętli
	       fprintf (stderr, _("Information: Road %6d. Loop cut. (%s @ %d)\n"), 
	     		currentRoadID+1, label.c_str(), lineNo());
	       if (numbersFound){		    
		  cerr << _("Warning: Road with 'Numbers' parameter cut.\n");
	       }

	       int seg_size;
	       PointsIter pn = da->begin();
	       // od początku do węzła
	       seg_size = n1 + 1;		  
	       outputSegment (pn, seg_size);
	       // od węzła do połowy pętli
	       seg_size = half (n1, n2) - n1 + 1;		  
	       outputSegment (pn, seg_size);
	       // od połowy pętli do węzła
	       seg_size = n2 - half (n1, n2) + 1;
	       outputSegment (pn, seg_size);
	       // od węzła do końca
	       seg_size = da->size() - n2;
	       outputSegment (pn, seg_size);
	       loopCounter++;
	    }
	    else {
	       // droga nie zapętlona
	       PointsIter pn = da->begin();
	       outputSegment (pn, da->size());
	    }
	    da++;
	 }
      	 sectionBuf.clear();
	 return;
      }
      else {
	 if (objectType != TYPE_BACKGROUND){
	    sectionBuf = sectionHeader + sectionBuf;
	    sectionBuf += "[" + sec + "]\n";
	    mapData->sections.push_back(sectionBuf);
	 }
       	 sectionBuf.clear();
	 return;
      }
   }
   sectionBuf += "[" + sec + "]\n";
}

bool RoadIDGenerator::token (const string& tok, const string& val){

   bool recognized = PFMStreamReader::token (tok, val);
   
   if (insideImgID){
      if (isEqual (tok, "Routing")){      
	 sectionBuf += "Routing=Y\n";
	 routing = 1;
	 return true;
      }
   }      
  
   if (insidePolyline){       
      if (isEqual (tok, "RoadID") || isEqual (tok, "Nod")){
	 // Eliminujemy 
	 return true;
      }      
   }

   // nie od razy wiadomo jaki jest typ !
   if (!isPolyline || !(isRoutable(objectType) || (config.isRestrictionOrRoadSign(objectType))) || !recognized){
      sectionBuf += tok + "=" + val + "\n";
   }

   if (insidePoint){
      // Punkty nie podlegają modyfikacji w procesie obróbki, 
      // więc można całą sekcję zapisać jako tekst 
      if ( (objectType == config.noCrossingType || config.isRoadEnd(objectType))
            && (isEqual (tok, "Origin0") || isEqual (tok, "Data0")) ){
	 MaskingPoint p;
      	 istringstream iv (val);
	 iv >> p;
	 p.x = round (p.x * SCALE_FACTOR) / SCALE_FACTOR;
	 p.y = round (p.y * SCALE_FACTOR) / SCALE_FACTOR;
	 if (objectType == config.noCrossingType){
	    mapData->noCrossings.insert (make_pair(p,false));
	 }	    
	 if (config.isRoadEnd(objectType)){
	    mapData->roadEnds.insert (make_pair(p,false));
	 }	    
      }
      return true;
   }
   
   // jedynie BACKGROUND jest jakos przetwarzany i uzywa Points'ow
   // wiec lepiej nie uzywac drogiego parsowania punktow
   if (insidePolygon && objectType == TYPE_BACKGROUND){
      if (isEqual (tok,"Data0")){
	 datalist.push_back (Points());
	 datalist.back().addFromString (val);
      }            
   }
   
   if (insidePolyline){
      // W Celem zrobienia przetwarzania jednoprzebiegowego
      // należy wczytywać wszystkie poziomy a nie tylko Data0
      // Ponadto należy zapisywać z którego poziomu pochodzą dane
      if (isWorthParsing(objectType) && isEqual (tok,"Data0")){
	 datalist.push_back (Points());
	 datalist.back().addFromString (val);
	 if (objectType == config.numbersType){
	    Polyline pl;
	    pl.label = label;
	    //pl.type = objectType;
	    mapData->numbers.push_back(pl);
	    Point previousPoint; 
	    istringstream iv (val);
	    while (iv){
	       Point p;
	       iv >> p;
	       if (p.valid()){
	       	  mapData->numbers.back().points.push_back(p);
		  previousPoint = p;
	       }
	       iv.ignore (100,',');	 
	    }
	 }
#ifdef CREATE_LIST_OF_LINES
	 if (isRoutable (objectType)){
	    Point previousPoint; 
	    istringstream iv (val);
	    checkMinAngle(NULL); // initialize (clear previous slope)
	    while (iv){
	       Point p;
	       iv >> p;
	       if (p.valid()){
		  if (previousPoint.valid()){
		     mapData->lineList.push_back (Line(previousPoint, p));
		     if (config.minAngle > 0){
			checkMinAngle (&mapData->lineList.back());
		     }
		  }
		  previousPoint = p;
	       }
	       iv.ignore (100,',');	 
	    }
	 }
#endif	 
      }            
   }
}

void RoadIDGenerator::comment (const string& line){
   sectionBuf += line + "\n";
}

void RoadIDGenerator::process (){
   sectionBuf.clear();
   PFMStreamReader::process ();
   if (sectionBuf.length() > 0){
      mapData->sections.push_back(sectionBuf);
   }
   cerr << endl;
   cerr << _("Number of cut loops: ") << loopCounter << endl;
   cerr << _("Number of duplicated points removed: ") << removedCounter << endl;
   cerr << _("Number of roads: " ) << currentRoadID -1 << endl;
   cerr << _("Number of numbers definitions: " ) << mapData->numbers.size() << endl;
   cerr << endl;
}

