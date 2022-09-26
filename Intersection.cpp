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


#include "Intersection.h"
#include "FPUtils.h"
#include "netgen.h"

#include <cmath>
#include <vector>
#include <algorithm>
#include <iomanip>

using namespace std;


// Znajdując przecięcia odcinków w których nie ma węzłów mozna wspomóc
// wyszukiwanie błędnych skrzyżowań.


LineSegment::LineSegment(list<Line>::iterator e):edge(e){
    if (e->p.x < e->k.x){
        p = &(e->p);
        k = &(e->k);
    } else {
        p = &(e->k);
        k = &(e->p);
    }
}


inline bool LSIterComparator::operator() (const LineSegIter & pe1, const LineSegIter & pe2) const {
   double diff = (pe1->edge->a * sweep + pe1->edge->b) - (pe2->edge->a * sweep + pe2->edge->b);
   if (diff<-DOUBLE_EQUAL_EPS || diff>DOUBLE_EQUAL_EPS)
     return diff<0;
   if (pe1->edge->a != pe2->edge->a)
     return pe1->edge->a < pe2->edge->a;
   return pe1->id < pe2->id;//rownolegle??, ale i tak lepiej aby set ich nie usunal
}

Point Intersection::find (Line& l1, Line& l2){
   // nie uzywane
    double x, y;
//   l1.computeParameters ();
//   l2.computeParameters ();
    if ((l1.p == l2.p) || (l1.p == l2.k))
        return Point (Point::INVALID, Point::INVALID);
    if ((l1.k == l2.p) || (l1.k == l2.k))
        return Point (Point::INVALID, Point::INVALID);
    if (l1.vertical && l2.vertical){
        return Point (Point::INVALID, Point::INVALID);
    }
    if (!l1.vertical && !l2.vertical){
        if (l1.a == l2.a){
	 // równloegłe
            return Point (Point::INVALID, Point::INVALID);
        }
        else {
            x = (l2.b - l1.b)/(l1.a - l2.a);
            y = l1.a * x + l1.b;
        }
    }
    if (l1.vertical && !l2.vertical){
        x = l1.p.x;
        y = l2.a * x + l2.b;
    }
    if (!l1.vertical && l2.vertical){
        x = l2.p.x;
        y = l1.a * x + l1.b;
    }
    Point p (x,y);
    if (l1.containsPoint (p) && l2.containsPoint(p)){
        return p;
    }
    else
        return Point (Point::INVALID, Point::INVALID);
}


int findImprovedCalls = 0;

//znajduje punkt przeciecia pary segmentow. Wynik w p, jesli wynik != 0
int Intersection::findImproved (const LineSegment& l1, const LineSegment& l2, Point *p){
    double x, y;
    findImprovedCalls++;

    if ( cmpDoubles(l1.p->x,l2.p->x)==0 && cmpDoubles(l1.p->y,l2.p->y)==0 )
        return 0;
    if ( cmpDoubles(l1.p->x,l2.k->x)==0 && cmpDoubles(l1.p->y,l2.k->y)==0 )
        return 0;
    if ( cmpDoubles(l1.k->x,l2.p->x)==0 && cmpDoubles(l1.k->y,l2.p->y)==0 )
        return 0;
    if ( cmpDoubles(l1.k->x,l2.k->x)==0 && cmpDoubles(l1.k->y,l2.k->y)==0 )
        return 0;
    if (l1.edge->vertical && l2.edge->vertical)
        return 0;

    if (!l1.edge->vertical && !l2.edge->vertical){
        double dif = l1.edge->a - l2.edge->a;
        // nie powinno sie uzywac == dla wartosci double
        if (dif < DOUBLE_EQUAL_EPS && dif > -DOUBLE_EQUAL_EPS)
        {
            // równloegłe
            return 0;
        } else {
            x = (l2.edge->b - l1.edge->b)/dif;
            y = l1.edge->a * x + l1.edge->b;
        }
    }
    if (l1.edge->vertical && !l2.edge->vertical){
        x = l1.edge->p.x;
        y = l2.edge->a * x + l2.edge->b;
    }
    if (!l1.edge->vertical && l2.edge->vertical){
        x = l2.edge->p.x;
        y = l1.edge->a * x + l1.edge->b;
    }
    p->x=x;
    p->y=y;
    if (l1.edge->containsPoint( *p) && l2.edge->containsPoint(*p)){
        return 1;
    } else
        return 0;
}

