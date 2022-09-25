/*  netgen - autorouting net generator for Polish Format map
 *
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


#include <getopt.h>
#ifdef __WIN32__
#include <io.h>
#endif
#include "NetAnalyzer.h"
#include "RoadIDGenerator.h"
#include "NodeWriter.h"
#include "ConfigReader.h"
#include "Intersection.h"
#include "OziOutput.h"
#include "MapData.h"
#include "datatypes.h"

#include <list>
#include <set>
#include <map>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <cstdlib>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>

using namespace std;

int main (int argc, char **argv){
 
       	
#ifdef GETTEXT	
   setlocale      (LC_MESSAGES, "");
   setlocale      (LC_COLLATE, "") ;
   setlocale      (LC_CTYPE, "");
   bindtextdomain ("netgen", NULL);
   textdomain     ("netgen");
#endif   
		
   cerr << _("Net of Nodes Generator for Polish Format Map.\n");
   cerr << _("Version ") <<  VERSION << " " 
	   << _("May") << " 2012 "  << _("\n");

   int flags = 0;
   string outfilename;
   string tablefilename;
   int opt;	   

   double epsilonMax =  0;
   double epsilonMin = -1;
   double minAngle = 0;
   double lineSearchRadius = 0;
   int restrictionType = 0x19;
   int precision = 6;
   bool epsilonMaxFound = false;
   bool epsilonMinFound = false;
   bool restrictionTypeFound = false;
   bool precisionFound = false;
   bool lineSearchRadiusFound = false;
   bool printSigns = false;
   bool nonRoutableBike = false;
   bool nonRoutablePederestian = false;
  
   while ((opt = getopt (argc, argv, "a:bce:hjko:p:r:Rs:St:T:x?NZ")) != -1){      
      if (opt == 'p'){
	 istringstream is (optarg);
	 is >> precision;
	 precisionFound = true;
      }
      if (opt == 'e'){
	 istringstream is (optarg);
	 is >> epsilonMax;
	 epsilonMaxFound = true;
      }
      if (opt == 'r'){
	 istringstream is (optarg);
	 is >> epsilonMin;
	 epsilonMinFound = true;
      }
      if (opt == 'a'){
	 istringstream is (optarg);
	 is >> minAngle;
      }
      if (opt == 't'){
	 istringstream is (optarg);
	 is >> restrictionType;
	 restrictionTypeFound = true;
      }
      if (opt == 's'){
	 istringstream is (optarg);
	 is >> lineSearchRadius;
	 lineSearchRadiusFound = true;
      }
      switch (opt){
	      case 'o':
		      outfilename = optarg;
		      break;
	      case 'T':
		      tablefilename = optarg;
		      break;
	      case 'b':
		      flags |= FLAG_NODE_ON_ROAD_END;
		      break;
	      case 'R':
		      flags |= FLAG_NODE_ON_RESTRICTION;
		      break;
	      case 'c':
		      flags |= FLAG_CHECK_ONLY;
		      break;
	      case 'j':
		      flags |= FLAG_ADJUST_CONNECTOR_CLASS;
		      break;
	      case 'k':
		      flags |= FLAG_ADJUST_CLASSES_IN_NODE;
		      break;
		      /*
	      case 'm': // lista węzłów przesunietych
		      flags |= FLAG_OUTPUT_MOVED;
		      break;
		      */
	      case 'S':
		      printSigns = true;
		      break;
	      case 'N':
		      flags |= FLAG_SKIP_REGULAR;
		      break;
	      case 'x':
		      flags |= FLAG_FIND_CROSSINGS;
		      break;
	      case 'Z':
			  nonRoutableBike = true;
			  nonRoutablePederestian = true;
		      break;
	      case '?':
	      case 'h':
		      cerr << _("Usage: ") << argv [0] << _(" [parameters]  inputfile > outputfile") << endl;
		      cerr <<   "       "  << argv [0] << _(" [parameters] < inputfile > outputfile") << endl;
		      cerr << _("Optional parameters:\n");
		      cerr << _("  -aAngle find acute angles less than given Angle\n");
		      cerr << _("  -b generate nodes at dead ends.\n");
		      cerr << _("  -R generate nodes in restriction/road sign points.\n");
		      cerr << _("  -c write list of nodes in OziExplorer format (wpt).\n");
		      cerr << _("  -rEpsilonMin check minimum distance between nodes.\n");
		      cerr << _("  -eEpsilonMax join points closer than EpsilonMax.\n");
		      cerr << _("  -j adjust road class of roundabouts, ramps and connectors  \n");
		      cerr << _("  -k adjust road classes to limit number of classes per node\n");
		      cerr << _("  -pN set number of decimal digits in output\n");
//		      cerr << _("  -m write list of joined points\n");
		      cerr << _("  -N exclude regular nodes from list generated by -c\n");
		      cerr << _("  -sRadius exclude dead end nodes form list generated by -c.\n");
		      cerr << _("           when no line in circle of given radius found\n");
		      cerr << _("  -oname output file name\n");
		      cerr << _("  -tType type of line interpreted as restriction\n");
		      cerr << _("  -Tname read assignment table and line types from file\n");
		      cerr << _("  -S analyze restrictions and output road sign hints\n");
		      cerr << _("  -x include intersections without nodes in list generated by -c\n");
