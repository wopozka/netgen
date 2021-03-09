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

#include "Line.h"
#include "FPUtils.h"
#include <iostream>

using namespace std;

// int Line::numCalls = 0;

void Line::computeParameters (){
    if (cmpDoubles(p.x, k.x)==0)
        vertical = true;
    else {
        a = (p.y - k.y)/(p.x - k.x);
        b = p.y  - p.x * a;
        vertical = false;
    }
}

Line::Line (Point& p1, Point& p2):
        p(p1), k(p2)
{
    computeParameters ();
}

// sprawdza czy współrzędne punktu q zawieraja się między
// współrzędnymi końców odcinka

bool Line::containsPoint (const Point& q){
    if (!between (q.x, p.x, k.x))
        return false;
    return between (q.y, p.y, k.y);
}

// returns squared distance between line and point

double Line::distanceFromPoint (const Point& q){

//   numCalls ++;

   // Compute coordinates of rectangular projection of given point

   double x,y;

   if (vertical){
      x = p.x;
      y = q.y;
   } else if (cmpDoubles (a,0) == 0){ 
      // horizontal line, so orthogonal is vertical
      x = q.x;
      y = p.y;
   } else {      
      // Line is neither vertical nor horizontal
      x = (q.y + (q.x / a) - b) / (a + 1/a);
      y = q.y + (q.x - x ) / a;
   }

   if (containsPoint(Point(x,y))){
      return (x - q.x) * (x - q.x) + (y - q.y) * (y - q.y);
   }

   // Compute distances from end of segment points, 
   // then return lower of them

   double pd = (p.x - q.x) * (p.x - q.x) + (p.y - q.y) * (p.y - q.y);
   double kd = (k.x - q.x) * (k.x - q.x) + (k.y - q.y) * (k.y - q.y);

   if (pd < kd){
      return pd;
   }
   return kd;
}

