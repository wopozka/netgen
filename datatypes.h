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

#ifndef datatypesH
#define datatypesH

#include "RouteParameters.h"
#include "Point.h"
#include <list>
#include <map>
#include <set>

#define FLAG_ROAD_END      1
#define FLAG_ROAD_BEGIN    2
// węzeł jest węzłem brzegowym mapy:
#define FLAG_MAP_BOUND     4
#define FLAG_RESTRICTION   8
#define FLAG_ALIGNED	  16
#define FLAG_CLOSE	  32
#define FLAG_HAS_ENTRANCE 64
#define FLAG_HAS_EXIT    128 
#define FLAG_ROAD_MIDPOINT 256
#define FLAG_HAS_ENTRANCE_AND_EXIT (FLAG_HAS_ENTRANCE | FLAG_HAS_EXIT)
#define FLAG_NEAR_ROAD 512

/* klasa obiektów przechowujących dane o drodze przyporządkowanej węzłowi */

class Road {
	public:
		int id;   // identyfikator drogi
		int node; // numer punktu w drodze, który pokrywa się z węzłem
		double length; // odległość od początku drogi
		bool endpoint; // punkt nalezacy do wezla jest pierwszym
		               // lub ostatnim w drodze
		Road (int road_id, int road_node, double len=0, bool endpt = false):
			id   (road_id), 
			node (road_node), 
			length (len),
			endpoint (endpt)
   { };
};


typedef std::list<Road> Roadlist;

typedef Roadlist::iterator RoadlistIter;

class PointParameters {
	public:
		int id;      // identyfikator węzła, do którego należy punkt 
		Roadlist roads; // lista przechodzących przez punkt dróg
		unsigned short  flags; // czy punkt jest pierwszym lub ostatnim w drodze
		                      // czy punkt jest punktem brzegowym mapy
		PointParameters (): id (0), roads(), flags(0) {};
};

typedef std::map<Point, PointParameters > Pointmap;

typedef Pointmap::iterator PointmapIter;

class RoadParameters {
	public:
		int node;  // węzeł w sieci
		int point; // punkt w drodze
		unsigned short flags; // rodzaj punktu
		double length;
		RoadParameters (int n, int p, unsigned short fl, double len=0): 
			node (n), point (p), flags(fl), 
			length (len) {};
		RoadParameters ():
			node (0), point (0), flags(0),
			length (0) {};
};

typedef int NodeID; 

class Incidence {
	public:
		int roadID;
		int segment;
		bool dir; // true jesli polaczenie zgodne z kierunkiem drogi
		Incidence (int r, int s, bool d = true):roadID(r), segment(s), dir(d){};
};

typedef std::multimap<NodeID, Incidence> IncidenceMap;

typedef IncidenceMap::iterator IncidenceMapIter;

class NodeParameters {
	public:
		Point p;
		int    n; // liczba dróg w węźle
		Roadlist roads; // lista przechodzących przez punkt dróg
		IncidenceMap incidences;
		short highestRoute;
		short averageRoute;
		unsigned short flags;
		NodeParameters () : p(0,0), n(0),
		           highestRoute(-1), averageRoute(-1),flags(0) {};
};

typedef int RoadID;

typedef std::multimap<RoadID, RoadParameters> Roadmap;

typedef std::map <NodeID, NodeParameters> Nodemap;

typedef Nodemap::iterator NodemapIter;

typedef std::set <Point> Pointset;

typedef Pointset::iterator PointsetIter;

typedef std::multimap <Point, bool> MaskingPointmap;

typedef MaskingPointmap::iterator MaskingPointmapIter;

class MapSection {
	public:
		int objectId;     // gdy 0 treść sekcji jest w stringu body
		                  // gdy != 0 sekcja została sparsowana
		std::string body;
		MapSection (const std::string& buf) : objectId(0), body(buf){

		};
		MapSection (int id) : objectId(id){

		};
		MapSection (int id, const std::string& buf) : objectId(id), body(buf){

		};
};

typedef std::list<MapSection> MapSectionList;

typedef MapSectionList::iterator MapSectionListIter; 

typedef std::map<int, std::set<int> > ConnectionMap;

typedef ConnectionMap::iterator ConnectionMapIter;

#endif
