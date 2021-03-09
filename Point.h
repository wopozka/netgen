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

#ifndef PointH
#define PointH

#include <iostream>
#include <list>

#define PI 3.14159265358979323846
// dlugosc jednego stopnia w metrach
#define DEG2M ( 40041000 /360 )

inline double deg2rad(double deg) {
  return (deg * PI / 180);
}

inline double rad2deg(double rad) {
  return (rad * 180 / PI);
}

/* Klasa obiektów przechowujących współrzędne punktu */

class Point {
	public:
		static const double INVALID = 400;

		double x;
		double y;

		Point ();
		Point (double xp, double yp);
		bool operator< (const Point& p) const ;
		bool operator== (const Point& p);
		void operator=  (const Point& p);
		bool lessT (const Point& p, double threshold = 0) const;
		bool valid ();
		double sqrDistance (const Point& p) const;
		double metDistance (const Point& p) const;
		double angle (const Point& p) const;
		void shift(double distance, double direction);
//		void test ();
};

std::ostream& operator<< (std::ostream& os, const Point& p);
std::istream& operator>> (std::istream& is, Point& p);

class Points : public std::list<Point> {
	public:
		void addFromString (const std::string& coords);		
};

typedef Points::iterator PointsIter;

class MaskingPoint : public Point {
	public:
		bool used;
		MaskingPoint () : Point(), used (false)  {};
		MaskingPoint (double xp, double yp) : Point(xp, yp),used (false){};
};

#endif
