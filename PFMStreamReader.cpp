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

#include "PFMStreamReader.h"

#include <sstream>
#include <algorithm>

using namespace std;

PFMStreamReader::PFMStreamReader (istream& istr): TextStreamReader (istr){
   insidePolyline = false;
   insidePolygon  = false;
   insidePoint    = false;
   insideImgID	  = false;
}

// TODO: przepisać zeby brać dane z cfg
bool PFMStreamReader::isRoutable (int type){
   //return (config.typeParameters[type].isRoutable);
   return ((type > 0) && (type < 0x14)) ||
   	   (type == 0x16) || (type == 0x1a) || (type == 0x1b) || ((type > 0x100) && (type < 0x120));
}

string PFMStreamReader::upCase (const string& s){
   string u (s);

   transform (u.begin (), u.end(), u.begin(), (int(*)(int))toupper);
   return u;
}

string PFMStreamReader::commaIfNotFirst (bool &first){
   if (!first) return ",";
   first = false;
   return "";
}

string PFMStreamReader::pointStartMarker (bool kind){
   if (kind) 
     return "[POI]\n";
   else
     return "[RGN10]\n";	     
}

string PFMStreamReader::polylineStartMarker (bool kind){
   if (kind) 
     return "[POLYLINE]\n";
   else
     return "[RGN40]\n";	     
}

string PFMStreamReader::polylineEndMarker (bool kind){
   if (kind) 
     return "[END]\n";
   else
     return "[END-RGN40]\n";	     
}

string PFMStreamReader::pointEndMarker (bool kind){
   if (kind) 
     return "[END]\n";
   else
     return "[END-RGN10]\n";	     
}

string PFMStreamReader::polygonStartMarker (bool kind){
   if (kind) 
     return "[POLYGON]\n";
   else
     return "[RGN80]\n";	     
}

string PFMStreamReader::polygonEndMarker (bool kind){
   if (kind) 
     return "[END]\n";
   else
     return "[END-RGN80]\n";	     
}

void PFMStreamReader::comment (const string& line){
}

void PFMStreamReader::line (const string& line){
      size_t opening, closing, equal;
      equal   = line.find ('=');
      if (equal != string::npos){
	 string tok, val;
	 tok = line.substr (0,equal);
	 val = line.substr (equal + 1);
	 token (tok, val);
      }
      else {
        opening = line.find ('[');
        closing = line.find (']');
   	 if ((opening != string::npos) && (closing != string::npos)){
   	    string sec = line.substr (opening + 1, closing - opening - 1);
   	    section (sec);
      	 }
	 else
	   comment (line);
      }
}

void PFMStreamReader::section (const string& sec){
   isImgID       = false;
   isImgIDEnd    = false;
   isPoint       = false;
   isPointEnd    = false;
   isPolyline    = false;
   isPolylineEnd = false;
   isPolygon     = false;
   isPolygonEnd  = false;

   if (isEqual (sec, "END")){
       isPointEnd = true;
       isPolylineEnd = true;
       isPolygonEnd = true;
   } else if (isEqual (sec, "POI")){
       isPoint = true;
       markerKind = true;
   } else if (isEqual (sec, "POLYLINE")){
       isPolyline = true;
       markerKind = true;
   } else if (isEqual (sec, "POLYGON")){
       isPolygon = true;
       markerKind = true;
   } else if (isEqual (sec, "RGN10")){
       isPoint = true;
       markerKind = false;
   } else if (isEqual (sec, "RGN40")){
       isPolyline = true;
       markerKind = false;
   } else if (isEqual (sec, "RGN80")){
       isPolygon = true;
       markerKind = false;
   } else if (isEqual (sec, "END-RGN10")){
       isPointEnd = true;
   } else if (isEqual (sec, "END-RGN40")){
       isPolylineEnd = true;
   } else if (isEqual (sec, "END-RGN80")){
       isPolygonEnd = true;
   } else if (isEqual (sec, "IMG ID")){
       isImgID = true;
   } else if (isEqual (sec, "END-IMG ID")){
       isImgIDEnd = true;
   }

   if (isPolyline){
      insidePolyline = true;
      endLevel = -1;
      roadID = -1;
      objectType = -1;
      forceClass = -1;
      forceSpeed = -1;
      dirIndicator = -1;
      routeParam.valid = false;
      numbersFound = false;
      label.clear ();
//      city.clear  ();
      miasto.clear ();
//      plik.clear ();
      datalist.clear ();
      restrParam.clear();
      roadsignType.clear ();
      roadsignPos.clear ();
      roadsignAngle.clear ();

   }
   if (isPolygon){
      insidePolygon = true;
      objectType = -1;
      label.clear ();
      datalist.clear ();
   }
   if (isPoint){
      insidePoint = true;
      label.clear ();
      objectType = -1;
   }
   if (isImgID){
      routing = -1;
      insideImgID = true;
   }
}

bool PFMStreamReader::token (const string& token, const string& value){
   if (isEqual (token,"Type")){
      istringstream is (value);
      unsigned posx = value.find_first_of ("Xx");
      if (posx != string::npos){
        is >> hex >> objectType;
      } else {
        is >> dec >> objectType;
      }
      return true;
   }
   else if (isEqual (token,"DirIndicator")){
      istringstream is (value);
      is >> dirIndicator;
      return true;
   }
   else if (isEqual (token, "EndLevel")){
      istringstream is (value);
      is >> endLevel;
      return true;
   }
   else if (isEqual (token, "RoadID")){
      istringstream is (value);
      is >> roadID;
      return true;
   }
   else if (isEqual (token, "ForceSpeed")){
      istringstream is (value);
      is >> forceSpeed;
      return true;
   }
   else if (isEqual (token, "ForceClass")){
      istringstream is (value);
      is >> forceClass;
      return true;
   }
   else if (isEqual (token, "RouteParam")){
      routeParam.valid = true;
      routeParam.parse (value, true);
      return true;
   }
   else if (isEqual (token, "Sign")){
      roadsignType = value;
      return true;
   }
   else if (isEqual (token, "SignPos")){
      roadsignPos = value;
      return true;
   }
   else if (isEqual (token, "SignAngle")){
      roadsignAngle = value;
      return true;
   }

/*
   else if (isEqual (token, "Plik")){
      plik = value;
      return true;
   }
   else if (isEqual (token, "Miasto")){
      miasto = value;
      return true;
   }
   else if (isEqual (token, "CityName")){
      city = value;
      return true;
   }
*/   
   else if (isEqual (token, "Label")){
      label = value;
      return true;
   }
   else if (isEqual (token, "RestrParam")){
      restrParam = value;
      return true;
   }
   else if (isEqual (token, "Data0")){
      return true;
   }
   else if (token.length() >= 7){
      if (isEqual (token.substr(0,7), "Numbers")){
         numbersFound = true;
      }
   }
   return true;
}
