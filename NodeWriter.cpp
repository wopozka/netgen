/*  netgen - autorouting net generator for Polish Format map
 *  Copyright (C) 2005 Mariusz Dąbrowski 
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

#include "Polyline.h"
#include "NodeWriter.h"

#include <algorithm>
#include <set>
#include <vector>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <cstdio>

using namespace std;

double vangle( const Point& a1, const Point& a2,const Point& b1,const Point& b2 ){

// oblicza kat miedzy vektorem a1-a2 a b1-b2 w radianach
// cosinus to poprawka na szerokosc geograficzna
  double ax = a2.x - a1.x ;
  double ay = ( a2.y - a1.y ) * cos( deg2rad((a2.x + a1.x)/2 ));
  double bx = b2.x - b1.x ;
  double by = ( b2.y - b1.y ) * cos( deg2rad((b2.x + b1.x)/2 ));

// wektory zerowe
  if ( (ax == 0 && bx == 0 ) ||
       (ay == 0 && by == 0 ) )
    return 0;
//
// orignalny wzor
//    math.atan2 (a[1]*b[2]-a[2]*b[1],a[1]*b[1]+a[2]*b[2]) 
//
   return  atan2 ( ax*by-ay*bx, ax*bx+ay*by );
}

double normalize( double deg) { // normalizuje wartosc w stopniach do zakresu 0 -360
   double norm_angle;
   norm_angle = std::floor((deg+22.5)/45)*45;
   double a;
   a = (floor(norm_angle / 360))*360;
   return norm_angle-a;
}


int64_t NodeWriter::rSignHash(const Point& p){
   double dx=p.x*DEG2M/HASHGRIDSIZE;
   double dy=p.y*DEG2M/HASHGRIDSIZE*cos(deg2rad(rSignHashLatitudeBase));
//   cerr << "DX=" <<(int64_t)round(dx) << " DY= " <<(int64_t)round(dy)  <<endl;
   return ((int64_t)round(dx)+(int64_t)round(dy)*HASHSQSIZE);
}

bool NodeWriter::rSignPlaceOccupied(const Point& p){
   int64_t h=rSignHash(p);
   return(  
           (rSignHashes.find(h-1) != rSignHashes.end() )  ||
           (rSignHashes.find(h)   != rSignHashes.end() )  ||
           (rSignHashes.find(h+1) != rSignHashes.end() )  ||
           (rSignHashes.find(HASHSQSIZE+h-1) != rSignHashes.end() )  ||
           (rSignHashes.find(HASHSQSIZE+h)   != rSignHashes.end() )  ||
           (rSignHashes.find(HASHSQSIZE+h+1) != rSignHashes.end() )  ||
           (rSignHashes.find(-HASHSQSIZE+h-1) != rSignHashes.end() )  ||
           (rSignHashes.find(-HASHSQSIZE+h)   != rSignHashes.end() )  ||
           (rSignHashes.find(-HASHSQSIZE+h+1) != rSignHashes.end() )  
         );
}


void NodeWriter::addRSignHash(const Point& p) {
               rSignHashes.insert(rSignHash(p));
}
   

void NodeWriter::markNodesNearRoad (double radius){

   double sradius = radius * radius;

   // create map of end road nodes sorted by coordinates (Point)

   Pointmap pm;
   NodemapIter nd; 

   for (nd =  nodes.find (firstEndNodeID); nd != nodes.end (); nd++){
      PointParameters pp;
      pp.id = nd->first;	 
      pm.insert (make_pair(nd->second.p,pp));
   }

   list<Line>::iterator li;
   
   for (li = mapData.lineList.begin (); li != mapData.lineList.end(); li++){

      // compute coordinates of bounding rectangle for particular segment
      double lx = fmin (li->p.x, li->k.x) - radius;
      double ly = fmin (li->p.y, li->k.y) - radius;
      double ux = fmax (li->p.x, li->k.x) + radius;
      double uy = fmax (li->p.y, li->k.y) + radius;

      // for every point (end road node) inside rectangle
      // compute distance between point and line segment
      
      PointmapIter nd;
      PointmapIter ndEnd = pm.upper_bound (Point(ux,uy));
      for (nd = pm.lower_bound (Point(lx,ly)); nd != ndEnd ; nd++){
	 if ((nd->first.y >= ly) && (nd->first.y <= uy)){ 
	    // y inside rectangle
	    if ((! (li->p == nd->first)) && (! (li->k == nd->first))){
	       // line segment other than node belongs to
	       if (li->distanceFromPoint (nd->first) < sradius){
		  // distance lower than requested
		  nd->second.flags |= FLAG_NEAR_ROAD;
	       }
	    }
	 }
      }
   }

   // write flags back into new_ndes map

   for (PointmapIter nd =  pm.begin (); nd != pm.end (); nd++){
      if (nd->second.flags & FLAG_NEAR_ROAD){
	 nodes [nd->second.id].flags |= FLAG_NEAR_ROAD;
      }
   }

// cerr << "Number of Line::distanceFromPoint calls " << Line::numCalls << endl;
}


void NodeWriter::checkMinimumDistance (double epsilon){
   // sprawdzamy czy zachowane są minimalne odstępy nodów
   double sqr_epsilon = epsilon * epsilon;
   PointmapIter pn, pn_base;
   pn_base = points.begin ();
   while (pn_base != points.end()){
      pn = pn_base;
      pn++;
      while ((pn != points.end()) && 
	     (fabs(pn_base->first.x - pn->first.x)) < epsilon){
	 if (pn_base->first.sqrDistance (pn->first) < sqr_epsilon){
	    if (pn->second.id != pn_base->second.id){
	       fprintf (stderr, _("Warning: Node %d too close to node %d.\n"), 
			pn->second.id, pn_base->second.id);
	       nodes [pn->second.id].flags |= FLAG_CLOSE;
	       nodes [pn_base->second.id].flags |= FLAG_CLOSE;
	       warningCounter++;
	    }
	 }
	 pn++;
      }   
      pn_base++;
   }
}

NodeWriter::NodeWriter (ostream& ostr, 
			MapData& md,  
			ConfigReader& cfg): 
	os (ostr),
	mapData (md),
	points (md.pointmap),
	nodes  (md.nodes),
	config (cfg),
	streetNum (md)
{
   warningCounter = 0;
   endNodeCounter = 0;
   restrictionCounter = 0;
   bordernodeCounter = 0;
   double epsilon = config.epsilonMax;
   double sqr_epsilon = epsilon * epsilon;
   NodeID nodeID = 1;
   cerr << _("Number of points in roads: ") << points.size () << "\n";
   cerr << _("Number of road segments: ") << mapData.lineList.size () << "\n";

   NodeID newID;
   int stickedCounter = 0;
   PointmapIter pn;
   pn = points.begin ();
   while (pn != points.end()){
      pn->second.id = 0;
      pn ++;
   }
   PointmapIter pn_base = points.begin ();
   while (pn_base != points.end()){
      pn = pn_base; // nie porównujemy węzła ze sobą !
      pn++;
      bool neighbourFound = false;
      if (pn_base->second.id != 0){	 
 	 newID = pn_base->second.id;
      }
      else {
 	 newID = nodeID;
      }
      while ((pn != points.end()) && 
	     (fabs(pn_base->first.x - pn->first.x)) < epsilon){
	 if (pn_base->first.sqrDistance (pn->first) < sqr_epsilon){
	    if (pn->second.id == 0) {
	       pn->second.id = newID;
	       neighbourFound = true;
	       pn->second.flags |= FLAG_ALIGNED;
	    }
	 }
	 pn++;
      }   
      if (pn_base->second.id == 0){
	 // jeśli własny był zerem to możemy:
	 // nadać nowy jeśli nie było w otoczeniu innego niezerowego
	 // przejąć od innego z otoczenia
	 // tylko co zrobić w sytuacji gdy w otoczeniu są różne niezerowe
	 //
	 // Generalnie powinno być tak że jeśli w grupie jeden jest niezerowy 
	 // to grupa przyjmuje od niego id. Ale jak jest więcej niezerowych 
	 // to robi się problem. Są wtedy dwie (lub więcej) grup, które
	 // należałoby zwinąć do jednej.
	 // 
	 // 
	 if (neighbourFound || (pn_base->second.roads.size() > 1) ||
	     ((pn_base->second.roads.size() > 0) &&
	      ((pn_base->second.flags & FLAG_RESTRICTION) ||
	     (pn_base->second.flags & FLAG_MAP_BOUND)))){
	   pn_base->second.id = newID;
	   nodeID++;
	 }
      }
      pn_base++;
   }

   firstEndNodeID = nodeID;
   pn = points.begin (); 
   while (pn != points.end ()){
      if (pn->second.id != 0){
	 pn++;
      }
      else {
	 if ((config.nodeOnRoadEnd) && 
	     ((pn->second.flags & FLAG_ROAD_BEGIN) || 
	     (pn->second.flags & FLAG_ROAD_END))
	     ){
	    endNodeCounter++;
	    pn->second.id = nodeID++;
	    pn++;
	 } else {
	    PointmapIter pv = pn;
	    pn++;
	    points.erase (pv);
	 }
      }
   }

   // obliczamy położenie nowego węzła - średnia arytmetyczna każdej
   // ze współrzednych 
   for (pn = points.begin (); pn != points.end(); pn++){ 
      int n = 0;
      RoadlistIter ri;
      for (ri = pn->second.roads.begin (); ri != pn->second.roads.end(); ri++){
	 if (ri->endpoint){
	    n++;
	 } else {
	    n+=2; // jesli droga nie kończy się w punkcie liczony jest podwojnie	    
	 }
      }
      nodes [pn->second.id].p.x += pn->first.x * (double)n;
      nodes [pn->second.id].p.y += pn->first.y * (double)n;
      nodes [pn->second.id].n   += n;
      nodes [pn->second.id].flags |= pn->second.flags;
      nodes [pn->second.id].roads.insert (nodes[pn->second.id].roads.end(), 
					  pn->second.roads.begin(), 
					  pn->second.roads.end());
   }

   NodemapIter pnn = nodes.begin ();
   while (pnn != nodes.end()){
      if (pnn->second.n > 0){
	 pnn->second.p.x = pnn->second.p.x / (double)(pnn->second.n);
	 pnn->second.p.y = pnn->second.p.y / (double)(pnn->second.n);      
	 if (pnn->second.flags & FLAG_ALIGNED){
	    stickedCounter ++;
	 }
	 pnn++;
      } else {
	 NodemapIter tmp = pnn;
	 pnn++;
	 nodes.erase (tmp);
      }
   }

   cerr << _("Number of sticked nodes: ") << stickedCounter++ << "\n";

   cerr << _("Number of end-road nodes: ") << endNodeCounter  << endl;
   cerr << _("Number of nodes: ") << points.size () << "\n";

   // do roads trzeba przekazać flagę brzegu mapy
   // 
   // Dla szybkiego wyszukiwania wezłów należących do drogi tworzymy mapę 
   // identyfikatorów węzłów i numerów punktów poindeksowaną po numerach dróg
   RoadlistIter rd;
   for (pn = points.begin (); pn != points.end (); pn++){
      for (rd = pn->second.roads.begin (); rd != pn->second.roads.end(); rd++){
	 NodemapIter nnf = nodes.find (pn->second.id);
	 int flags;
	 if (nnf == nodes.end()){
	    flags = pn->second.flags;
	 }
	 else {
	    flags = nnf->second.flags;
	 }
	 roads.insert (make_pair (rd->id, 
				  RoadParameters (pn->second.id,
						  rd->node,
						  flags,
						  rd->length
						 )));
      }
   }

 //

  rSignHashLatitudeBase=nodes.begin()->second.p.x;
  cerr <<"rSignHashLatitudeBase=" << rSignHashLatitudeBase <<endl;
   //
   //proteza dla danych rSigns w cfg
   // nie chcial mi dzialac constructor kopiujacy
   // wiec kopiuje tu.
   //

   map<int,vecString>::iterator it;

   for (it = cfg.rSigns.begin(); it != cfg.rSigns.end() ; it ++){
        vecString vs(0);
        
        for( unsigned int t2=0 ; t2 < (*it).second.size() ; t2++)
             vs.push_back((*it).second[t2]);
        

        config.rSigns[(*it).first]=vs;
   }

  

}

int NodeWriter::getFirstEndNodeID (){
   return firstEndNodeID;
}

// Funkcja wyszukuje drogi przechodzące przez parę punktów

int NodeWriter::findRoad (Polyline& pl, PointParameters *na, PointParameters *nb){
   RoadlistIter rdA;
   RoadlistIter rdB;
   int roadCounter = 0;
   int roadID = -1;
   for (rdA = na->roads.begin(); rdA != na->roads.end(); rdA++){
      for (rdB = nb->roads.begin(); rdB != nb->roads.end(); rdB++){
	 if ((rdA->id == rdB->id) && (rdA->id != -1)){
	    roadCounter++;
	    roadID = rdA->id;
	 }
      }
   }
   if (roadCounter == 1)
     return roadID;
   else {
      fprintf (stderr, 
	       _("Restriction/Road Sign: %s."
		 " Number of roads connecting node %d (%2.6f,%2.6f) with node %d (%2.6f,%2.6f) is other than 1 (%d).\n") 
	       ,pl.label.c_str(), na->id, nodes[na->id].p.x, nodes[na->id].p.y, nb->id, nodes[nb->id].p.x, nodes[nb->id].p.y, roadCounter);
      // Required: 1
     warningCounter++;
     return -1;
   }
}

string NodeWriter::processRestriction (Polyline& pl, bool isRoadSign){

   ostringstream osr;
   int road;
   list<RoadID>         roads;
   list<PointParameters*> nodes_param;
   int point = 0;
  
   for (PointsIter pn = pl.points.begin ();pn != pl.points.end ();pn++){
      PointmapIter nd = points.find (*pn);
      if (nd != points.end()){
	 nodes_param.push_back (&(nd->second));
      }
      else {
	 ostringstream osp;
	 osp << fixed << setprecision (config.precision);
	 osp << *pn;
	 fprintf (stderr, _("Restriction/Road Sign: %s."
			    " Point %d %s isn't member of any node.\n"), 
		  pl.label.c_str(), point, osp.str().c_str());
	 warningCounter++;
	 return osr.str();
      }
      point++;
   }
   if (point < 3){
      ostringstream osp;
      if (point > 0){
	 osp << fixed << setprecision (config.precision);
	 osp << *(pl.points.begin());
      }
      fprintf (stderr, _("Restriction/Road Sign: %s %s. Number of nodes (%d) less than 3.\n"), 
	       pl.label.c_str(), osp.str().c_str(), point);
      warningCounter++;
      return osr.str();
   }
   if ((point > 3) && isRoadSign){
      ostringstream osp;
      if (point > 0){
	 osp << fixed << setprecision (config.precision);
	 osp << *(pl.points.begin());
      }
      fprintf (stderr, _("Road Sign: %s %s. Number of nodes (%d) greater than 3.\n"), 
	       pl.label.c_str(), osp.str().c_str(), point);
      warningCounter++;
      return osr.str();
   }
   if (isRoadSign && (pl.label.length() == 0)){
      ostringstream osp;
      if (point > 0){
	 osp << fixed << setprecision (config.precision);
	 osp << *(pl.points.begin());
      }
      fprintf (stderr, _("Road Sign: %s. Empty label\n"), osp.str().c_str());
      warningCounter++;
      return osr.str();
   }
   list<PointParameters*>::iterator np = nodes_param.begin();
   PointParameters *prev_node = *np;
   while ((++np) != nodes_param.end()){
      road = findRoad (pl, prev_node, *np);
      if (road == -1)
	return osr.str();
      roads.push_back (road);
      prev_node = *np;
   }

   bool first = true;
   if (!isRoadSign){
      osr << "[RESTRICT]\n";
      osr << "NOD=";
      np = nodes_param.begin();
      np++;
      for (uint32_t i=0; i < nodes_param.size() - 2; i++){
	 osr << PFMStreamReader::commaIfNotFirst (first) << (*np)->id;
	 np++;
      }      
   } else {
      osr << "[SIGN]\n";
   }
   if (!isRoadSign){
      osr << "\nTRAFFPOINTS=";
   } else {
      osr << "SignPoints=";
   }
   first = true;
   for (np = nodes_param.begin(); np != nodes_param.end(); np++){
      osr << PFMStreamReader::commaIfNotFirst (first) << (*np)->id;
   }
//   Point* restrPoints[10]; // 4 wystarcza
//   int restrPointsNum=0;
   if (config.printRoadSigns && !isRoadSign) {
      // i jeszcze raz dla odtworzenia wspolrzednych z Data0= -- zeby sie generowaly znaki drogowe zakazow na WWW
      osr << "\nData0="; first = true;
      osr << fixed << setprecision (config.precision);
      for (np = nodes_param.begin(); np != nodes_param.end(); np++){
//		  restrPoints[restrPointsNum++]=& nodes[(*np)->id].p;
         osr << PFMStreamReader::commaIfNotFirst (first) << "(" << nodes[(*np)->id].p.x << "," << nodes[(*np)->id].p.y << ")";
      }
   }
   if (!isRoadSign){
      osr << "\nTRAFFROADS=";
   } else {
      osr << "\nSignRoads=";
   }
   list<RoadID>::iterator rd;
   first =true;
   for (rd = roads.begin(); rd != roads.end(); rd++){
      osr << PFMStreamReader::commaIfNotFirst (first) << *rd;
   }
   if (!isRoadSign){
      if (pl.restrParam.length() > 0){
	 osr << "\nRestrParam=" << pl.restrParam;
      }
   } else {
      osr << "\nSignParam=";
      if ((pl.label.length() < 3) || 
	  ((pl.label.substr(0,2) != "O,") && 
	   (pl.label.substr(0,2) != "T,") &&
	   (pl.label.substr(0,2) != "E,"))){
	 osr << "T,";
      } 
      osr << pl.label;
   }
   if (config.printRoadSigns && !isRoadSign) {
	   // Tu beda wywolanie funkcji generujacej znaki zakazu, jesli ich jeszcze nie ma
	   // a jak sa, to je po prostu drukujemy:

           // tu rozpoznajemy czy czasami znak nie jest wylaczony przez Sign=BRAK
           bool rSignDisabled=false;

	   if (pl.roadsignType.length() >0 ) {
                   istringstream ist (pl.roadsignType);
                   string st;
                   getline(ist,st,',');
                   int rt=config.findRoadSign(st);
                   if( rt == ConfigReader::Z_BRAK )
                      rSignDisabled=true;
           }

           vector<Road *> restRoads;
           //  list<PointParameters*>::iterator np ;
           list<RoadID>::iterator rd;

           first =true;
	   rd = roads.begin();
	   np = nodes_param.begin();
           for ( ; rd != roads.end(); rd++ ){
		  RoadlistIter it;
		  for( it = (*np)->roads.begin(); it != (*np)->roads.end() ; it ++) {
			  if( it->id == *rd ) {
				  restRoads.push_back( &(*it));
			  }
		  }
		  np++;
		  for( it = (*np)->roads.begin(); it != (*np)->roads.end() ; it ++) {
			  if( it->id == *rd ) {
				  restRoads.push_back(&(* it));
			  }
		  }
	    }

            vector<Point *> restPoints; // punkty na drodze, po 4 na kazdym segmencie restrykcji

            for(unsigned int i=0; i < restRoads.size();i+=2) {
                  list<Point>::iterator pi;
                  list<Point> * pll = & mapData.lines[restRoads[i]->id].points;
                  int pii;

                if ( restRoads[i]->node < restRoads[i+1]->node ){
                  for( pi=pll->begin(), pii=0; pi != pll->end(); pi++,pii++) {
                     if( pii == restRoads[i]->node ) {
                        restPoints.push_back(&(*pi));
                        pi++;
                        restPoints.push_back(&(*pi));
                        break;
                     }
                   }
                  for( pi=pll->begin(), pii=0; pi != pll->end(); pi++,pii++) {
                     if( pii == restRoads[i+1]->node-1) {
                        restPoints.push_back(&(*pi));
                        pi++;
                        restPoints.push_back(&(*pi));
                        break;
                     }
                   }
                    
                     
		} else {
                  Point * ptmp;
                  for( pi=pll->begin(), pii=0; pi != pll->end(); pi++,pii++) {
                     if( pii+1 == restRoads[i]->node ) {
                        ptmp = &(*pi); 
                        pi++;
                        restPoints.push_back(&(*pi));
                        restPoints.push_back(ptmp);
                        break;
                     }
                   }
                  for( pi=pll->begin(), pii=0; pi != pll->end(); pi++,pii++) {
                     if( pii == restRoads[i+1]->node) {
                        ptmp = &(*pi); 
                        pi++;
                        restPoints.push_back(&(*pi));
                        restPoints.push_back(ptmp);
                        break;
                     }
                   }
                }
            }
            
           
	   double distance=restPoints[0]->metDistance (*restPoints[3]);
           int rtype = -1;
	   Point rp ;  //pkt restrykcji
           double rangle;
           double ent_angle = restPoints[2]->angle(*restPoints[3]);
           double sign_angle = normalize(rad2deg(ent_angle));




           rangle = vangle (*restPoints[2],*restPoints[3],*restPoints[4],*restPoints[5]);
           
           // Restrykcje 4-elemntowe
           if ( point == 4 ) {
	      rp = *restPoints[3] ;  
              double rangle1 = rangle;
              double rangle2 = vangle (*restPoints[6],*restPoints[7],*restPoints[8],*restPoints[9]);
              // dwa razy w lewo to zawracanie
              if ( rad2deg(rangle1) < -20 && rad2deg(rangle2) < -20 ) { // dwa razy w lewo
		double signshift = 5;
                rtype = config.Z_ZAWRACANIA;
                double  uturn_dist = restPoints[4]->metDistance (*restPoints[7]);
                if ( uturn_dist/2 < signshift ) // jesli waska zawrotka to znak w polowie zawrotki
                      signshift = uturn_dist/2 ;
                rp.shift(signshift,ent_angle-PI/2);
                rp.shift(10,ent_angle-PI);

              } else { // to bardziej skomplikowane przypadki
                rtype = config.Z_RESTRYKCJA;
                rp.shift(5,ent_angle+PI/2);
                rp.shift(10,ent_angle+PI);
              }
             
              if ( ! rSignDisabled ) {
                if ( ! rSignPlaceOccupied(rp) ){
		  addRSignHash(rp);
                } else {
                  rp.shift(5,ent_angle+PI);
		  addRSignHash(rp);
                  cerr << "Warning:Sign: Znak przesuniety4=(" << rp.x << "," << rp.y << ")" <<endl;
                }
              }else{
                rp.shift(2,ent_angle+PI/2);
                rp.shift(2,ent_angle+PI);
              }
           // ....
           }

           // Restrykcje 3-elemntowe
           if ( point == 3 ) {
              if ( *restPoints[0] == *restPoints[7]) { // zawracania na drodze dwukierunkowej
                rtype = config.Z_ZAWRACANIA ;
	        rp = *restPoints[3] ;
                // po lewej 5 m z boku drogi , 10m do tylu
                rp.shift(5,ent_angle+PI/2);
                rp.shift(10,ent_angle+PI);
              } else if ( 20 > rad2deg( rangle )  && rad2deg( rangle ) > -20 ) { //prawie na wprost
                rtype = config.Z_PROSTO ;
	        rp = *restPoints[3] ;
                // po lewej 5 m z boku drogi , 10m do tylu
                rp.shift(5,ent_angle+PI/2);
                rp.shift(10,ent_angle+PI);
              } else if ( rad2deg(rangle) < -150 ) { //ostry zakaz w lewo to zakaz zawracanania
                rtype = config.Z_ZAWRACANIA ;
	        rp = *restPoints[3] ;
                // po lewej 5 m z boku drogi , 10m do tylu
                rp.shift(5,ent_angle+PI/2);
                rp.shift(10,ent_angle+PI);
                // cerr << "Zawracanie3=(" << rp.x << "," << rp.y << ")" <<endl;
              } else if ( rangle < 0 ) {
                rtype = config.Z_LEWO ;
	        rp = *restPoints[3] ;
                // po lewej 5 m z boku drogi , 10m do tylu
                rp.shift(5,ent_angle+PI/2);
                rp.shift(10,ent_angle+PI);
              } else {
                rtype = config.Z_PRAWO ;
	        rp = *restPoints[3] ;
                // po lewej 5 m z boku drogi , 10m do tylu
                rp.shift(5,ent_angle+PI/2);
                rp.shift(10,ent_angle+PI);
              } 
              if ( ! rSignDisabled ) {
                if ( ! rSignPlaceOccupied(rp) ){
		  addRSignHash(rp);
                } else {
                  rp.shift(5,ent_angle+PI);
		  addRSignHash(rp);
                  cerr << "Warning:Sign: Znak przesuniety3=(" << rp.x << "," << rp.y << ")" <<endl;
                }
              }else{
                rp.shift(2,ent_angle+PI/2);
                rp.shift(2,ent_angle+PI);
              }
           }
           

	   if (pl.roadsignType.length() >0 ) {
                   istringstream ist (pl.roadsignType);
                   string st;
                   getline(ist,st,',');
                   int rt=config.findRoadSign(st);
                   if( rt == ConfigReader::Z_BLAD ){
                       cerr << "Error:Sign: Sign=" << pl.roadsignType <<endl;
                       osr << "\nSign=" << config.stRoadSign(rtype);
                   } else {
                       osr << "\nSign=" << config.stRoadSign(rt); 
                   }
	   } else {
		   osr << "\nSign=" << config.stRoadSign(rtype);
           }
	   if (pl.roadsignPos.length() >0 ) {
                   char c;
                   string slat(""),slon("");
                   Point rpo;
                   istringstream ist (pl.roadsignPos);
                   if(ist.good()) ist.get(c);
                   if(ist.good()) getline(ist,slat,',');
                   if(ist.good()) getline(ist,slon,')');
                   if( c=='(' && slat.length()>0 && slon.length()>0 ) {
                         rpo=Point(atof(slat.c_str()),atof(slon.c_str()));
                   }
                   double distance=rpo.metDistance(rp);
                   osr << fixed << setprecision (config.precision);
                   if( distance <100 ) { // jesli dalej niz 100m to blad
		      osr << "\nSignPos" << "=(" << rpo.x << "," << rpo.y << ")";
                   }else{
		      osr << "\nSignPos" << "=(" << rp.x << "," << rp.y << ")";
                      cerr << "Error:Sign: SignPos=" << pl.roadsignPos <<endl;
                   }
	   } else {
                   osr << fixed << setprecision (config.precision);
		   osr << "\nSignPos" << "=(" << rp.x << "," << rp.y << ")";
	   }
	   if (pl.roadsignAngle.length() >0 ) {
                   double ang=atof(pl.roadsignAngle.c_str());
                   if(ang == 0 && pl.roadsignAngle != "0" ) {
                       cerr << "Error:Sign: SignAngle=" << pl.roadsignAngle <<endl;
                       osr << "\nSignAngle=" << (int)sign_angle;
                   } else {
		       osr << "\nSignAngle=" << (int)normalize(ang);
                   }
	   } else {
		   osr << "\nSignAngle=" << (int)sign_angle;
           }
	   osr << "\n" << pl.unknown << "[END]\n";;
   } else {
	   osr << "\n[END]\n";
   }

   restrictionCounter++;

   return osr.str();
}

void NodeWriter::NumberingReport (int roadID, map<int,RoadParameters>& nodes){
/*
   bool numbersPresent = false;
   bool cross_streetFound;
   int segment = 0;

   Roadnumbermap::iterator ni = mapData.numbermap.find (roadID);
   if (ni != mapData.numbermap.end()){
      if (!(*ni).second.segmentNumbers.empty()){
	 cerr << "Numbering for road ";
	 cerr <<  roadID << ":";
	 cerr << (*ni).second.label << endl;
	 numbersPresent = true;
	 PointmapIter pn;
      }
   }

   if (numbersPresent){
      map<int,RoadParam>::iterator nd;
      for (nd = nodes.begin(); nd!= nodes.end(); nd++){
	 cerr << "L:" << nd->second.length << " ";
	 cross_streetFound = false;
	 // Znaleźć RoadID przecznic

	 bool first = true;
	 Roadmap::iterator ri;	 
	 for (ri = roads.begin(); ri != roads.end(); ri++){
	    if (ri->second.node == nd->second.node){
	       if (ri->first != roadID){
		  cross_streetFound = true;
		  Roadnumbermap::iterator nip = mapData.numbermap.find (ri->first);
		  cerr << commaIfNotFirst (first);
		  cerr << ri->first << ":";
		  if (nip != mapData.numbermap.end()){
		     cerr << nip->second.label;
		  }
	       }
	    }

	 }

	 if (!cross_streetFound){
	    cerr << "----------------------------------------";
	 }
	 cerr << endl;
	 bool segmentFound = false;
	 list<HouseNumbers>::iterator sn;
	 for (sn = (*ni).second.segmentNumbers.begin();
	      sn != (*ni).second.segmentNumbers.end();
	      sn++){
	    if (sn->startPoint == segment){
	       //	    cerr << "Numbers=" << (*sn).segment << endl;
	       cerr << setw (5) << (*sn).right.start << "          " << setw (5) << (*sn).left.start << endl;
	       cerr << "    " << (*sn).right.kind <<    "              " << (*sn).left.kind << endl;;
	       cerr << setw (5) << (*sn).right.end  <<  "          " << setw(5) << (*sn).left.end << endl;;
	       segmentFound = true;
	    }
	 }

	 map<int,RoadParam>::iterator ne = nd;
	 ne++;
	 if (ne != nodes.end()){
	    if (!segmentFound){
	       cerr << "    ?              ?\n";
	       cerr << "    ?              ?\n";
	       cerr << "    ?              ?\n";
	    }
	 }
	 segment++;
      }
      if (numbersPresent){
	 cerr << endl;
      }
   }
*/
}

