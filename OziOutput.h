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

#ifndef OziOutputH
#define OziOutputH

#include "datatypes.h"
#include "MapData.h"
#include <ostream>
#include <list>
#include <set>
#include <string>

class OziOutput {
	private:
		std::ostream& os;
		std::string flagsToString (int flags);
		static const std::string unknownParameters;
	public:
		OziOutput (std::ostream& ostr);
		void Header ();
		void Nodes (Nodemap &nm, int endNodeCounter, 
			    MapData& md, bool skip_regular, 
			    bool skip_alone);
		void Intersections (std::list<Point> &lp, MapData& md);
		void List (std::list<Point> &isi, const std::string& prefix);
		void Unused (MapData& md, bool intersections, bool ends);

};


#endif

