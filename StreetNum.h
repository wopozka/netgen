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

#ifndef streetnumH
#define streetnumH

#include "ConfigReader.h"
#include "MapData.h"
#include "datatypes.h"
#include <istream>

class SideNumbers {
	public:
		char kind;
		int  start;
		int  end;
};

class HouseNumbers {
	public:		
		int startPoint;
		int endPoint;	


		SideNumbers left;
		SideNumbers right;
		void clear ();
};

class StreetNum {
	private:
		MapData& mapData;
		std::list<Points>* datalist; 
		std::map<int,int> nodes;
		std::list<HouseNumbers> numberList;
		bool even (int n);
		bool odd  (int n);
		bool streetNumOverlaps (HouseNumbers& hn);
		char numbersType (int begin, int end);
		int countCommas (std::string& str);
		int extractNumbers (const std::string& buf, int& begin, int& end, char& type);
		int pointInList(Point& p, std::list<Points>* datalist);
		void numberNodes ();
		void parseLabel (std::string label,
				 int& rBegin,int& rEnd,char& rType,
				 int& lBegin,int& lEnd,char& lType);
	public:
		StreetNum (MapData& md);
		static void description (PolylineListIter pl, 
	       				 std::ostream& os);
		void clear ();
		void addNode (int point);
		void generateNumbers (std::ostream& os,
				      std::list<Points>* dl,
				      int& warningCounter);

};

#endif

