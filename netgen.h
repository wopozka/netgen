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


// pomijanie powielonych punktów polylinii
#define REMOVE_DUPLICATED

#undef NUMBERING_REPORT

// dzielenie pętli na nie zapętlone fragmenty
#define REMOVE_LOOP

#define CREATE_LIST_OF_LINES

#define FLAG_OZI_ONLY     1
#define FLAG_CHECK_ONLY   2
// #define FLAG_OUTPUT_MOVED 4
//#define FLAG_DO_NOT_REMOVE_TEMP 8
#define FLAG_NODE_ON_ROAD_END 16
#define FLAG_NODE_ON_RESTRICTION 32
#define FLAG_FIND_CROSSINGS      64
#define FLAG_SKIP_REGULAR	128
#define FLAG_ADJUST_CONNECTOR_CLASS       256
#define FLAG_ADJUST_CLASSES_IN_NODE       512

#define SCALE_FACTOR 1e7

#define FGETS_BUFFER 5000000

#define MAX_ROAD_ID 16000000

#ifndef __WIN32__

#define GETTEXT
#include <libintl.h>
#define _(String) gettext (String)
#define gettext_noop(String) String
#define N_(String) gettext_noop (String)

#else 
	
#define _(String) (String)
#define N_(String) String
#define textdomain(Domain)
#define bindtextdomain(Package, Directory)

#endif


