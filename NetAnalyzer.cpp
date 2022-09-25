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
#include "NetAnalyzer.h"

#include <algorithm>
#include <set>
#include <vector>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cmath>

using namespace std;


bool NetAnalyzer::checkNextNode (short route, Point& startPoint, 
		int startRoadID, int roadID,
		int nodeID, int depth, double distance,
		list<int> history){

	if (depth > 4){
		//      cerr << "Max depth reached";
		return false;
	}

	history.push_back(roadID);

	bool found = false;

	//   cerr << depth << ": " << roadID << endl;

	NodemapIter ni = nodes.find(nodeID);
	if (ni != nodes.end()){

		distance += sqrt(ni->second.p.sqrDistance(startPoint));

		if (distance > config.routeSearchDistance){
			return false;
		}

		for (RoadlistIter ri=ni->second.roads.begin(); ri!=ni->second.roads.end(); ri++){
			PolylineMapIter pmi = mapData.lines.find(ri->id);
			if (pmi != mapData.lines.end()){

				if ((roadID != ri->id) && (startRoadID != ri->id)){
					if (pmi->second.routeParam.route >= route){
						// Jest kontynuacja drogą pierwotnej klasy
						//		  cerr << " Found! " << ni->second.p <<
						//			  " " << distance <<
						//			  "\n";
						//		     errors.push_back(ni->second.p);
						found = true;
						/*
		  list<int>::iterator li;
		  for (li = history.begin(); li != history.end(); li++){
		     cerr << *li << ",";
		  }
		  cerr << ri->id << ":" << distance << endl;
						 */
					}
					map<int, PolylineNode>::iterator ndi;
					ndi = pmi->second.nodes.begin();
					while (ndi != pmi->second.nodes.end()){
						if (ndi->second.node != nodeID){
							if (checkNextNode (route, startPoint, startRoadID, ri->id, ndi->second.node, depth + 1, distance, history)){
								found = true;
							}
						}
						ndi++;
					}
				}

			} else {
				cerr << "Polyline " << ri->id << " not found\n";
			}
		}

	}
	else {
		cerr << "checkNextNode: Node " << nodeID << " not found\n";
	}
	return found;
}

bool NetAnalyzer::checkEndNode (short route, Point& startPoint, int roadID, int nodeID){
	//   cerr << "S: " << roadID;

	list<int> history;

	history.push_back(roadID);

	bool found = false;
	NodemapIter ni = nodes.find(nodeID);
	if (ni != nodes.end()){

		for (RoadlistIter ri=ni->second.roads.begin(); ri!=ni->second.roads.end(); ri++){
			PolylineMapIter pmi = mapData.lines.find(ri->id);
			if (pmi != mapData.lines.end()){

				if (roadID != ri->id){
					if (pmi->second.routeParam.route >= route){
						// Jest kontynuacja drogą tej samej lub wyższej klasy
						return false;
					}
				}

			} else {
				cerr << "Polyline " << ri->id << " not found\n";
			}
		}

		for (RoadlistIter ri=ni->second.roads.begin();
				ri!=ni->second.roads.end(); ri++){
			PolylineMapIter pmi = mapData.lines.find(ri->id);
			if (pmi != mapData.lines.end()){

				if (roadID != ri->id){
					map<int, PolylineNode>::iterator ndi;
					ndi = pmi->second.nodes.begin();
					while (ndi != pmi->second.nodes.end()){
						if (ndi->second.node != nodeID){
							//	     	     cerr << " Start " << ni->second.p << "\n";
							if (checkNextNode (route, startPoint, roadID, ri->id, ndi->second.node, 0, 0, history)){
								found = true;
							}
						}
						ndi++;
					}
				}

			} else {
				cerr << "Polyline " << ri->id << " not found\n";
			}
		}

	}
	else {
		cerr << "checkEndNode: Node " << nodeID << " not found\n";
	}
	return found;
}

void NetAnalyzer::findLowClassHoles (short route, std::list<Point>& errors){
	// dla wszystkich końców dróg wskazanej (wysokiej) klasy
	// jeśli wśród innych dróg z końcowego węzła znajdziesz
	// inną drogę tej samej klasy - zakończ poszukiwania dla tego węzła
	// dla każdej odchodzącej od węzła drogi znajdź węzły z którymi się
	// łączy sprawdź czy w tych węzłach nie ma drogi klasy
	// od której zaczęliśmy szukanie. Jeśli tak zaraportuj ten węzeł
	// wraz z pierwszym węzłem jako znaleziony odcinek niższej klasy
	// Operację można powtarzać dla kolejnych węzłów

	for (MapSectionListIter mi = mapData.sections.begin(); mi != mapData.sections.end();mi++){
		if (mi->objectId){
			if (!config.isRestrictionOrRoadSign(mapData.lines[mi->objectId].type)){

				if (mapData.lines[mi->objectId].routeParam.route == route){
					if (mapData.lines[mi->objectId].nodes.size() > 0){
						if (checkEndNode (route,
								mapData.lines[mi->objectId].points.front(),
								mi->objectId,
								mapData.lines[mi->objectId].nodes.begin()->second.node)){
							errors.push_back(mapData.lines[mi->objectId].points.front());
						}

						//	       cerr << endl;
						if (mapData.lines[mi->objectId].nodes.size() > 1){
							if (checkEndNode (route,
									mapData.lines[mi->objectId].points.back(),
									mi->objectId,
									mapData.lines[mi->objectId].nodes.rbegin()->second.node)){
								errors.push_back(mapData.lines[mi->objectId].points.back());
							}
						}
						//	       cerr << endl;
					}
				}
			}
		}
	}
}


