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
#include "netgen.h"

#include <iostream>

using namespace std;

Polyline::Polyline (): used(false) {
}

void Polyline::pushNode (int p, int n, bool b){
   nodes.insert(make_pair(p, PolylineNode(n,b)));
}


void Polyline::output (ostream& os){
   if (markerKind)
     os << "[POLYLINE]\n";
   else
     os << "[RGN40]\n";
   if ((roadID >=0) && (roadID <= MAX_ROAD_ID)){
      os << "RoadID=" << roadID << "\n";
   }
   os << "Type=0x" << std::hex << type << std::dec << "\n";
   if (label.length()){
      os << "Label=" << label << "\n";
   }		   
   if (endLevel > 0){
      os << "EndLevel=" <<  endLevel << "\n";
   }
   if (dirIndicator > 0){
      os << "DirIndicator=" << (int)dirIndicator << "\n";
   }
   os << "Data0=";
   PointsIter pn;
   bool first = true;
   pn = points.begin();
   while (pn != points.end()){
      if(!first)
	os << ',';
      os << *pn;
      pn++;
      first = false;
   }
   os << "\n";
   if (unknown.length()){
      os << unknown ;
   }		   
   int counter = 1;
   std::map<int, PolylineNode>::iterator ni = nodes.begin();
   while (ni != nodes.end()){
      os << "Nod" << counter++ << '=' 
	      << ni->first << ',' << ni->second.node << ','
	      << (ni->second.border ? '1' : '0') << '\n';
      ni++;
   }
   if (routeParam.valid){
      os << "Routeparam=";
      routeParam.outputSpeedClass (os);
      routeParam.outputRouteRestrictions (os, true);		      
   }
   if (markerKind)
     os << "[END]\n";
   else
     os << "[END-RGN40]\n";	     
}

