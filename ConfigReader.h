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

#ifndef ConfigReaderH
#define ConfigReaderH

#include "PFMStreamReader.h"
#include <vector>
#include <string>
#include <map>

typedef std::vector<std::string> vecString ;

class ConfigReader : public PFMStreamReader {
	private:
		bool removedFound;
		int removedTmp, removedLastTmp;
	protected:
		static const int MAXTYPE = 0x120;
		bool token (const std::string& tok, const std::string& val);
	public:
                static const int Z_BLAD         = -1;
                static const int Z_BRAK         = 0;
                static const int Z_RESTRYKCJA   = 1;
                static const int Z_PROSTO       = 2;
                static const int Z_LEWO         = 3;
                static const int Z_PRAWO        = 4;
                static const int Z_ZAWRACANIA   = 5;
		RouteParameters typeParameters [MAXTYPE];
		bool connectorType [MAXTYPE];
		int overrideNullSpeed;
		double epsilonMax;
		double epsilonMin;
		double maskSize;
		double minAngle;
		double lineSearchRadius;
		double routeSearchDistance;
		short lowestClassToCheck;
		short connectorClassesAdjustmentVariant;
		bool checkNetIntegrity;
		bool nodeOnRoadEnd;
		bool adjustConnectorClass;
		bool adjustClassesInNode;
		bool createNodeForRestriction;
		bool printRoadSigns;
		bool nonRoutableBike;
		bool nonRoutablePederestian;
		int numbersType;
		int restrictionType;
		int roadSignType;
		int roadEndType;
		int roadEndTypeLast;
		int removedType;
		int removedTypeLast;
		int noCrossingType;   
		int precision; 		
                bool isCheckOnly;
                std::map<int,vecString> rSigns ;     // znaki drogowe dla restrykcji
		void process ();
		ConfigReader (std::istream& istr);
//		ConfigReader (const ConfigReader& cf);
//		ConfigReader (ConfigReader& cf);
		bool isRoadEnd (int type);
		bool isRemoved (int type);
		bool isConnector (int type);
		bool isRestrictionOrRoadSign (int type);
		bool isRoadSign (int type);
                int findRoadSign (const std::string& st);
                std::string stRoadSign (int type);
		void dumpParameters ();
		bool isRoutable (int type);
};

#endif