double LSIterComparator::sweep = -200;

EventPoint::EventPoint(LineSegment &s, int t, LineSegIter idx)
                                               :type(t),segIdx(idx){
    if (type==LEFT_EVENT)
        p = *(s.p);
    else
        p = *(s.k);
}
EventPoint::EventPoint(const Point& cp, LineSegIter idx, LineSegIter idx2)
                :p(cp),type(INTERSECTION_EVENT),segIdx(idx),intSegIdx(idx2){
}


inline bool EventPoint::operator< (const EventPoint& ep) const {

   double diff = p.x - ep.p.x;
   if (diff<-DOUBLE_EQUAL_EPS||diff>DOUBLE_EQUAL_EPS)
     return (p.x < ep.p.x);
   // porzadkowanie wg. typu
   if (type != ep.type)
     return type > ep.type;
   // pozostale kryteria tylko by rozrozniac
   if (p.y != ep.p.y)
     return (p.y < ep.p.y);
   if (segIdx != ep.segIdx || type != -1)
     return segIdx < ep.segIdx;
   return intSegIdx->id < ep.intSegIdx->id;
}

const char *event2Char[] = {"LEFT_EVENT","INTERSECTION_EVENT","RIGHT_EVENT","VERTICAL_EVENT"};

std::ostream& operator<< (std::ostream& os, const EventPoint& e){
    os << e.p << " " << e.segIdx->id;
    if (e.type==INTERSECTION_EVENT)
        os << " x " << e.intSegIdx->id;
    os << " " << event2Char[e.type];
    return os;
}

// weryfikator poprawnosci porzadkowania segmentow w sweepLine
bool Intersection::checkSW(bool print){
    Point p;
    bool failed = false;
    if (sweepLine.empty())
        return false;
    SweepLineSet::iterator it,pre;
    LSIterComparator cmpd;
    it=pre=sweepLine.begin();
    if (print)
        cerr << *((**it).p) << "," << *((**it).k) << "," << ((**it).id)<< "," << ((**it).edge->a)<< "," << cmpd.heigh(*it);
    for(it++; it!=sweepLine.end(); it++, pre++){
        if (!cmpd(*pre,*it)){
            failed = true;
            if (print)
                cerr << " ? ";
        } else {
            if (print)
                cerr << " < ";
        }
        if (print)
            cerr << *((**it).p) << "," << *((**it).k) << "," << ((**it).id)<< "," << ((**it).edge->a)<< "," << cmpd.heigh(*it);
    }
    if (print)
        cerr << endl;
    return failed;
}

// generator etykiet unikalnych dla danej pary segmentow
// ileMaks - ilosc wszystkich segmentow (maxymalny id + 1)
int64_t iteratorHasher(const LineSegIter &itA, const LineSegIter &itB, int ileMaks){
    int64_t a = itA->id,b = itB->id,c;
    if (a>b) {//swap(a,b)
        c=a;
        a=b;
        b=c;
    }
    return a*ileMaks+b;
}