void NodeWriter::processRoad (Polyline& pl){
   
   int  nodCounter = 1;
   bool nodeFound = false;
   set<NodeID> node_ids;
   Roadmap::iterator pb = roads.lower_bound (pl.roadID);
   Roadmap::iterator pe = roads.upper_bound (pl.roadID);

   streetNum.clear();

   while (pb != pe){

      nodCounter++;

      streetNum.addNode (pb->second.point);

      if (pb->second.flags & FLAG_MAP_BOUND) {
	bordernodeCounter++;
      } 

      pl.pushNode (pb->second.point, 
		   pb->second.node, 
		   pb->second.flags & FLAG_MAP_BOUND);
	
      if (!node_ids.insert (pb->second.node).second){
	 cerr << _("Warning: Road ") << pl.roadID << " ";
	 if (pl.label.length() > 0){
	    cerr << "(" << pl.label << ") ";
	 }
	 cerr << _("passes through node ") << pb->second.node;
	 cerr << " (" << nodes [pb->second.node].p.x << ",";
	 cerr <<        nodes [pb->second.node].p.y << ") ";
	 cerr << _("more than once.\n");
	 warningCounter++;
      }
      nodeFound = true;
      pb++;
   }

   if (nodCounter > 63){      
      cerr << _("Warning: Too many nodes") << " (" << nodCounter << ") ";
      cerr << _("in road") << " " << pl.roadID;
      if (pl.label.length() >0 ){
	 cerr << " (" << pl.label << ")";
      }
      else {
	 Roadmap::iterator pb = roads.lower_bound (pl.roadID);
	 cerr << " (" << nodes [pb->second.node].p.x << ",";
	 cerr <<        nodes [pb->second.node].p.y << ")";
      }
      cerr << ".\n";
	 warningCounter++;
   }

//   streetNum.generateNumbers (os, &datalist, warningCounter);

}

