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

#ifndef NodeWriterH
#define NodeWriterH

#include "OziOutput.h"
#include "ConfigReader.h"
#include "StreetNum.h"
#include "RoadIDGenerator.h"
#include "datatypes.h"
#include "netgen.h"
#include <list>
#include <set>
#include <vector>
// for int64_t
#include <stdint.h>

#define  HASHSQSIZE   1073741824  //2^30
#define  HASHGRIDSIZE  2

class NodeWriter {
	private:
		Roadmap     roads;  // numery węzłów i punktów dla drogi
		Pointmap&   points; // id węzła i lista dróg dla punktu 
		Nodemap&    nodes;  // współrzedne dla node_id
		MapData&    mapData;
                std::set<int64_t>    rSignHashes;
                double rSignHashLatitudeBase;
		std::string sectionBuf;
		std::ostream& os;
		std::string processRestriction (Polyline& pl, bool isRoadSign);
		void processRoad (Polyline& pl);
		int findRoad (Polyline& pl, PointParameters *na, PointParameters *nb);
		int warningCounter;
		int endNodeCounter;
		int firstEndNodeID;
		int restrictionCounter;
		int bordernodeCounter;
		ConfigReader& config;
		StreetNum streetNum;

		void NumberingReport (int roadID, std::map<int,RoadParameters>& nodes);
		void findAllowedClassesForAllNodes (std::list<int>& nodesToChange);
		void findAllowedClassesForNodesOfRoads (std::list<int>& linesToCheck, 
					  	        std::list<int>& nodesToChange);
		void changeClassesOfLinesForNodes (std::list<int>& nodesToCheck, 
						   std::list<int>& modifiedLines);
		void copyTmpRouteToRoute ();
		void copyRouteToTmpRoute ();
		short setHighestRouteForNode (const NodemapIter& ni);
		bool  setAverageRouteForNode (const NodemapIter& ni);
                int64_t rSignHash(const Point& p);
                bool rSignPlaceOccupied(const Point& p);
                void addRSignHash(const Point& p);
	public:
		NodeWriter (std::ostream& ostr, 
			    MapData& md,
			    ConfigReader& cfg);
		void process ();
		int getFirstEndNodeID ();
		void markNodesNearRoad (double radius);
		void checkMinimumDistance (double epsilon);
		void generateIncidenceMap ();
		void outputIncidenceMap (std::ostream& os);
		void outputMap ();
};

#endif