// implementacja wyszukiwania przeciec na bazie algorytmu Bentley-Ottmanna:
// http://www.geometryalgorithms.com/Archive/algorithm_0108/algorithm_0108.htm#Bentley-Ottmann%20Algorithm
// sweepLine zaimplementowana na set, ze zmiennym warunkiem sortowania, zatem uzylem
// pewnej sztuczki: usunac problematyczne, zmienic warunek, dodac problematyczne
// Inaczej trzeba implementowac drzewa R&B samodzielnie.
void Intersection::findAll (list<Line>& lineList){
    int noOfSegments = lineList.size();
    cerr << _("Number of segments: ") << noOfSegments << endl;
    int iterations = 0;
    double lastepx=-220;          // do wykrywania zmiany polozenia linii wymiatajacej

    findImprovedCalls = 0;
    intersectionCounter = 0;
    segmenty.clear();
    sweepLine.clear();
    eventsQueue.clear();
    existingInt.clear();
    wertykalne.clear();
    swapowane.clear();

    // wypelnij kolejke zdarzen wszystkimi koncami
    segmenty.resize(noOfSegments);
    int i=0;
    FORALL(lineList, l1){
        l1->computeParameters();
        segmenty[i] = LineSegment(l1);
        segmenty[i].id = i;
        if (l1->vertical) {
            eventsQueue.insert(EventPoint(segmenty[i],VERTICAL_EVENT,segmenty.begin()+i));
        } else {
            eventsQueue.insert(EventPoint(segmenty[i],LEFT_EVENT,segmenty.begin()+i));
            eventsQueue.insert(EventPoint(segmenty[i],RIGHT_EVENT,segmenty.begin()+i));
        }
        i++;
    }

    while(!eventsQueue.empty()){
        iterations++;
        Point interPoint;

        // pobierz nastepny punkt z kolejki
        EventPoint e = *(eventsQueue.begin());
        eventsQueue.erase(eventsQueue.begin());

        // zdarzenia przychodza w kolejnosci: (vri@l),(vri@l),(vri@l), gdzie
        // v - wertykalne : obsluga przy left/right oraz po zmianie sweepa (@)
        // r - konce (RIGHT_EVENT) : usuwanie segmentu ze sweepLine
        // @ : wlasciwe przejscie sweepa (punktu porownywania
        //     wysokosci segmentow) na kolejny poziom:
        //     - usun swapowane
        //     - przesun sweep'a
        //     - dodaj wszystkie pomyslnie usuniete i potem sprawdz sasiadow (generuj intersection eventy)
        //     - weryfikuj wertykalne
        // l - poczatki (LEFT_EVENT) : dodaj nowy segment do sweepLine
        // , - nowa wartosc e.p.x
        // przykladowe sekwencje zdarzen:
        // (V,R,R,I,@,L,L),(V,I,@),(V,R,R,@),(V,R,R,@),(V,@,L,L),(V,I,@,L),...
        // (vri@l),(vri@),(vr@),(@l)

        if (e.type==LEFT_EVENT) { // start point
            //sprawdz przeciecie/stycznosc z wertykalnymi
            FORALL(wertykalne,wit){
                if (findImproved( **wit, *e.segIdx, &interPoint)){
                    int64_t index = iteratorHasher(*wit, e.segIdx, segmenty.size());
                    existingInt.insert(index); // aby wykrywac powtorki z wertykalnymi
                    intersectionCounter++;
                    intersections.push_back(interPoint);
                }
            }
            //Let segE = E's segment;
            //Add segE to SL;
            //Let segA = the segment above segE in SL;
            //Let segB = the segment below segE in SL;
            SweepLineSet::iterator segE, segA, segB;
            segA = segB = segE = sweepLine.insert(e.segIdx).first;
            segA++;
            segB--;
            //If (I = Intersect( segE with segA) exists)
            //    Insert I into x;
            if (segA!=sweepLine.end() && findImproved( **segA, **segE, &interPoint) ) {
                int64_t index = iteratorHasher(*segE, *segA, segmenty.size());
                existingInt.insert(index);
                intersectionCounter++;
                if ( cmpDoubles(interPoint.x,e.p.x) <= 0 ) {
                    // ip.x<=e.p.x - punkt stycznosci swap juz uwzglednione przy dodawaniu
                    intersections.push_back(interPoint);
                } else {
                    eventsQueue.insert(EventPoint(interPoint, *segA, *segE ));
                }
            }
            //If (I = Intersect( segE with segB) exists)
                //    Insert I into x;
            if (segE!=sweepLine.begin() && findImproved( **segB, **segE, &interPoint) ) {
                int64_t index = iteratorHasher(*segE, *segB, segmenty.size());
                existingInt.insert(index);
                intersectionCounter++;
                if ( cmpDoubles(interPoint.x,e.p.x) <= 0 ) {
                    // ip.x<=e.p.x - punkt stycznosci juz uwzgledniony przy dodawaniu
                    intersections.push_back(interPoint);
                } else {
                    eventsQueue.insert(EventPoint(interPoint, *segB, *segE ));
                }
            }
        } else if (e.type==RIGHT_EVENT) { // end point
            //sprawdz przeciecie/stycznosc z wertykalnymi
            FORALL(wertykalne,wit){
                if (findImproved( **wit, *e.segIdx, &interPoint)){
                    intersectionCounter++;
                    intersections.push_back(interPoint);
                }
            }
            //Let segE = E's segment;
            //Let segA = the segment above segE in SL;
            //Let segB = the segment below segE in SL;
            SweepLineSet::iterator segE, segA, segB;
            segA = segB = segE = sweepLine.find(e.segIdx);
            if (segE == sweepLine.end()){
                // Przyczyny::
                // - blad w programie: np. zle liczone wysokosci -> koniec przed poczatkiem
                // - bledy numeryczne: np. blad wyznaczania wysokosci wiekszy niz przyjete DOUBLE_EQUAL_EPS
                cerr << "WARNING: Logic assumption Failed (segment to removal not found): " << e << endl;
                break;
            }
            segA++;
            segB--;
            //Remove segE from SL;
            //If (I = Intersect( segA with segB) exists)
            //    If (I is not in x already) Insert I into x;
            if (segA!=sweepLine.end() && segE!=sweepLine.begin() && findImproved( **segA, **segB, &interPoint) ) {
                int64_t index = iteratorHasher(*segA, *segB, segmenty.size());
                if (existingInt.insert(index).second) {
                    intersectionCounter++;
                    eventsQueue.insert(EventPoint(interPoint, *segA, *segB ));
                }
            }
            sweepLine.erase(segE);;
        } else if (e.type==VERTICAL_EVENT) { // vertical line
            // dodaj do 'wertykalne' a wlasciwa obsluga wertykalnych:
            // - LEFT_EVENT/RIGHT_EVENT : sprawdz czy punkt przecina wertykalne przy wstawianiu/usuwaniu
            // - @ : przy zmianie sweep'a sprawdz przeciecia z segmentami w sweepLine
            wertykalne.push_back(e.segIdx);
        } else { // intersection
            //Add E to the output list L;
            intersections.push_back(e.p);
            //reszta odroczona do momentu przejscia do kolejnego sweep'a
            swapowane.insert(e.segIdx);
            swapowane.insert(e.intSegIdx);
            //Let segE1 above segE2 be E's intersecting segments in SL;
            //Swap their positions so that segE2 is now above segE1;
            //Let segA = the segment above segE2 in SL;
            //Let segB = the segment below segE1 in SL;
            //If (I = Intersect(segE2 with segA) exists)
            //    If (I is not in x already) Insert I into x;
            //If (I = Intersect(segE1 with segB) exists)
            //    If (I is not in x already) Insert I into x;
        }


        if (eventsQueue.empty())
            break;

        if (lastepx<-200)// inicjalizacja polozenia linii wymiatajacej
            lastepx = LSIterComparator::sweep = e.p.x;
        lastepx = e.p.x;
        e = *(eventsQueue.begin());
        // przejscie przez przypadek bez LEFT_EVENT:  '@),('
        if ( cmpDoubles(lastepx,e.p.x)!=0
                        && cmpDoubles(LSIterComparator::sweep,lastepx)!=0 ){
            moveSweepLine(lastepx);
        }
	e = *(eventsQueue.begin());
        // Przejscie przez przypadek ...@L... , poza powyzszym, bo np.: ...@),(@L... )
        if ( e.type==LEFT_EVENT && cmpDoubles(LSIterComparator::sweep, e.p.x)!=0 ){
            moveSweepLine(e.p.x);
        }
    }

    //cerr << _("Number of sweepLine events: ") << iterations << endl;
    cerr << _("Number of intersections without node: ") << intersectionCounter << endl;
    //cerr << _("Number of pair intersection comparisions: ") << findImprovedCalls << endl;
}

