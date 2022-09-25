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

#ifndef NetAnalyzerH
#define NetAnalyzerH

#include "MapData.h"
#include "ConfigReader.h"
#include "datatypes.h"
#include "netgen.h"
#include <list>

class NetAnalyzer {
	private:
		MapData&      mapData;
		ConfigReader& config;
		Nodemap& nodes; // współrzedne dla node_id

		bool checkEndNode (short route, Point& startPoint, int roadID, int nodeID);
		bool checkNextNode (short route, Point& startPoint, int startRoadID, int roadID, int nodeID, int depth, double distance, std::list<int> history);

		bool checkContinuity (int startNode, int endNode, const std::string& label, short route, std::set<int>& roadSoFar, int depth, int depthAny, std::list<Point>& errors);
	public:
		NetAnalyzer (MapData& md, ConfigReader& cfg);
		void findLowClassHoles (short route, std::list<Point>& errors);
		void checkContinuity (const Point& startPoint, const Point& endPoint, 
		     		      short route, const std::string& label, std::list<Point>& errors);
	

};

#endif
