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

#include "StreetNum.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdlib>

using namespace std;

StreetNum::StreetNum (MapData& md):
	mapData (md)
{
}

bool StreetNum::even (int n){
   if (n % 2 == 0)
     return true;
   else
     return false;
}

bool StreetNum::odd (int n){
   return ! even(n);
}


void StreetNum::description (PolylineListIter pl, ostream& os){
   os << "StreetNum " <<  pl->label << " " 
  	   << pl->points.back() << "-" << pl->points.front();
}

bool StreetNum::streetNumOverlaps (HouseNumbers& hn){
   bool overlaps = false;
   list<HouseNumbers>::iterator hni;
   for (hni = numberList.begin(); hni != numberList.end(); hni++){
      if (!(((hni->startPoint < hn.startPoint) && (hni->endPoint <= hn.startPoint)) || 
	    ((hni->startPoint >= hn.endPoint)&&( hni->endPoint > hn.endPoint)))){
	 return true;
      }
   }
   // do ustalenia czy ma tu zwracać true czy false, ale chyba false
   return false;
}

char StreetNum::numbersType (int begin, int end){
   if ((begin == -1 && end == -1) || (begin == 0 && end == 0)){
      return 'n';
   }
   else {
      if (even (begin) && even(end)){
	 return 'e';
      }
      else {
	 if (odd (begin) && odd(end)){
	    return 'o';
	 }
	 else {
	    return 'b';				    
	 }
      }
   }
}

int StreetNum::countCommas (string& str){
   int n = 0;
   string::iterator si;
   for (si=str.begin();si!=str.end();si++){
      if (*si == ',')
	n++;
   }
   return n;
}

int StreetNum::extractNumbers (const string& buf, int& begin, int& end, char& type){
   unsigned pos;
   int tmpl, tmpr;
   pos = buf.find ('+');
   if (pos > 0 && pos < buf.length()-1){
      istringstream isl (buf.substr (0,pos));
      istringstream isr (buf.substr (pos + 1));
      isl >> tmpl;     
      isr >> tmpr;
      if (isl.fail() || isr.fail())
	return -1;
      begin = tmpl;
      end   = tmpr;
      if ((begin == -1 && end == -1) || (begin == 0 && end == 0))
	type = 'n';
      else  
	type = 'b';
      return 0;
   }
   else {
      pos = buf.find ('-');
      if (pos > 0 && pos < buf.length()-1){
	 istringstream isl (buf.substr (0,pos));
	 istringstream isr (buf.substr (pos + 1));

	 isl >> tmpl;     
	 isr >> tmpr;
	 if (isl.fail() || isr.fail())
	   return -1;
	 begin = tmpl;
	 end   = tmpr;

	 type = numbersType (begin,end);
	 return 0;
      }
      else {	
	return -1;
      }
   }
   
}

int StreetNum::pointInList(Point& p, list<Points>* datalist){
   list<Points>::iterator da = datalist->begin ();
   int pnum = 0;
   while (da != datalist->end()){
      for (PointsIter pn = da->begin (); pn != da->end(); pn++){
	 if (p == *pn){
	    return pnum;
	 }
	 pnum++;
      }
      da++;
   }
   return -1;
}

void StreetNum::addNode (int point){
      nodes.insert(make_pair(point,0));
}

void StreetNum::clear (){
   nodes.clear ();
}

void StreetNum::numberNodes (){
   map<int,int>::iterator trni;
   trni = nodes.begin ();
   int nodenum = 0;
   while (trni != nodes.end()){
      trni->second = nodenum++;
      trni++;
   }
}