void NodeWriter::outputMap (){

   int classCounter [5] = {0};

   for (MapSectionListIter mi = mapData.sections.begin(); mi != mapData.sections.end();mi++){
      if (!mi->objectId){
	 os << mi->body;
      } else {
	 mapData.lines[mi->objectId].output(os);
	 if (mapData.lines[mi->objectId].routeParam.valid){
	    if (mapData.lines[mi->objectId].routeParam.route >= 0 &&
		mapData.lines[mi->objectId].routeParam.route < 5){
	       classCounter [mapData.lines[mi->objectId].routeParam.route]++;
	    }
	 }
      }
   }

   double total = 0;
   
   for (int i=0;i<5;i++){
      total += classCounter[i];
   }

   cerr << "Route class statistics:\n";

   for (int i=0;i<5;i++){
      cerr << "Class " << i << ": " << classCounter[i] << " roads " << setprecision(1) << ((double)classCounter[i]/total * 100.0) << "%" << endl;
      cerr << setprecision (config.precision);
   }
}


void NodeWriter::changeClassesOfLinesForNodes (list<int>& nodesToChange, list<int>& modifiedLines){

   list<int>::iterator nli = nodesToChange.begin();
   while (nli != nodesToChange.end()){
      NodemapIter ni = nodes.find(*nli);
      if (ni != nodes.end()){

	 for (RoadlistIter ri=ni->second.roads.begin(); ri!=ni->second.roads.end(); ri++){
	    PolylineMapIter pmi = mapData.lines.find(ri->id);
	    if (pmi != mapData.lines.end()){

      	       if ((pmi->second.tmpRoute != ni->second.highestRoute) &&
		   (pmi->second.tmpRoute != ni->second.averageRoute)){
		  // klasa nie jest równa żadnej z dopuszczalnych
		  // należy ją zmienić
		  int hrDistance = abs((int)pmi->second.tmpRoute - (int)ni->second.highestRoute);
		  int avDistance = abs((int)pmi->second.tmpRoute - (int)ni->second.averageRoute);
		  if (hrDistance <= avDistance){
		     pmi->second.tmpRoute = ni->second.highestRoute;
		  }
		  else {
		     pmi->second.tmpRoute = ni->second.averageRoute;
		  }
		  // dodać linię do listy linii dla których zmieniono klasę
		  modifiedLines.push_back(ri->id);
	       }

	    } else {			   
	       cerr << "Polyline " << ri->id << " not found\n";
	    }
	 }

      }
      else {
	 cerr << "Node " << *nli << " not found\n";
      }
      nli++;
   }
}

