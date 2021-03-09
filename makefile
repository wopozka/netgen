VERSION = 1.24zakazy
CXX = g++ 
CXXWIN = i586-mingw32msvc-g++
# EXEEXT=.exe
CXXLINK = $(CXX)
# CXXFLAGS = -g -O2 -Wall --pedantic
CXXFLAGS = -g -O2 -Wall -std=gnu++03
# CXXFLAGS = -ggdb -Wall --pedantic
netgen_LDFLAGS = -static
netgen_EXTRA_SRC =makefile netgen.po
netgen_EXTRA_DIST = Changes Authors Copying netgen.cfg README

netgen_SOURCES  = TextStreamReader.cpp \
		  PFMStreamReader.cpp \
		  NetAnalyzer.cpp \
		  NodeWriter.cpp \
		  Point.cpp \
		  Line.cpp \
		  RoadIDGenerator.cpp \
		  ConfigReader.cpp \
		  Intersection.cpp \
		  StreetNum.cpp \
		  OziOutput.cpp \
		  RouteParameters.cpp \
		  Polyline.cpp \
		  netgen.cpp \
		  
netgen_HEADERS  = TextStreamReader.h \
		  PFMStreamReader.h \
		  NetAnalyzer.h \
		  NodeWriter.h \
		  FPUtils.h \
		  Point.h \
		  Line.h \
		  RoadIDGenerator.h \
		  ConfigReader.h \
		  Intersection.h \
		  StreetNum.h \
		  OziOutput.h \
		  RouteParameters.h \
		  Polyline.h \
		  netgen.h \
		  MapData.h \
		  datatypes.h
		  

netgen_OBJECTS = PFMStreamReader.o \
		 TextStreamReader.o \
		 NetAnalyzer.o \
		 NodeWriter.o \
		 Point.o \
		 Line.o \
		 RoadIDGenerator.o \
		 ConfigReader.o \
		 Intersection.o \
		 StreetNum.o \
		 OziOutput.o \
		 RouteParameters.o \
		 Polyline.o \
		 netgen.o
	
areafilter_OBJECTS = PFMStreamReader.o \
		 TextStreamReader.o \
		 Point.o \
		 ConfigReader.o \
		 AreaFilter.o \
		 areaflt.o
		 
netgen: $(netgen_OBJECTS)
	@rm -f netgen$
	$(CXXLINK) $(netgen_LDFLAGS) $(netgen_OBJECTS) $(netgen_LDADD) $(LIBS) -o netgen

netgen.exe: $(netgen_SOURCES)
	@rm -f netgen.exe
	$(CXXWIN) -DVERSION='"$(VERSION)"' $(netgen_LDFLAGS) \
	$(netgen_SOURCES) $(netgen_LDADD) $(LIBS) -o netgen.exe

netgen.mo: netgen.po
	msgfmt -o netgen.mo netgen.po

areafilter$(EXEEXT): $(areafilter_OBJECTS)
	@rm -f areafilter$(EXEEXT)
	$(CXXLINK) $(netgen_LDFLAGS) $(areafilter_OBJECTS) $(netgen_LDADD) $(LIBS) -o areafilter
	
.SUFFIXES: .cpp .o 

.cpp.o: 
	$(CXX) $(CXXFLAGS) -DVERSION='"$(VERSION)"' -c -o $@ $<

.PHONY: clean

clean:
	rm $(netgen_OBJECTS) netgen netgen.exe

arch:
	tar -cvjf netgen-$(VERSION).tar.bz2 $(netgen_SOURCES) $(netgen_HEADERS) $(netgen_EXTRA_DIST) $(netgen_EXTRA_SRC)

arch-bin:
	strip netgen
	tar -cvjf netgen-linux-static-bin-$(VERSION).tar.bz2  netgen netgen.mo $(netgen_EXTRA_DIST)

arch-win:
	i586-mingw32msvc-strip netgen.exe
	zip netgen-win-$(VERSION).zip  netgen.exe netgen.mo $(netgen_EXTRA_DIST)

dist: netgen netgen.exe arch arch-win arch-bin 

