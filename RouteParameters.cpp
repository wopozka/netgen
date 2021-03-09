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

#include "RouteParameters.h"

#include <string>
#include <sstream>
#include <iostream>

using namespace std;

void RouteParameters::outputRouteRestrictions (std::ostream &os, bool withOneWay){
   if (withOneWay){
      os << (oneWay         ? ",1" : ",0");
   }
   os << (toll         ? ",1" : ",0");
   os << (noEmergency  ? ",1" : ",0");
   os << (noDelivery   ? ",1" : ",0");
   os << (noCar        ? ",1" : ",0");
   os << (noBus        ? ",1" : ",0");
   os << (noTaxi       ? ",1" : ",0");
   os << (noPedestrian ? ",1" : ",0");
   os << (noBicycle    ? ",1" : ",0");
   os << (noTruck      ? ",1" : ",0");
   os << "\n";
}

void RouteParameters::outputSpeedClass (std::ostream &os){
   os << speed << "," << route; 
}

void RouteParameters::storeBool (bool& storage, int value){
   if ((value == 0) || (value == 1))
     storage = (value == 1);
}

void RouteParameters::parse (const string& buf, 
			     bool withOneWay){
   istringstream is (buf);
   int _oneWay = -1;
   int _speed = -1, _route = -1;
   int _toll = -1, _noEmergency = -1, _noDelivery = -1;
   int _noCar = -1, _noBus = -1, _noTaxi = -1;
   int _noPedestrian = -1, _noBicycle = -1, _noTruck = -1;
   is >> _speed;         is.ignore (100,',');
   is >> _route;         is.ignore (100,',');
   if (withOneWay){
      is >> _oneWay;
      is.ignore (100,',');
   }
   is >> _toll;          is.ignore (100,',');
   is >> _noEmergency;   is.ignore (100,',');
   is >> _noDelivery;    is.ignore (100,',');
   is >> _noCar;         is.ignore (100,',');
   is >> _noBus;         is.ignore (100,',');
   is >> _noTaxi;        is.ignore (100,',');
   is >> _noPedestrian;  is.ignore (100,',');
   is >> _noBicycle;     is.ignore (100,',');
   is >> _noTruck;       is.ignore (100,',');
   if  ((_speed >= 0) && (_speed <= 7))
     speed = _speed;
   if ((_route >= 0) && (_route <= 4))
     route = _route;
   storeBool (oneWay, _oneWay);
   storeBool (toll, _toll);
   storeBool (noEmergency , _noEmergency );
   storeBool (noDelivery  , _noDelivery  );
   storeBool (noCar       , _noCar       );
   storeBool (noBus       , _noBus       );
   storeBool (noTaxi      , _noTaxi      );
   storeBool (noPedestrian, _noPedestrian);
   storeBool (noBicycle   , _noBicycle   );
   storeBool (noTruck     , _noTruck     );
   storeBool (isRoutable  , true );
}

void RouteParameters::clear(){	
   oneWay = 0;
   toll = 0;
   noEmergency = 0;
   noDelivery  = 0;
   noCar       = 0;
   noBus       = 0;
   noTaxi      = 0;
   noPedestrian= 0;
   noBicycle   = 0;
   noTruck     = 0;
   isRoutable  = 0;
}