void NodeWriter::findAllowedClassesForNodesOfRoads (list<int>& linesToCheck, list<int>& nodesToChange){

   nodesToChange.clear();

   list<int>::iterator li = linesToCheck.begin();
   while (li != linesToCheck.end()){

      Roadmap::iterator pb = roads.lower_bound (*li);
      Roadmap::iterator pe = roads.upper_bound (*li);
      while (pb != pe){	    
	 NodemapIter ni = nodes.find(pb->second.node);
	 short highestRoute = -1;
	 if (ni != nodes.end()){
	    highestRoute = setHighestRouteForNode (ni); 
	    bool moreThanTwoRoutesFound = setAverageRouteForNode (ni);
	    if (moreThanTwoRoutesFound){
	       nodesToChange.push_back(pb->second.node);
	    }
	 } else {
	    cerr << "NodeId not found\n";
	 }
	 pb++;
      }
      li++;
   }
   cerr << "Number of nodes where more than 2 classes are needed: " 
	   << nodesToChange.size() << endl; 
}	   

bool NodeWriter::setAverageRouteForNode (const NodemapIter& ni){
   int sumOfRoutes = 0;
   int numberOfRoads = 0;
   short previousRoute = -1;
   bool moreThanTwoRoutesFound = false;
   short highestRoute = ni->second.highestRoute;

   for (RoadlistIter ri=ni->second.roads.begin(); ri!=ni->second.roads.end(); ri++){
      PolylineMapIter pmi = mapData.lines.find(ri->id);
      if (pmi != mapData.lines.end()){

	 if (pmi->second.tmpRoute != highestRoute){
	    sumOfRoutes += pmi->second.tmpRoute;
	    numberOfRoads++;
	    if (previousRoute < 0){
	       previousRoute = pmi->second.tmpRoute;
	    }
	    else {
	       if (previousRoute != pmi->second.tmpRoute){
		  moreThanTwoRoutesFound = true;
	       }
	    }
	 }

      } else {			   
	 cerr << "Polyline " << ri->id << " not found\n";
      }
   }

   if (numberOfRoads > 0){
      ni->second.averageRoute = (short)ceil((double)sumOfRoutes/(double(numberOfRoads)));
   }
   return moreThanTwoRoutesFound;
}

