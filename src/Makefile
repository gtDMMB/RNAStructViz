CXX=g++
LD=g++
OBJEXT=o

BINARY_OUTPUT=RNAStructViz

CXXFLAGS = -g -Wall -O2
CXXFLAGS_PKGCONFIG=`../scripts/pkg-config-flags.sh --cflags`
CXXFLAGS_FLTK=`fltk-config --use-gl --use-images --use-glut --use-forms --use-cairo --cxxflags`
CXXFLAGS_EXTRA=$(CXXFLAGS_FLTK) $(CXXFLAGS_PKGCONFIG) -std=c++11 -DFLTK_HAVE_CAIRO -DFLTKSTATIC -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_THREAD_SAFE -D_REENTRANT 
CXXFLAGS_FULL=$(CXXFLAGS) $(CXXFLAGS_EXTRA)

LDFLAGS=-g -Wall
LDFLAGS_PKGCONFIG=`../scripts/pkg-config-flags.sh --libs`
LDFLAGS_FLTK=`fltk-config --use-gl --use-images --use-glut --use-forms --use-cairo --ldstaticflags`
LDFLAGS_EXTRA=$(LDFLAGS_FLTK) $(LDFLAGS_PKGCONFIG) 
LDFLAGS_FULL=$(LDFLAGS) $(LDFLAGS_EXTRA)

RNASTRUCTVIZ_OBJECTS = BranchTypeIdentification.$(OBJEXT) ConfigParser.$(OBJEXT)\
	DiagramWindow.$(OBJEXT) DisplayConfigWindow.$(OBJEXT)\
	StatsWindow.$(OBJEXT) \
	GLWindow.$(OBJEXT) InputWindow.$(OBJEXT) \
	FolderWindow.$(OBJEXT) Main.$(OBJEXT) MainWindow.$(OBJEXT) \
	RNAStructure.$(OBJEXT) RNAStructViz.$(OBJEXT) \
	StructureManager.$(OBJEXT) Fl_Rotated_Text.$(OBJEXT)\
	rocbox.$(OBJEXT) 

default: $(RNASTRUCTVIZ_OBJECTS)
	$(LD) -o $(BINARY_OUTPUT) $(RNASTRUCTVIZ_OBJECTS) $(LDFLAGS_FULL)

clean:
	rm -f *.o *.code $(BINARY_OUTPUT) $(RNASTRUCTVIZ_OBJECTS)

install: default
	cp -ap $(BINARY_OUTPUT) /usr/local/bin/
	mkdir -p /usr/local/share/RNAStructViz
	mkdir -p /usr/local/share/RNAStructViz/sample_structures
	cp -ap sample_structures/* /usr/local/share/RNAStructViz/sample_structures/

BranchTypeIdentification.$(OBJEXT): RNAStructure.h BranchTypeIdentification.h\
	BranchTypeIdentification.cpp
	$(CXX) $(CXXFLAGS_FULL) -c BranchTypeIdentification.cpp

ConfigParser.$(OBJEXT): ConfigOptions.h ConfigParser.h ConfigParser.cpp
	$(CXX) $(CXXFLAGS_FULL) -c ConfigParser.cpp

DiagramWindow.$(OBJEXT): DiagramWindow.h RNAStructViz.h \
	BranchTypeIdentification.h RNAStructure.h DiagramWindow.cpp
	$(CXX) $(CXXFLAGS_FULL) -c DiagramWindow.cpp

DisplayConfigWindow.$(OBJEXT): DisplayConfigWindow.h ConfigOptions.h \
	DisplayConfigWindow.cpp
	$(CXX) $(CXXFLAGS_FULL) -c DisplayConfigWindow.cpp

StatsWindow.$(OBJEXT): StatsWindow.h StructureManager.h RNAStructViz.h\
	RNAStructure.h InputWindow.h StatsWindow.cpp
	$(CXX) $(CXXFLAGS_FULL) -c StatsWindow.cpp

GLWindow.$(OBJEXT): GLWindow.h
	$(CXX) $(CXXFLAGS_FULL) -c GLWindow.cpp

InputWindow.$(OBJEXT): InputWindow.h InputWindow.cpp
	$(CXX) $(CXXFLAGS_FULL) -c InputWindow.cpp

FolderWindow.$(OBJEXT): FolderWindow.h StructureManager.h RNAStructViz.h\
	ConfigOptions.h MainWindow.h FolderWindow.cpp
	$(CXX) $(CXXFLAGS_FULL) -c FolderWindow.cpp

Main.$(OBJEXT): MainWindow.h RNAStructViz.h Main.cpp
	$(CXX) $(CXXFLAGS_FULL) -c Main.cpp

MainWindow.$(OBJEXT): MainWindow.h RNAStructViz.h MainWindow.cpp
	$(CXX) $(CXXFLAGS_FULL) -funsigned-char -c MainWindow.cpp

RNAStructure.$(OBJEXT): RNAStructure.h BranchTypeIdentification.h\
	RNAStructure.cpp
	$(CXX) $(CXXFLAGS_FULL) -c RNAStructure.cpp

RNAStructViz.$(OBJEXT): StructureManager.h RNAStructViz.h\
	MainWindow.h FolderStructure.h DiagramWindow.h\
	StatsWindow.h RNAStructViz.cpp
	$(CXX) $(CXXFLAGS_FULL) -c RNAStructViz.cpp

StructureManager.$(OBJEXT): StructureManager.h FolderStructure.h\
	FolderWindow.h MainWindow.h RNAStructViz.h InputWindow.h\
	RNAStructure.h StructureManager.cpp
	$(CXX) $(CXXFLAGS_FULL) -c StructureManager.cpp

Fl_Rotated_Text.$(OBJEXT): Fl_Rotated_Text.H Fl_Rotated_Text.cpp
	$(CXX) $(CXXFLAGS_FULL) -c Fl_Rotated_Text.cpp

rocbox.$(OBJEXT): rocbox.h rocbox.cpp
	$(CXX) $(CXXFLAGS_FULL) -c rocbox.cpp

