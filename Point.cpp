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

#include "Point.h"


#include <sstream>
#include <iomanip>
#include <cmath>

using namespace std;


double Point::angle (const Point& p) const {
  double dx = p.x - x ;
  double dy = (p.y - y ) * cos(deg2rad((p.x + x)/2));
  if ( dx == 0 && dy == 0 )
    return 0;
  return atan2 ( dy, dx );
}



Point::Point (){
   x = y = INVALID;
}

Point::Point (double xp, double yp){
   x = xp; y = yp;
}

bool Point::operator< (const Point& p) const {
   if (x != p.x) return (x < p.x);
   else return (y < p.y);
}


bool Point::lessT (const Point& p, double threshold) const {
   if (x < p.x - threshold) return true;
   if (x > p.x + threshold) return false;
   if (y < p.y - threshold) return true;   
   return false;
}

double Point::sqrDistance (const Point& p) const {
   double dx = x - p.x;
   double dy = y - p.y;
   return (dx * dx) + (dy * dy);
}

double Point::metDistance (const Point& p) const {
// odleglosc w metrach
   double dx = (x - p.x) * DEG2M ;
   double dy = (y - p.y) * DEG2M * cos(deg2rad((x+p.x)/2));
   return sqrt((dx * dx) + (dy * dy));
}




void Point::shift(double distance, double direction) {
// przesuniecie punkt o disance metrow w kierunku direction
// direction w radianach jak azymut na kompasie E=pi/2 S=pi W=3/2pi
   x = x + distance/DEG2M * cos ( direction ) ;
   y = y + distance/DEG2M * sin ( direction ) / cos ( deg2rad( x ));
}

bool Point::operator== (const Point& p){

   return (x == p.x) && (y == p.y);
}

void Point::operator= (const Point& p){
   x = p.x; y = p.y;
}

bool Point::valid (){
   return ((x >= -90) && (x <= 90) && (y >= -180) && (y <= 180));
}

ostream& operator<< (ostream& os, const Point& p){
   os << "("  << p.x << "," << p.y << ")";
   return os;
}

istream& operator>> (istream& is, Point& p){
   char separator;
   is >> separator;
   is >> p.x;
   is >> separator;
   is >> p.y;
   is >> separator;
   return is;
}

void Points::addFromString (const string& coords){
   Point previousPoint; 
   istringstream iv (coords);
   while (iv){
      Point p;
      iv >> p;
      if (p.valid()){
//	 
	 if (previousPoint == p){
//	    cerr << "addFromString: pominięty powielony punkt\n";
	 }
	 else
//	   
	   push_back (p);
	 previousPoint = p;
      }
      iv.ignore (100,',');
   }
}
#if 0
void Point::test (){
   Point p1 (10,10);
   for (double y = 10 - 2 * TEST_EPS; y < 10 + 2* TEST_EPS + CMP_EPS; y += TEST_EPS ){
      for (double x = 10 - 2* TEST_EPS; x < 10 + (2* TEST_EPS) + CMP_EPS; x += TEST_EPS ){
	// cerr << setw (10) << setprecision (8) << x << "," << setw (10) << setprecision (8)<< y;
	 Point p2 (x,y);
	 if (p2 < p1){
	    if (p1 < p2){
	       cerr << "?";
	    }
	    else {
	       cerr << "<";
	    }
	 }
	 else {
	    if (p1 < p2){
	       cerr << ">";
	    }
	    else {
	       cerr << "=";
	    }
	 }
	 cerr << " ";
      }

      cerr << endl;
   }


}
#endif