short NodeWriter::setHighestRouteForNode (const NodemapIter& ni){
   short highestRoute = -1;

   for (RoadlistIter ri=ni->second.roads.begin(); ri!=ni->second.roads.end(); ri++){
      PolylineMapIter pmi = mapData.lines.find(ri->id);
      if (pmi != mapData.lines.end()){

	 if (pmi->second.tmpRoute > highestRoute){
	    highestRoute = pmi->second.tmpRoute;
	 }

      } else {			   
	 cerr << "Polyline " << ri->id << " not found\n";
      }
   }

   if (highestRoute > 0){
      ni->second.highestRoute = highestRoute;
   }
   return highestRoute;
}

void NodeWriter::findAllowedClassesForAllNodes (list<int>& nodesToChange){

   nodesToChange.clear();
   NodemapIter ni = nodes.begin();
   while (ni != nodes.end()){

      setHighestRouteForNode (ni); 

      bool moreThanTwoRoutesFound = setAverageRouteForNode (ni);
      
      if (moreThanTwoRoutesFound){
	 nodesToChange.push_back(ni->first);
      }
      ni++;
   }

   cerr << "Number of nodes where more than 2 classes are needed: " << nodesToChange.size() << endl; 

}

void NodeWriter::copyTmpRouteToRoute (){
   for (MapSectionListIter mi = mapData.sections.begin(); mi != mapData.sections.end();mi++){
      if (mi->objectId){
	 if (!config.isRestrictionOrRoadSign(mapData.lines[mi->objectId].type)){
	    mapData.lines[mi->objectId].routeParam.route = 
		    mapData.lines[mi->objectId].tmpRoute;
	 }
      }
   }
}