// odroczona obsluga wszystkich intersection z danego sweepa
// Generowane zdarzenia moga byc wczesniejsze niz kolejny e, dlatego
// tutaj dodatkowo podgladanie jakie jest nastepne zdarzenie, do
// stwierdzenia czy przejscie przez punkt @ w sekwencji (vri@l),(vri@l)
void Intersection::moveSweepLine(double newSL){
    list<LineSegIter> pomyslne;
    list<SweepLineSet::iterator> wstawione;

    // usun swapowane (te ktore zaburzaja kolejnosc po sweepie)
    FORALL(swapowane,swIt){
        if ( sweepLine.erase(*swIt) )
            pomyslne.push_back(*swIt);
    }

    // zmien sweep na nowy
    // zbedna zmienna?
    // double preSweep = LSIterComparator::sweep;
    LSIterComparator::sweep = newSL;
    // dodaj wszystkie usuniete
    FORALL(pomyslne,swIt){
        wstawione.push_back(sweepLine.insert(*swIt).first);
    }
    // sprawdz sasiadow (odroczona obsluga INTERSECTION_EVENT)
    FORALL(wstawione,wsIt){
        Point interPoint;
        SweepLineSet::iterator segA, segB, segE;
        segA = segB = segE = *wsIt;
        segA++;
        segB--;
        //If (I = Intersect(segE with segA) exists)
        //    If (I is not in x already) Insert I into x;
        if (segA!=sweepLine.end() && findImproved( **segA, **segE, &interPoint) ) {
            int64_t index = iteratorHasher(*segA, *segE, segmenty.size());
            if (existingInt.insert(index).second) {
                intersectionCounter++;
                eventsQueue.insert(EventPoint(interPoint, *segA, *segE ));
            }
        }
        //If (I = Intersect(segE with segB) exists)
        //    If (I is not in x already) Insert I into x;
        if (segE!=sweepLine.begin() && findImproved( **segB, **segE, &interPoint) ) {
            int64_t index = iteratorHasher(*segB, *segE, segmenty.size());
            if (existingInt.insert(index).second) {
                intersectionCounter++;
                eventsQueue.insert(EventPoint(interPoint, *segB, *segE ));
            }
        }
    }
    // sprawdz wertykalne
    if (!wertykalne.empty() && !sweepLine.empty()){
        Point p(0,0),k(1,1);
        list<Line> dumbList;
        dumbList.push_back(Line(p,k));
        vector<LineSegment> dumbVector;
        dumbVector.push_back(LineSegment(dumbList.begin()));
        dumbVector[0].edge->a=0;
        dumbVector[0].id=-1;
        FORALL(wertykalne,wrIt){
            dumbVector[0].edge->b = (*wrIt)->p->y;
            SweepLineSet::iterator bsit, sit;
            bsit = sit = sweepLine.lower_bound(dumbVector.begin());
            while(bsit!=sweepLine.begin()){
                bsit--;
                // zbedna zmienna?
                // double hgh = LSIterComparator::heigh(*bsit);
                if (findImproved(**bsit, **wrIt,&p)){
                    int64_t index = iteratorHasher(*bsit, *wrIt, segmenty.size());
                    if (existingInt.insert(index).second){
                        intersectionCounter++;
                        intersections.push_back(p);
                    }
                } else
                    break;
            }
            for(bsit=sit;bsit!=sweepLine.end(); bsit++){
                // zbedna zmienna?
                // double hgh = LSIterComparator::heigh(*bsit);
                if (findImproved(**bsit, **wrIt,&p)){
                    int64_t index = iteratorHasher(*bsit, *wrIt, segmenty.size());
                    if (existingInt.insert(index).second){
                        intersectionCounter++;
                        intersections.push_back(p);
                    }
                } else
                    break;
            }
        }
    }
    // wyczysc zbiory tymczasowe
    swapowane.clear();
    wertykalne.clear();
}