NetAnalyzer::NetAnalyzer (MapData& md,  
		ConfigReader& cfg):
		mapData (md),
		config (cfg),
		nodes (md.nodes)
/*
: 
	flags(flg),
	os (ostr),
	points (md->pointmap),
	streetNum (md)
 */
{
}

bool NetAnalyzer::checkContinuity (int startNode, int endNode, const string& label, short route, set<int>& roadSoFar, int depth, int depthAny, list<Point>& errors){

	static bool debug = false;
	// ============
	// kierunkowość
	// ============
	// any road odpowiedniej klasy

	depth++;
	depthAny++;
	//   cerr << "depth: " << depth << " depthAny: " << depthAny << endl;

	if (depth > 24){
		cerr << "max depth reached\n";
		return false;
	}
	int currentNode = startNode;
	while (currentNode != endNode){
		if (currentNode == 225099){
			debug = true;
		}
		if (depthAny > 24){
			cerr << "max depthAny reached\n";
			return false;
		}
		roadSoFar.insert (currentNode);
		errors.push_back (mapData.nodes[currentNode].p);
		cerr << "Current node: " << currentNode << endl;
		//      cerr << "Nuber of nodes: " << roadSoFar.size() << endl;
		set<int> nextNodes;
		set<int> nextNodesAnyRoad;
		IncidenceMapIter imi = mapData.nodes[currentNode].incidences.begin();
		while (imi != mapData.nodes[currentNode].incidences.end()){
			const string& cLabel (mapData.lines[imi->second.roadID].label);
			short cRoute (mapData.lines[imi->second.roadID].routeParam.route);
			short cType (mapData.lines[imi->second.roadID].type);
			if (debug){
				cerr << "checking: " << cLabel << " "
						<< imi->first << " "
						//	   	    <<  mapData.lines[imi->second.roadID].routeParam.route << " "
						<< endl;
			}
			if (roadSoFar.find(imi->first) == roadSoFar.end()){
				if (debug){
					cerr << "so far - not found" << endl;
				}
				//	    if (cRoute >= route){
				cerr << cLabel << " "
						<<  cRoute << " "
						<< endl;
				if ((cLabel.find(label) != string::npos)
				){
					if (debug){
						cerr << "label found" << endl;
					}
					nextNodes.insert (imi->first);
				} else {
					if (config.isConnector(cType)){
						nextNodesAnyRoad.insert (imi->first);
					}
				}
				//	    }
			}

			imi++;
		}
		if (nextNodes.size() > 0) {
			set<int>::iterator nni = nextNodes.begin();
			currentNode = *(nni);
			nni++;
			while (nni != nextNodes.end()){
				if (checkContinuity (*nni, endNode, label, route, roadSoFar, depth, 0, errors)){
					return true;
				}
				nni++;
			}
			depthAny = 0;
		} else {
			//	 cerr << "Continuation by any road\n";
			if (nextNodesAnyRoad.size() > 0) {
				set<int>::iterator nni = nextNodesAnyRoad.begin();
				currentNode = *(nni);
				nni++;
				while (nni != nextNodesAnyRoad.end()){
					if (checkContinuity (*nni, endNode, label, route, roadSoFar, depth, depthAny, errors)){
						return true;
					}
					nni++;
				}
				depthAny++;
			} else {
				cerr << "Couldn't find road continuation at node: " << currentNode << "\n";
				return false;
			}
		}
	}
	return true;
}

void NetAnalyzer::checkContinuity (const Point& startPoint, const Point& endPoint, 
		short route, const std::string& label, list<Point>& errors){


	int startNode, endNode;


	PointmapIter pmi = mapData.pointmap.find(startPoint);

	if (pmi == mapData.pointmap.end()){
		cerr << "Start node not found\n";
		return;
	}

	startNode = pmi->second.id;

	pmi = mapData.pointmap.find(endPoint);

	if (pmi == mapData.pointmap.end()){
		cerr << "End node not found\n";
		return;
	}

	endNode = pmi->second.id;

	cerr << "Checking continuity of road " << label << endl;


	set<int> roadSoFar;

	if (checkContinuity (startNode, endNode, label, route, roadSoFar, 0, 0, errors)){

		cerr << "Road is not interrupted\n";

	}
}