void NodeWriter::copyRouteToTmpRoute (){
   for (MapSectionListIter mi = mapData.sections.begin(); mi != mapData.sections.end();mi++){
      if (mi->objectId){
	 if (!config.isRestrictionOrRoadSign(mapData.lines[mi->objectId].type)){
	    mapData.lines[mi->objectId].tmpRoute = 
		    mapData.lines[mi->objectId].routeParam.route;
	 }
      }
   }
}

void NodeWriter::outputIncidenceMap (ostream& os){
   for (NodemapIter ni = nodes.begin(); ni != nodes.end(); ni++){
      os << ni->first << " - ";
      IncidenceMapIter imi = ni->second.incidences.begin();
      while (imi != ni->second.incidences.end()){

	 os << "(" << imi->first << "," << imi->second.roadID << "," << imi->second.segment << ") ";

	 imi++;
      }
      os << endl;
   }
}

void NodeWriter::generateIncidenceMap (){
   int incCounter = 0;
   for (NodemapIter ni = nodes.begin(); ni != nodes.end(); ni++){
      for (RoadlistIter ri = ni->second.roads.begin(); 
	   ri != ni->second.roads.end(); ri ++){

	 // mamy numer punktu drogi, trzeba stwierdzić który to jej nod
	 // oraz jakie id mają poprzedni i nastepny. Taka informacja jest tylko
	 // w liście polylinii, a dokładniej w jej atrybucie nodes

	 PolylineMapIter pi = mapData.lines.find(ri->id);
	 if (pi != mapData.lines.end()){

	    map<int, PolylineNode>::iterator pni;
	    map<int, PolylineNode>::iterator pni_n;
	    map<int, PolylineNode>::iterator pni_c;

	    pni = pi->second.nodes.find(ri->node);
	    if (pni != pi->second.nodes.end()){

	       int nodeNum = 0;

	       pni_c = pni;
	       while (pni_c != pi->second.nodes.begin()){
		  nodeNum++;
		  pni_c--;
	       }

	       pni_n = pni;
	       pni_n++;

	       if (pni_n != pi->second.nodes.end()){
		  ni->second.incidences.insert (make_pair (pni_n->second.node, Incidence(ri->id, nodeNum)));
     		  incCounter++;
	       }
	       if (pni != pi->second.nodes.begin()){
		  pni--;

		  if (pi->second.dirIndicator > 0){
		     // połączenie w kierunku przeciwnym do kierunku drogi
   		     ni->second.incidences.insert (make_pair (pni->second.node, Incidence(ri->id, nodeNum - 1, false)));
		  } else {
		     ni->second.incidences.insert (make_pair (pni->second.node, Incidence(ri->id, nodeNum - 1, true)));
		  }
     		  incCounter++;
	       }

	    } else {
	       cerr << "Node not found\n";
	    }

	 } else {
	    cerr << "Polyline not found\n";
	 }
      }

   }
   cerr << "Incidence map generated\n";
   cerr << "Number of incidences: " << incCounter << endl;
}

