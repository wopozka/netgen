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

#ifndef IntersectionH
#define IntersectionH

#include "RoadIDGenerator.h"
#include "Point.h"
#include "datatypes.h"

#include <list>
#include <set>
#include <vector>

// for int64_t
#include <stdint.h>

#define FORALL(zbior,it) for(typeof(zbior.begin()) it=zbior.begin(); it!=zbior.end(); it++)




// Segment, Opakowuje Line dodajac p,k, tak ze p->x <= k->x
struct LineSegment{
    std::list<Line>::iterator edge;
    Point *p;
    Point *k;
    int id; //etykieta krawedzi
    LineSegment(std::list<Line>::iterator e);
    LineSegment(){}
};
typedef std::vector<LineSegment>::iterator LineSegIter;

// porzadkowanie odcinków w zbiorze sweepLine po wysokosciach w punkcie sweep.
// zakladam, ze brak poziomych (oddzielna obsluga wyznaczania ich przeciec z reszta)
class LSIterComparator{
  public:
    static double sweep;
    static double heigh(const LineSegIter & pe1){
        return (pe1->edge->a * sweep + pe1->edge->b);
    }
    inline bool operator() (const LineSegIter & pe1, const LineSegIter & pe2) const;
};

typedef std::set<LineSegIter, LSIterComparator> SweepLineSet;



// Kolejnosc sortowania zdarzen wg. typu:
// - najpierw vertical(by sprawdzic po drodze stare konce),
// - pozniej konce (by usunac przed zmiana warunku sortowania)
// - potem intersecty, by wiedziec kogo swapowac
// - potem poczatki, by dodane w nowym porzadku
#define NOT_EVENT -1
#define VERTICAL_EVENT 3
#define RIGHT_EVENT 2
#define INTERSECTION_EVENT 1
#define LEFT_EVENT 0
// warto by to chyba przepisac na klase, funkcje w strukturze to slaby pomysl
struct EventPoint{
    Point p;
    int type;
    LineSegIter segIdx;
    LineSegIter intSegIdx; // intersecting segment
    EventPoint(LineSegment &s, int t, LineSegIter idx);
    EventPoint(const Point& cp, LineSegIter idx, LineSegIter idx2);
    EventPoint():type(NOT_EVENT){}
    inline bool operator< (const EventPoint& ep) const;
};



class Intersection {
    public:

        std::list<Point> intersections;
        Point find (Line& l1, Line& l2);
        void findAll (std::list<Line>& linelist);
        void findAll_old (std::list<Line>& linelist);
        void removeNodes (Pointmap& nodes);
        void removePoints (MapData&);
        void alignNodes (std::list<Line>& linelist,
                         Pointmap& nodes,
                         Nodemap& new_nodes);
    private:
        std::vector<LineSegment> segmenty; // wszystkie segmenty
        SweepLineSet sweepLine;       // zbior SW (posortowane po wysokosciach w sweep)
        std::set<EventPoint> eventsQueue;  // kolejka zdarzen
        std::set<int64_t> existingInt;   // zbior wykrywania powtorzen przeciec
        std::list<LineSegIter> wertykalne; // zbior poziomych dla aktualnej pozycji
        std::set<LineSegIter> swapowane;   // zbior odcinkow zaburzajacych sortowanie w sweepLine
        int intersectionCounter;

        // znajdowanie punktu przeciecia dwoch segmentow, wynik zapisany do p.
        // wartosc zwracana != 0 jesli punkt przeciecia zostal znaleziony
        static int findImproved (const LineSegment& l1, const LineSegment& l2, Point *p);
        // zmiana pozycji linii wymiatajacej polaczona z obsluga przeciec
        void moveSweepLine(double newSL);
        // weryfikator poprawnosci porzadkowania segmentow w sweepLine (debug)
        bool checkSW(bool print=false);
};

#endif
