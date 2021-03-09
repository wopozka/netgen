#ifndef mapdataH
#define mapdataH

#include "Polyline.h"
#include "Line.h"
#include "Point.h"
#include "datatypes.h"
#include <set>

class MapData {
	public:
		MaskingPointmap roadEnds;       // końce dróg (maskowane)
		MaskingPointmap noCrossings;    // przecięcia bez skrzyzowań (maskowane)
		double maskSize;
		std::list<Line> lineList;
		std::list<Point> acuteAngles;
		Pointmap pointmap;
		PolylineList numbers; 	
		PolylineMap lines;
				    
		Nodemap nodes; // współrzedne dla node_id

                MapSectionList sections;

		MapData (double ms) :maskSize(ms){ }

		void outputMap (std::ostream& os){
		   for (MapSectionListIter mi = sections.begin();mi != sections.end();mi++){
		      if (!mi->objectId){
			 os << mi->body;
		      } else {
			 lines[mi->objectId].output(os);
		      }
		   }
		}

		bool isMaskedRoadEnd(const Point &p){
			return isMasked(p,roadEnds,maskSize);
		}
		bool isMaskedNoCrossing(const Point &p){
			return isMasked(p,noCrossings,maskSize);
		}
		bool isMasked(const Point &p, MaskingPointmap &ps,
				     double thres){

			MaskingPointmapIter psit, endIter;

			psit    = ps.lower_bound (Point (p.x - thres, p.y - thres));

			endIter = ps.upper_bound (Point (p.x + thres, p.y + thres));

			for(; psit != endIter; psit++){
		   	   if (!psit->first.lessT(p,thres) && !p.lessT(psit->first,thres)){
			      psit->second = true;
   			      return true;
	   		   }
			}
			return false;
		}
};


#endif