void NodeWriter::process (){

   list<Polyline*> connectors;

   for (MapSectionListIter mi = mapData.sections.begin(); mi != mapData.sections.end();mi++){
      if (mi->objectId){
	 if (!config.isRestrictionOrRoadSign(mapData.lines[mi->objectId].type)){
	    if (PFMStreamReader::isRoutable(mapData.lines[mi->objectId].type)){
	       for (PointsIter pn = mapData.lines[mi->objectId].points.begin();
		    pn != mapData.lines[mi->objectId].points.end(); 
		    pn++){
		  PointmapIter pm = points.find (*pn);
		  if (pm != points.end()){
		     *pn = nodes[pm->second.id].p;  
		  }
	       }	    
	    }
	    if (config.isConnector(mapData.lines[mi->objectId].type)){
	       connectors.push_back(&mapData.lines[mi->objectId]);
	    }
	    if (mapData.lines[mi->objectId].roadID != -1)
	      processRoad (mapData.lines[mi->objectId]);
	 } else { // restrykcja albo road sign
      	    string buf = processRestriction (mapData.lines[mi->objectId], 
					     config.isRoadSign(mapData.lines[mi->objectId].type));
      	    mi->objectId = 0;
	    mi->body = buf;
	    // informacja o restrykcji jako o linii nie jest już dłużej potrzebna
	    mapData.lines.erase(mi->objectId);
	 }
      }
   }

   if (config.adjustConnectorClass){

      cerr << "Adjusting classes of " << connectors.size() << " connectors" << endl;

      copyRouteToTmpRoute ();
/*
      // Find connected connectors
     
      ConnectionMap cmap;
       
      for (list<Polyline*>::iterator connIter = connectors.begin();connIter != connectors.end(); connIter++){
	 
	 int type = (*connIter)->type;
     	 cerr << "Checking road: " << (*connIter)->roadID << " type: " << type << endl;

	 set<int> conns;

      	 Roadmap::iterator pb = roads.lower_bound ((*connIter)->roadID);
	 Roadmap::iterator pe = roads.upper_bound ((*connIter)->roadID);
	 while (pb != pe){	    
//	       cerr << "Checking node: " << pb->second.node << endl;
	    NodemapIter ni = nodes.find(pb->second.node);
	    if (ni != nodes.end()){
	       for (RoadlistIter ri=ni->second.roads.begin(); ri!=ni->second.roads.end(); ri++){
		  PolylineMapIter pmi = mapData.lines.find(ri->id);
		  if (pmi != mapData.lines.end()){
		     if (((*connIter)->roadID != pmi->second.roadID) && 
			 (pmi->second.type == type)){

			cerr << "  conncted with: " << ri->id << endl;
			conns.insert(ri->id);
		     }
		  } else {			   
		     cerr << "Polyline " << ri->id << " not found\n";
		  }
	       }
	    } else {
	       cerr << "Node not found\n";
	    }
	    pb++;
	 }	    	    
	 if (conns.size() > 0){
	    cmap.insert(make_pair((*connIter)->roadID, conns));
	 }
      }
      ConnectionMapIter cmi;
      cmi = cmap.begin();
      do {
	 while (cmi != cmap.end()){
	    set<int>::iterator sii;
	    sii = cmi->second.begin();
	    while (sii != cmi->second.end()){
	       ConnectionMapIter next = cmap.find(cmi->first);
	       if (next != cmap.end()){
		  // dodaj połączenia połączenia do listy połączeń
		  bool inserted = false;
		  set<int>::iterator nsii;
		  nsii = cmi->second.begin();
		  while (nsii != cmi->second.end()){
		     set<int>::iterator cmci;
		     cmci = cmi->second.find (*nsii);
		     if (cmci == cmi->second.end()){
			cmi->second.insert(*nsii);
			inserted = true;
		     }
		     nsii++;
		  }
		  if (inserted){
		  }
	       }
	       sii++;
	    }
	    cmi++;
	 }
      } while (cmi != cmap.end());

*/

      //////////////

      bool somethingToDo = true;

      int iterationCounter = 0;

      // Zamiast pętlić się bezmyślnie można zastosować algorytm podobny do 
      // użytego w STAGE2, ale przy małej liczbie łączników poniższe
      // rozwiązanie też jest do przyjęcia

      while (somethingToDo){
	 iterationCounter++;
	 somethingToDo = false;
	 cerr << "Iteration: " << iterationCounter << endl;
	 for (list<Polyline*>::iterator connIter = connectors.begin();connIter != connectors.end(); connIter++){
//	    cerr << "Processing road: " << (*connIter)->roadID << endl;
	    // Na podstawie roadID znaeźć nody, a dla nodów inne drogi 
	    // wybrać najwyższą klasę spośród znalezionych dróg i ustawić
	    // ją dla connectora, jeśli jest inna niż była ustawić somethingToDo
	    int highestRoute = -1;
	    int secondRoute  = -1;
	    int highestRouteNodeId = -1;

	    Roadmap::iterator pb = roads.lower_bound ((*connIter)->roadID);
	    Roadmap::iterator pe = roads.upper_bound ((*connIter)->roadID);
	    while (pb != pe){	    
//	       cerr << "Checking node: " << pb->second.node << endl;
	       NodemapIter ni = nodes.find(pb->second.node);
	       if (ni != nodes.end()){
		  int hr = setHighestRouteForNode (ni);
//		  cerr << "hr: " << hr << endl;
		  if (hr > highestRoute){
		     highestRouteNodeId = pb->second.node;
		     highestRoute = hr;		     
		  }
	       } else {
		  cerr << "Node not found\n";
	       }
	       pb++;
	    }	    	    
	    if (config.connectorClassesAdjustmentVariant == 2){
	       pb = roads.lower_bound ((*connIter)->roadID);
	       while (pb != pe){	    
		  if (pb->second.node != highestRouteNodeId){
		     NodemapIter ni = nodes.find(pb->second.node);
		     if (ni != nodes.end()){
			int hr = setHighestRouteForNode (ni);
			if (hr > secondRoute){
			   secondRoute = hr;		     
			}
		     } else {
			cerr << "Node not found\n";
		     }
		  }
		  pb++;
	       }
	       /*
	       cerr << "roadId: " << (*connIter)->roadID << " hr: " << highestRoute
		       << " sr: " << secondRoute << endl;
	       */
	       if (secondRoute >= 0){
		  highestRoute = secondRoute;
	       }
	    }
	    if (highestRoute > (*connIter)->routeParam.route){
   	       (*connIter)->routeParam.route = highestRoute;
   	       (*connIter)->tmpRoute = highestRoute;
   	       somethingToDo = true;
	    }
	 }
      }
      cerr << endl;
   }

   if (config.adjustClassesInNode){

      cerr << "Reducing number of classes for single node\n";

      copyRouteToTmpRoute ();

      list<int> nodesToChange;

      // Wyznacz dwie dopuszczalne klasy dla każdego noda
      // nody, dla których nalezy zmienić klasy dróg umieść
      // we wskazanej liście
      findAllowedClassesForAllNodes (nodesToChange);

      int iterationCounter = 1;

      do {

	 cerr << "Iteration: " << iterationCounter << "\n";

	 list<int> modifiedLines;

	 // dla nodów z listy, zmień klasy dróg zgodnie z wyznaczonymi
	 // klasami dopuszczalnymi dla noda, identyfikatory dróg
	 // dla których zmieniono klasę umieść we wskazanej liście
	 
	 changeClassesOfLinesForNodes (nodesToChange, modifiedLines);

   	 cerr << "Number of modified routes: " << modifiedLines.size() << endl; 

	 // dla dróg z listy ponownie wyznacz pary klas dla nodów, przez które 
	 // przechodzą owe drogi. nody w których drogi wymagają zmiany
	 // umieśc we wskazanej liście

	 findAllowedClassesForNodesOfRoads (modifiedLines, nodesToChange);
      
      	 iterationCounter++;

   	 // powtarzać proces do momentu gdy lista węzłów do modyfikacji będzie 
	 // pusta lub przekroczona maksymalna liczba iteracji
	 
      } while ((nodesToChange.size() > 0) && (iterationCounter < 30));

      if (iterationCounter >= 30){
      	 cerr << "Maximum number of iterations reached, process has not been finished\n";

	 // Sprawdź stan w jakim algorytm zakończył działanie i wyświetl go
	 findAllowedClassesForAllNodes (nodesToChange);
      }

      // dla kontroli działania algorytmu, powtórz przeszukiwanie dla wszystkich nodów.
      // findAllowedClassesForNodes (nodesToChange);

      // skopiuj skorygowane klasy z tmpRoute do route  
      copyTmpRouteToRoute ();

   } // STAGE_2


   PolylineListIter pl;

   for (pl = mapData.numbers.begin();pl != mapData.numbers.end();pl++){
      if (!pl->used){
	 cerr << "Warning: ";
	 StreetNum::description(pl, cerr); 
	 cerr << " is not used.\n"; 
	 warningCounter++;
      }
   }
   cerr << endl;
   cerr << _("Number of restrictions and road signs: ") << restrictionCounter  << endl;
   cerr << _("Number of warnings: ") << warningCounter  << endl;
   cerr << _("Number of border nodes: ") << bordernodeCounter  << endl << endl;

//   generateIncidenceMap ();
//   outputIncidenceMap (cerr);

}
