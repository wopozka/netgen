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

#include "OziOutput.h"
#include "netgen.h"

#include <cmath>

using namespace std;

const std::string OziOutput::unknownParameters = 
                                  ",,0,1,3,255,65535,,,0,0,0,6,0,19\n";

OziOutput::OziOutput (ostream& ostr):
	os (ostr)
{
}
	
void OziOutput::Header (){
   os << "OziExplorer Waypoint File Version 1.1\n";
   os << "WGS 84\n";
   os << "Reserved 2\n";
   os << "Reserved 3\n";
}

string OziOutput::flagsToString (int flags){
   string rv;
   if (flags & FLAG_CLOSE)
     rv += "C";
   if (flags & FLAG_ALIGNED)	   
     rv += "A";
   if (flags & FLAG_MAP_BOUND)	   
     rv += "M";
   if ((flags & FLAG_HAS_ENTRANCE) == 0){
     rv += "E"; // Node without entrance
   }	    
   if ((flags & FLAG_HAS_EXIT) == 0){
     rv += "X"; // Node without exit
   }
   return rv;
}

void OziOutput::Nodes (Nodemap &nodes, int endNodeID, MapData& md, bool skip_regular, bool skip_alone){

   int endNodeCounter = 0;
   for (NodemapIter nd =  nodes.begin (); nd != nodes.end (); nd++){
      // check for "too close" and aligned nodes and distinguish them
      // by other name (NC, BC, NA, BA, NCA, NBA)
      // or N-n-nnnn-CA
      // Removing of "regular" nodes from list also can be done here
      // 
      if (nd->first < endNodeID){
      	 if (skip_regular && 
	     ((nd->second.flags & (FLAG_CLOSE | FLAG_ALIGNED | FLAG_MAP_BOUND)) == 0)
	     && ((nd->second.flags & FLAG_HAS_ENTRANCE_AND_EXIT) == FLAG_HAS_ENTRANCE_AND_EXIT)
	     )
   	   continue;
	 os << "-1,N" ;
	 os << flagsToString (nd->second.flags);
	 os << nd->first << "-" << nd->second.n << "," 
		 << nd->second.p.x << "," << nd->second.p.y 
		 << unknownParameters;
      }
      else {
	 // jeśli jest oznaczony jako C lub M to nie powinien być maskowany
	 if ((!md.isMaskedRoadEnd(nd->second.p)) ||
	     ((nd->second.flags & (FLAG_CLOSE | FLAG_ALIGNED | FLAG_MAP_BOUND)) != 0)){
	    if (!skip_alone || (nd->second.flags & FLAG_NEAR_ROAD)) {
   	       os << "-1,B" ;
	       os << flagsToString (nd->second.flags);
   	       os << nd->first << "-" << nd->second.n << "," 
   		       << nd->second.p.x << "," << nd->second.p.y 
	      	       << unknownParameters;
   	       endNodeCounter++;
	    }
	 }
      }
   }
   cerr << _("Number of unmasked end-road nodes: ") << endNodeCounter  << endl;   

}

void OziOutput::Intersections (list<Point> &is, MapData& md){
   List (is,"I");

}

void OziOutput::Unused (MapData& md, bool intersections, bool roadends){
   MaskingPointmapIter mpi;
   if (intersections){
      int counter = 1;
      mpi = md.noCrossings.begin();
      while (mpi != md.noCrossings.end()){
	 bool usedInOther = false;
	 MaskingPointmapIter mpiOther = md.roadEnds.find(mpi->first);
	 if (mpiOther != md.roadEnds.end()){
	    usedInOther = mpiOther->second;
	 }
	 if (!mpi->second && !usedInOther){
	    os << "-1,UI" << counter++ << "," << mpi->first.x << ',' << mpi->first.y << unknownParameters;
	 }
	 mpi++;
      } 
   }
   if (roadends){
      int counter = 1;
      mpi = md.roadEnds.begin();
      while (mpi != md.roadEnds.end()){
	 bool usedInOther = false;
	 MaskingPointmapIter mpiOther = md.noCrossings.find(mpi->first);
	 if (mpiOther != md.noCrossings.end()){
	    usedInOther = mpiOther->second;
	 }
	 if (!mpi->second && !usedInOther){
	    os << "-1,UB" << counter++ << "," << mpi->first.x << ',' << mpi->first.y << unknownParameters;
	 }
	 mpi++;
      } 
   }
}

void OziOutput::List (list<Point> &is, const string& prefix){
   int cntr = 1;
   for (list<Point>::iterator pn = is.begin(); pn != is.end(); pn++){
      os << "-1," << prefix << cntr++  << "," 
	      << pn->x << "," << pn->y 
	      << unknownParameters;
   }
}