void StreetNum::parseLabel (string label,
			    int& rBegin,int& rEnd,char& rType,
			    int& lBegin,int& lEnd,char& lType){

   if (label.length() == 0){
      rType = lType = 'n';
      rBegin = rEnd = lBegin = lEnd = 0;
      return;
   }
   istringstream is (label);
   // string str; 
   //  As the first step - check number of commas in string
   //  less then 1 - only right side, + or - required
   //  1 - split into 2 parts, + or - required in each of them		     
   //  2 - error
   //  3 - get four numerals
   //  more than 3 - error
   int nc = countCommas (label);
   unsigned cpos;
   switch (nc){
	   case 0:
		   if (extractNumbers (label,rBegin,rEnd,rType) == 0){
		      lType = 'n';
		      lBegin = lEnd = 0;
		   }
		   break;
	   case 1:
		   cpos = label.find (',');
		   if (cpos > 0 && cpos < label.length()-1){
	     	      if (extractNumbers (label.substr (0, cpos),rBegin,rEnd,rType) == 0){
     			 if (extractNumbers (label.substr (cpos+1),lBegin,lEnd,lType)==0){
			 }
		      }
		   }
		   break;
	   case 3:
		   is >> rBegin;
		   is.ignore (100,',');
		   is >> rEnd;
		   is.ignore (100,',');
		   is >> lBegin;
		   is.ignore (100,',');
		   is >> lEnd;
		   if ((rBegin > rEnd && lBegin < lEnd) ||
		       (rBegin < rEnd && lBegin > lEnd)){
		      lType = rType = 'b';
		   }
		   else {
		      rType = numbersType (rBegin, rEnd);
		      lType = numbersType (lBegin, lEnd);
		   }
   }
}

void StreetNum::generateNumbers (ostream& os,
				 list<Points>* datalist,
				 int& warningCounter
				 ){

   int pnum = 0;
   int numnum = 1;
   numberList.clear();
   numberNodes ();

   list<Points>::iterator da = datalist->begin ();
   while (da != datalist->end()){
      for (PointsIter pn = da->begin (); pn != da->end(); pn++){
	 //  Find point in numbers, check if line was not used before
	 PolylineListIter pl;
	 for (pl = mapData.numbers.begin();pl != mapData.numbers.end();pl++){
	    if (!pl->used){
	       if (pl->points.back() == *pn){ 
		  //  Check if second point is in current road as well
		  int secPoint = pointInList(pl->points.front(), datalist);
		  //  Check if points are consecutive road points or nodes
		  if (secPoint != -1){
		     map<int,int>::iterator ndB, ndE;
		     ndB = nodes.find (pnum);
		     ndE = nodes.find (secPoint);
		     bool consecutiveNodes  = false;
	     	     bool consecutivePoints = false;
     		     if (ndE != nodes.end() && ndB !=nodes.end()){
		       	if (abs (ndE->second - ndB->second) == 1){
	     		   consecutiveNodes = true;
			}
		     }
	     	     if (abs (secPoint - pnum) == 1 ){
     			consecutivePoints = true;
		     }
	     	     if (!consecutivePoints && !consecutiveNodes){

			// interpolation may take place here
			//
     			cerr << "Warning: " ;
			description(pl, cerr); 
		 	cerr << " doesn't connect consecutive points (or nodes) of road.\n";
			warningCounter++;
			continue;
		     }

     		     int rBegin = -9, rEnd = -9, lBegin = -9, lEnd = -9;
     		     char lType = 'n', rType = 'n';
		     parseLabel (pl->label,rBegin,rEnd,rType,lBegin,lEnd,lType);
		     if (rBegin == -9 || rEnd == -9 || 
			 lBegin == -9 || lEnd ==-9){
			   cerr << "Warning: ";
			   description(pl, cerr); 
			   cerr << " invalid label: " << pl->label << " \n";
			   warningCounter++;
			   pl->used = true;
			   continue;
	     	     }

		     HouseNumbers hn;
	       	     hn.startPoint = min (secPoint, pnum);
	  	     hn.endPoint =   max (secPoint, pnum);

	     	     if (streetNumOverlaps (hn)){
     			cerr << "Warning: ";
			description(pl, cerr); 
			cerr << " overlaps with Numbers defined before.\n";
			warningCounter++;
			continue;
		     }
		     //  generate NumbersX parameter
		     os << "Numbers" << numnum << "=" 
			     << min (secPoint, pnum)	<< ","
			     << lType << ","
			     << lBegin << ","
			     << lEnd << ","
			     << rType << ","
			     << rBegin << ","
			     << rEnd << ","
			     << "-1,-1,-1,-1" << endl;

		     numberList.push_back (hn);	
	       	     numnum++;
     		     pl->used = true;
		  }
	       }
	    }
	 }
	 pnum++;
      }	    
      da++;
   }
}