struct LSComparator{
    bool operator() (const LineSegment & pe1, const LineSegment & pe2) const {
        if (pe1.p->x != pe2.p->x)
            return pe1.p->x < pe2.p->x;
        if (pe1.p->y != pe2.p->y)
            return pe1.p->y < pe2.p->y;
        return pe1.k < pe2.k;
    }
};

void Intersection::findAll_old (list<Line>& lineList){
    cerr << _("Number of segments: ") << lineList.size() << endl;
    int intersectionCounter = 0;
    int tc = 0;
    list<Line>::iterator l1;
    Point p;
    vector<LineSegment> poczatki;
    LineSegIter ip1, ip2;
    int i=0;

    //poczatki.resize(lineList.size());
    for (l1 = lineList.begin (); l1 != lineList.end(); l1++, i++){
        poczatki.push_back(LineSegment(l1));
    }
    sort(poczatki.begin(), poczatki.end(), LSComparator());

    for(ip1 = poczatki.begin(); ip1!=poczatki.end(); ip1++){
        ip2=ip1;
        for(ip2++; ip2!=poczatki.end() && ip1->k->x+0.0001 > ip2->p->x; ip2++){
            tc++;
            if (findImproved(*ip1, *ip2, &p)) {
                //p.x = round (p.x * SCALE_FACTOR) / SCALE_FACTOR;
                //p.y = round (p.y * SCALE_FACTOR) / SCALE_FACTOR;
                intersectionCounter++;
                intersections.push_back (p);
            }
        }
    }
    cerr << _("Number of intersections without node: ") << intersectionCounter << endl;
    //cerr << _("Number of pair intersection comparisions: ") << tc << endl;
}

