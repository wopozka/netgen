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

#ifndef polylineH
#define polylineH

#include "RouteParameters.h"
#include "Point.h"
#include <list>
#include <map>
#include <vector>

class PolylineNode {

	public:
		int  node;
		bool border;
		PolylineNode (int n, bool b): node(n), border(b){};
};

class Polyline {
	public:
		int roadID;
		short type;
		short endLevel;
		RouteParameters routeParam;
		char dirIndicator;
		bool markerKind;
		short tmpRoute; // tymczasowa klasa drogi używana przy korekcji dla nodów
		std::string label;
		std::string unknown;
		std::string restrParam;
		std::string roadsignType;
		std::string roadsignPos;
		std::string roadsignAngle;
		
		std::list<Point> points;
//		std::vector<Point> points;
		std::map<int, PolylineNode> nodes;
		bool used;
		Polyline ();
		void pushNode (int p, int n, bool b);
		void output (std::ostream& os);

};

typedef std::list<Polyline> PolylineList;

typedef PolylineList::iterator PolylineListIter;

typedef std::map<int,Polyline> PolylineMap;

typedef PolylineMap::iterator PolylineMapIter;

#endif
