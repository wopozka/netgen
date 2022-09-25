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

#include "ConfigReader.h"

#include "netgen.h"
#include <sstream>
#include <iostream>

using namespace std;

ConfigReader::ConfigReader (istream& istr): PFMStreamReader (istr){
	for (int t=0; t<MAXTYPE; t++){
		typeParameters [t].speed = 0;
		typeParameters [t].route = 0;
		typeParameters [t].toll         = false;
		typeParameters [t].noEmergency  = false;
		typeParameters [t].noDelivery   = false;
		typeParameters [t].noCar        = false;
		typeParameters [t].noBus        = false;
		typeParameters [t].noTaxi       = false;
		typeParameters [t].noPedestrian = false;
		typeParameters [t].noBicycle    = false;
		typeParameters [t].noTruck      = false;
		connectorType [t] = false;
	}
	typeParameters [1] .speed = 6;
	typeParameters [1] .route = 4;
	typeParameters [2] .speed = 5;
	typeParameters [2] .route = 3;
	typeParameters [3] .speed = 4;
	typeParameters [3] .route = 2;
	typeParameters [4] .speed = 4;
	typeParameters [4] .route = 2;
	typeParameters [5] .speed = 3;
	typeParameters [5] .route = 1;
	typeParameters [6] .speed = 2;
	typeParameters [6] .route = 0;
	typeParameters [7] .speed = 2;
	typeParameters [7] .route = 0;
	typeParameters [8] .speed = 2;
	typeParameters [8] .route = 0;
	typeParameters [9] .speed = 2;
	typeParameters [9] .route = 0;
	typeParameters [10].speed = 2;
	typeParameters [10].route = 0;
	typeParameters [11].speed = 6;
	typeParameters [11].route = 4;
	typeParameters [12].speed = 3;
	typeParameters [12].route = 1;

	for (int t=1; t < MAXTYPE ;t++) {
		if (!( ((t > 0) && (t < 0x14)) ||
				(t == 0x16) || (t == 0x1a) || (t == 0x1b)) ) {
			continue;
		}
		typeParameters [t].isRoutable=true;
	}

	overrideNullSpeed = 0;
	epsilonMax =  0;
	epsilonMin = -1;
	numbersType = 0x2a;
	restrictionType = 0x19;
	roadSignType = 0x2f;
	precision = 6;
	roadEndType = 0x1709;
	roadEndTypeLast = roadEndType;
	removedType = roadEndType;
	removedTypeLast = removedType;
	noCrossingType = 0x1708;
	maskSize = 0.00001;
	minAngle = 0;
	lineSearchRadius = 0;
	isCheckOnly = false;
	removedFound = false;
	routeSearchDistance = 0.01000;
	lowestClassToCheck = 2;
	checkNetIntegrity = true;
	connectorClassesAdjustmentVariant = 1;
	nodeOnRoadEnd = false;
	adjustConnectorClass = false;
	adjustClassesInNode = false;
	createNodeForRestriction = false;
	printRoadSigns = false;
    nonRoutableBike = false;
    nonRoutablePederestian = false;

}