/*
void Intersection::removeNodes (Pointmap& nodes){
    list<Point>::iterator pn = intersections.begin();
    while ( pn != intersections.end()){
        PointmapIter nd = nodes.find (*pn);
        if (nd != nodes.end()){
            pn = intersections.erase (pn);
        }
        else
            pn++;
    }
    cerr << _("Number of intersections without node: ") << intersections.size() << endl;
}
*/

void Intersection::removePoints (MapData& md){
    list<Point>::iterator pn = intersections.begin();
    while ( pn != intersections.end()){
        if (md.isMaskedNoCrossing(*pn)){
            pn = intersections.erase (pn);
        }
        else
	  pn++;
    }
    cerr << _("Number of unmasked intersections without node: ") << intersections.size() << endl;
}

void Intersection::alignNodes (list<Line>& lineList,
                            Pointmap& nodes,
                            Nodemap& new_nodes){
    list<Line>::iterator ln;
    for (ln = lineList.begin (); ln != lineList.end(); ln++){
        PointmapIter pm;
        pm = nodes.find (ln->p);
        if (pm != nodes.end()){
            ln->p = new_nodes[pm->second.id].p;
        }
        pm = nodes.find (ln->k);
        if (pm != nodes.end()){
            ln->k = new_nodes[pm->second.id].p;
        }
    }
}

