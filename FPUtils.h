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

#ifndef FPUtilsH
#define FPUtilsH

// wartosc przyjmowana jako zakres rownosci liczb double
// conajmniej e-10 (e-7 za malo dla przeciec) nie wiecej niz dokladnosc double (e-17?)
#define DOUBLE_EQUAL_EPS 0.00000000001

// porownanie wartosci double. zwraca 0 dla rownych, -1 dla a<b i 1 dla a>b
inline int cmpDoubles(double a, double b){
    a-=b;
    if (a<-DOUBLE_EQUAL_EPS)
        return -1;
    if (a>DOUBLE_EQUAL_EPS)
        return 1;
    return 0;
}

// sprawdza czy n mieści się między n1 i n2 uwzgledniajac niedokladnosc double
inline bool between ( const double &n, const double &n1, const double &n2){
    if (n1 < n2)
        return (n+DOUBLE_EQUAL_EPS>n1 && n-DOUBLE_EQUAL_EPS<n2);
    return (n+DOUBLE_EQUAL_EPS>n2 && n-DOUBLE_EQUAL_EPS<n1);
}

#endif