bool ConfigReader::token (const string& tok, const string &val){
	istringstream is (val);
	if (tok.length () >= 5){
		if (isEqual (tok.substr (0,4), "Type")){
			istringstream ist (tok.substr(4));
			int type = -1;
			ist >> hex >> type;
			//cerr << "typ " << type << " : " << val << "\n" ;
			if ((type >= 0) && (type < MAXTYPE)){
				typeParameters[type].parse (val, false);
			}
		}
	}
	if (tok.length () >= 5){
		if (isEqual (tok.substr (0,5), "RSign")){
			int idx = -1;
			char c = 'E';
			is >> idx;
			vecString vs;
			rSigns[idx]= vs;
			is >> c;
			if ((idx >= 0) && (idx < MAXTYPE) && c==','){
				string st;
				while (is.good()){
					getline(is,st,',');
					rSigns[idx].push_back(st);
				}
			} else {
				cerr << "Error parsing parm file: " << tok << "=" << val << "\n";
			}
		}
	}
	if (isEqual (tok, "OVERRIDENULLSPEED")){
		is >> overrideNullSpeed;
	}
	if (isEqual (tok, "LINESEARCHRADIUS")){
		is >> lineSearchRadius;
	}
	if (isEqual (tok, "ANGLEMIN")){
		is >> minAngle;
	}
	if (isEqual (tok, "EPSILONMIN")){
		is >> epsilonMin;
	}
	if (isEqual (tok, "EPSILONMAX")){
		is >> epsilonMax;
	}
	if (isEqual (tok, "MASKSIZE")){
		is >> maskSize;
	}
	if (isEqual (tok, "NUMBERS")){
		is >> hex >> numbersType;
	}
	if (isEqual (tok, "ROADSIGN")){
		is >> hex >> roadSignType;
	}
	if (isEqual (tok, "ROADSIGNHINTS")){
		is >> printRoadSigns;
	}
	if (isEqual (tok, "RESTRICTION")){
		is >> hex >> restrictionType;
	}
	if (isEqual (tok, "ROADEND")){
		unsigned pos = val.find ('-');
		if (pos > 0 && pos < val.length()-1){
			istringstream isl (val.substr (0,pos));
			istringstream isr (val.substr (pos + 1));

			isl >> hex >> roadEndType;
			isr >> hex >> roadEndTypeLast;
		}
		else {
			is >> hex >> roadEndType;
			roadEndTypeLast = roadEndType;
		}
	}
	if (isEqual (tok, "REMOVED")){
		removedFound = true;
		unsigned pos = val.find ('-');
		if (pos > 0 && pos < val.length()-1){
			istringstream isl (val.substr (0,pos));
			istringstream isr (val.substr (pos + 1));

			isl >> hex >> removedTmp;
			isr >> hex >> removedLastTmp;
		}
		else {
			is >> hex >> removedTmp;
			removedLastTmp = removedTmp;
		}
	}
	if (isEqual (tok, "NOCROSSING")){
		is >> hex >> noCrossingType;
	}
	if (isEqual (tok, "PRECISION")){
		is >> precision;
	}
	if (isEqual (tok, "LOWESTCLASSTOCHECK")){
		is >> lowestClassToCheck;
	}
	if (isEqual (tok, "CONNECTORCLASSESADJUSTMENTVARIANT")){
		is >> connectorClassesAdjustmentVariant;
	}
	if (isEqual (tok, "CHECKNETINTEGRITY")){
		is >> checkNetIntegrity;
	}
	if (isEqual (tok, "ROUTESEARCHDISTANCE")){
		is >> routeSearchDistance;
	}
	if (isEqual (tok, "CONNECTORTYPES")){
		while (!is.fail()){
			int t = -1;
			is >> hex >> t;
			if (!is.fail()){
				if ((t > 0) && (t < MAXTYPE)){
					connectorType[t] = true;
					is.ignore(100,',');
				}
			}
		}
	}
}


void ConfigReader::process (){
	PFMStreamReader::process();
	if (removedFound){
		removedType = removedTmp;
		removedTypeLast = removedLastTmp;
	} else {
		removedType = roadEndType;
		removedTypeLast = roadEndTypeLast;
	}
}

bool ConfigReader::isRoadSign (int type){
	return (type == roadSignType);
}

bool ConfigReader::isRestrictionOrRoadSign (int type){
	if (type == restrictionType) return true;
	return isRoadSign (type);
}

bool ConfigReader::isRoadEnd (int type){
	if (type < roadEndType) return false;
	if (type > roadEndTypeLast) return false;
	return true;
}

bool ConfigReader::isRemoved (int type){
	if (type < removedType) return false;
	if (type > removedTypeLast) return false;
	return true;
}

bool ConfigReader::isConnector (int type){
	if (type >= MAXTYPE)
		return false;
	if (type < 0)
		return false;
	return connectorType[type];
}

