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

#ifndef RoadIDGeneratorH
#define RoadIDGeneratorH

#include "ConfigReader.h"
#include "PFMStreamReader.h"
#include "MapData.h"
//#include "Intersections.h"
#include "datatypes.h"
#include <iostream>
#include <string>
#include <list>

class RoadIDGenerator : public PFMStreamReader {
	private:
		const static int TYPE_BACKGROUND = 0x4b;
		Polyline *pl;
		bool first; // point in road
		int pointInRoad;
		Point *previousPoint;
		int restrictionID;
		int currentRoadID;
		double length; // Road length from begining to current point (squared)
//		RoadNumbers   numbers;
		int loopCounter;
		int removedCounter;
		std::string sectionHeader;
		std::string sectionBuf;
		void checkMinAngle(Line* line);
		void startRoad ();
		void writeToMapAndOutput (Point pn, bool last);
		void writeAllToMap (unsigned char flags);
		void endRoad ();
		void outputSegment (PointsIter& pn, int segmentSize);
		bool findNode (Points& pts, int &n1, int &n2);
		int half (int m, int n);
		ConfigReader& config;

		MapData *mapData;

		void comment    (const std::string& line);
		void section    (const std::string& sec);
		bool token      (const std::string& tok, 
				 const std::string& val);
	public:

		RoadIDGenerator (std::istream& istr,
				 MapData *md,
				 ConfigReader& cfg); 
		void process    ();
};

#endif
