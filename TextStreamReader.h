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

#ifndef TextStreamReaderH
#define TexttreamReaderH

#include <iostream>
#include <string>

class TextStreamReader {
	private:
		static const char COMMENT_DELIMITER = ';';
		std::istream& is;
		int lineCounter;
	protected:
                std::string linebuf;//linia przed podzialem na tok,val
		virtual void comment (const std::string& line)=0;
		virtual void line    (const std::string& line)=0;
//		void displayError ();
	public:
		TextStreamReader (std::istream& istr);
		virtual void process ();
		inline int lineNo(void){return lineCounter;}
};

#endif