int ConfigReader::findRoadSign (const string& st){
	map<int,vecString>::iterator it;
	string u = upCase(st);

	for (it=rSigns.begin(); it!=rSigns.end() ; it++)
		for( unsigned int t2=0 ; t2 < (*it).second.size() ; t2++)
			if ((*it).second[t2] == st)
				return (*it).first;
	return ConfigReader::Z_BLAD;

}
string ConfigReader::stRoadSign (int type){
	string blad="BLAD";
	if( rSigns.find(type) == rSigns.end() )
		return blad;
	return rSigns[type][0];
}


void ConfigReader::dumpParameters (){

	cerr << "Precision=" << precision << endl;
	if (lineSearchRadius > 0){
		cerr << "LineSearchRadius=" << lineSearchRadius << endl;
	}

	if (minAngle > 0){
		cerr << "MinAngle=" << minAngle << endl;
	}
	cerr << "EpsilonMax=" << epsilonMax << endl;
	if (epsilonMin > 0){
		cerr << "EpsilonMin=" << epsilonMin << endl;
	}
	cerr << "MaskSize=" << maskSize << endl;

	cerr << showbase << hex << _("Road Sign line type: ")
			   << roadSignType << noshowbase << dec << endl;

	cerr << showbase << hex << _("Restriction line type: ")
			   << restrictionType << noshowbase << dec << endl;

	cerr << showbase << hex << _("Flyover intersection masking point type ")
			   << noCrossingType << noshowbase << dec << endl;

	cerr << showbase << hex << _("Dead-end masking point types ")
			   << roadEndType << "-" << roadEndTypeLast << noshowbase << dec << endl;

	cerr << showbase << hex << _("Removed point types ")
			   << removedType << "-" << removedTypeLast << noshowbase << dec << endl;

	cerr << showbase << hex << _("StreetNum line type ")
			   << numbersType << noshowbase << dec << endl;

	for (int t=0; t<MAXTYPE; t++){
		if ( ! (typeParameters [t].speed == 0 &&
				typeParameters[t].route == 0  &&
				typeParameters[t].noCar == 0  &&
				typeParameters[t].noPedestrian == 0 &&
				typeParameters[t].noBicycle ==0 &&
				typeParameters[t].noBus == 0

		)) {

			cerr << "Routeparam" << hex << t << dec << "=";
			cerr << typeParameters [t].speed << ","
					<< typeParameters [t].route;

			cerr << ",D";

			cerr << (typeParameters[t].toll         ? ",1" : ",0");
			cerr << (typeParameters[t].noEmergency  ? ",1" : ",0");
			cerr << (typeParameters[t].noDelivery   ? ",1" : ",0");
			cerr << (typeParameters[t].noCar        ? ",1" : ",0");
			cerr << (typeParameters[t].noBus        ? ",1" : ",0");
			cerr << (typeParameters[t].noTaxi       ? ",1" : ",0");
			cerr << (typeParameters[t].noPedestrian ? ",1" : ",0");
			cerr << (typeParameters[t].noBicycle    ? ",1" : ",0");
			cerr << (typeParameters[t].noTruck      ? ",1" : ",0");
			cerr << "\n";
		}
	}
	cerr << "OverrideNullSpeed=" << overrideNullSpeed << endl;

	cerr << "Connector types: ";
	for (int t = 0; t < MAXTYPE; t++){
		if (isConnector(t)){
			cerr << hex << "0x" << t << dec << " ";
		}
	}
	cerr << endl;
	map<int,vecString>::iterator it;
	for (it=rSigns.begin(); it!=rSigns.end() ; it++){
		cerr << "RSignidx:" << (*it).first ;
		for( unsigned int t2=0 ; t2 < (*it).second.size() ; t2++)
			cerr  << "," << (*it).second[t2];
		cerr << endl;
	}
	cerr << endl;
}

// TODO: przepisać zeby brać dane z cfg
bool ConfigReader::isRoutable (int type){
   //return (config.typeParameters[type].isRoutable);
   if (nonRoutableBike && type == 0xd) {
	   return false;
   }
   if (nonRoutablePederestian && type == 0x16){
	   return false;
   }

   return ((type > 0) && (type < 0x14)) ||
   	   (type == 0x16) || (type == 0x1a) || (type == 0x1b) || ((type > 0x100) && (type < 0x120));
}