//		      cerr << _("  -Z non routable bike and pederastian road\n");
		      exit (0);
      }
   }

   if (argc - optind > 1){
      cerr << _("Too many input files.\n");
      exit (1);
   }
   
   if (argc - optind == 1){
      int inf;
      if ((inf=open(argv[optind], O_RDONLY)) < 0) {
	 cerr << _("Cannot open input file ");
	 cerr << argv[optind] << endl;
	 exit(1);
      }
      close (0);
      (void)dup(inf);
   }

   // -o, reopen stdout to a filename
   if (outfilename.length ()) {
	   int of;
	   if ((of=open(outfilename.c_str(), O_CREAT|O_TRUNC|O_WRONLY, 0600)) < 0) {
		   cerr << _("Cannot open output file for writing.\n");
		   exit(1);
	   }
	   close(1);
	   (void)dup(of);
   }

   ifstream cfg(tablefilename.c_str());
   ConfigReader cfgr (cfg);
   if (cfg){
      cfgr.process ();
      cerr << _("Config file read.\n");
   }
   cfg.close ();

   if (precisionFound)
     cfgr.precision = precision;
   if (restrictionTypeFound)
     cfgr.restrictionType = restrictionType;
   if (epsilonMaxFound)
     cfgr.epsilonMax = epsilonMax;
   if (epsilonMinFound)
     cfgr.epsilonMin = epsilonMin;

   if (minAngle > 0){
     cfgr.minAngle = minAngle;
   }
   if (lineSearchRadiusFound)
     cfgr.lineSearchRadius = lineSearchRadius;
   if (printSigns)
     cfgr.printRoadSigns = true;

   cerr << fixed << setprecision (cfgr.precision);
   cout << fixed << setprecision (cfgr.precision);

   cfgr.dumpParameters();
   
   cfgr.isCheckOnly          = (flags & (FLAG_CHECK_ONLY));
   cfgr.nodeOnRoadEnd        = (flags & (FLAG_NODE_ON_ROAD_END));
   cfgr.adjustConnectorClass = (flags & (FLAG_ADJUST_CONNECTOR_CLASS));
   cfgr.adjustClassesInNode  = (flags & (FLAG_ADJUST_CLASSES_IN_NODE));
   cfgr.adjustClassesInNode  = (flags & (FLAG_ADJUST_CLASSES_IN_NODE));
   cfgr.createNodeForRestriction  = (flags & (FLAG_NODE_ON_RESTRICTION));
   
   cfgr.nonRoutableBike 		= nonRoutableBike;
   cfgr.nonRoutablePederestian 	= nonRoutablePederestian;

   MapData md (cfgr.maskSize);

   RoadIDGenerator ridg (cin, &md, cfgr);
   ridg.process ();

//   md.outputMap (cout); // test print

   cerr << _("Road identifiers generated.\n");
 
   NodeWriter niw (cout, md, cfgr);
   if (cfgr.epsilonMin > 0)
     niw.checkMinimumDistance (cfgr.epsilonMin);
   if ((flags & FLAG_CHECK_ONLY) || (flags &FLAG_FIND_CROSSINGS)){
      OziOutput ozo (cout);
      ozo.Header ();
      if (cfgr.lineSearchRadius > 0){
	 niw.markNodesNearRoad (cfgr.lineSearchRadius);
      }
      if (flags & FLAG_FIND_CROSSINGS){
	 Intersection *ints = new Intersection ();
	 ints->alignNodes (md.lineList, md.pointmap ,md.nodes); 
	 ints->findAll (md.lineList);
	 ints->removePoints (md);
//	 cerr << "md.noCrossings.size()=" << md.noCrossings.size() << endl;
	 ozo.Intersections (ints->intersections, md);
	 delete ints;
      }
      if (flags & FLAG_CHECK_ONLY){
	 ozo.Nodes (md.nodes, niw.getFirstEndNodeID(),
		    md, flags & FLAG_SKIP_REGULAR, cfgr.lineSearchRadius > 0);
      }
      if (cfgr.minAngle > 0){
	 ozo.List (md.acuteAngles,"A");
      }
      if (cfgr.checkNetIntegrity){
	 niw.process ();
	 NetAnalyzer netan (md, cfgr);
	 list<Point> errors;
	 cerr << "routeSearchDistance=" << cfgr.routeSearchDistance << endl;
	 for (int i = 4; i >= cfgr.lowestClassToCheck ; i--){
	    cerr << "Searching for holes in routes of class " << i;
	    int n = errors.size();
   	    netan.findLowClassHoles (i, errors); 
	    cerr << " Found: " << errors.size() - n << endl;
	 }
	 ozo.List (errors, "H");
//	 niw.generateIncidenceMap();
//	 list<Point> errors2;
//	 netan.checkContinuity(Point (52.119780,19.931900),Point(52.087670,19.920160),
//	 netan.checkContinuity(Point (52.119780,19.931900),Point(51.925100,19.643940),
//	 netan.checkContinuity(Point (52.119780,19.931900),Point(51.590490,19.143980),
//			       3,"~[0x05]14");
//	 netan.checkContinuity(Point (51.542270,14.728370),Point(51.442120,21.973990),
//			       3,"~[0x2e]12", errors2);
//	 ozo.List(errors2, "ERR");
      }
      ozo.Unused (md, flags & FLAG_FIND_CROSSINGS, flags & FLAG_CHECK_ONLY);
   }
   else {
	niw.process ();
	niw.outputMap ();
   }
   
}
