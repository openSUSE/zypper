####### Compiler, tools and options

CC            = gcc
CXX           = g++
DEFINES       =  
CFLAGS        = -pipe -O2 -Wall -W -D_REENTRANT  $(DEFINES)
CXXFLAGS      = -pipe -O2 -Wall -W -D_REENTRANT  $(DEFINES)
INCPATH       = -I/usr/include -I. -I. -I.
LINK          = g++
LFLAGS        = -lmysqlclient
LIBS          = $(SUBLIBS)  -L/usr/lib

####### Output directory

OBJECTS_DIR   = ./

####### Files

SOURCES       = db.cc resolvable.h dbmain.cc 
OBJECTS       = db.o dbmain.o resolvable.o
OBJCOMP		  = /usr/lib/libzypp.so 
DESTDIR       = 
TARGET        = dbread

first: all
####### Implicit rules

.SUFFIXES: .o .c .cpp .cc .cxx .C

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.cc.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.cxx.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.C.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.c.o:
	$(CC) -c $(CFLAGS) $(INCPATH) -o "$@" "$<"

####### Build rules

all: Makefile $(TARGET)

$(TARGET):  $(OBJECTS)  
	$(LINK) $(LFLAGS) -o $(TARGET) $(OBJECTS) $(OBJCOMP) $(LIBS)

