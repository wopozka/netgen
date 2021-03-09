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

#ifndef PFMStreamReaderH
#define PFMStreamReaderH

#include "RouteParameters.h"
#include "TextStreamReader.h"
#include "datatypes.h"
#include "Point.h"
#include <list>
#include <string>
#include <vector>
#include <cstring>

class PFMStreamReader : public TextStreamReader {
	protected:
                static std::vector<Points> pointBoxes;
		int  objectType;
		int  roadID;
		int  dirIndicator;
		int  routing;
		bool insidePolyline;
		bool insidePolygon;
		bool insidePoint;
		bool insideImgID;
		bool markerKind; // [POLYLINE] czy [RGN40]
		RouteParameters routeParam;
		short forceClass;
		short forceSpeed;
		bool numbersFound;
		int endLevel;
		std::string label;
//		std::string city;
		std::string miasto;
//		std::string plik;
		std::string roadsignType;
		std::string roadsignPos;
		std::string roadsignAngle;

		std::string restrParam;
		std::list<Points> datalist; // lista elementów Data[0-9]
		
		std::string upCase (const std::string& s);

		bool isImgID       ;
		bool isImgIDEnd    ;
		bool isPoint       ;
		bool isPointEnd    ;
		bool isPolyline    ;
		bool isPolylineEnd ;
		bool isPolygon     ;
		bool isPolygonEnd  ;
                bool isNotTokenData0;

                inline bool isEqual (const std::string& s1,
                                    const char *  s2){
                    return strcasecmp(s1.c_str(),s2)==0;
                }

		std::string polylineStartMarker (bool kind);
		std::string pointStartMarker    (bool kind);
		std::string polygonStartMarker  (bool kind);
		std::string polylineEndMarker   (bool kind);
		std::string pointEndMarker      (bool kind);
		std::string polygonEndMarker    (bool kind);
		
		virtual void comment (const std::string& line);
		virtual void line    (const std::string& line);
		virtual void section (const std::string& s);
		virtual bool token   (const std::string& t,
				      const std::string& v)=0;   
	public:
		static bool isRoutable    (int type);
		static std::string commaIfNotFirst (bool &first);
		PFMStreamReader (std::istream& istr);
};

#endif
