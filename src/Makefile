CXX=g++
LD=g++
OBJEXT=o

BINARY_OUTPUT=RNAStructViz

CXXFLAGS = -g -Wall -O2
CXXFLAGS_EXTRA = `fltk-config --cflags` `pkg-config --cflags cairo` -std=c++11 -DFLTK_HAVE_CAIRO -I/usr/local/include -I/usr/include/cairo -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/pixman-1 -I/usr/include/freetype2 -I/usr/include/libpng12 -I/usr/local/include/FL/images -I/usr/include/freetype2 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_THREAD_SAFE -D_REENTRANT -I/usr/include/cairo -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/pixman-1 -I/usr/include/freetype2 -I/usr/include/libpng12
AM_CPPFLAGS = -g -Wall -std=c++11 -DFLTK_HAVE_CAIRO -I/usr/local/include -I/usr/include/cairo -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/pixman-1 -I/usr/include/freetype2 -I/usr/include/libpng12 -I/usr/local/include/FL/images -I/usr/include/freetype2 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_THREAD_SAFE -D_REENTRANT -I/usr/include/cairo -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/pixman-1 -I/usr/include/freetype2 -I/usr/include/libpng12
CXXFLAGS_FULL=$(CXXFLAGS) $(CXXFLAGS_EXTRA) $(AM_CPPFLAGS)

LDFLAGS = 
LDFLAGS_EXTRA = `fltk-config --ldflags` `pkg-config --ldflags cairo` -std=c++11 /usr/local/lib/libfltk.a -lfltk -lfltk_gl -lfltk_cairo -lfltk_images -lstdc++ -lpthread -ldl -lm -ldl -lfontconfig -lfreetype -lcairo -lcairo
AM_LDFLAGS = -g -Wall -std=c++11 /usr/local/lib/libfltk.a -lfltk -lfltk_gl -lfltk_cairo -lfltk_images -lstdc++ -lpthread -ldl -lm -ldl -lfontconfig -lfreetype -lcairo -lcairo
LIBS = -std=c++11 /usr/local/lib/libfltk.a -lfltk -lfltk_gl -lfltk_cairo -lfltk_images -lstdc++ -lpthread -ldl -lm -ldl -lfontconfig -lfreetype -lcairo -lcairo
LDFLAGS_FULL=$(LDFLAGS) $(LDLAGS_EXTRA) $(AM_LDFLAGS) $(LIBS)

RNAStructViz_SOURCES = \
      BranchTypeIdentification.cpp \
      DiagramWindow.cpp \
      StatsWindow.cpp \
      GLWindow.cpp \
      InputWindow.cpp \
      FolderWindow.cpp \
      Main.cpp \
      MainWindow.cpp \
      RNAStructure.cpp \
      RNAStructViz.cpp \
      StructureManager.cpp \
      Fl_Rotated_Text.cpp \
      rocbox.cpp
RNASTRUCTVIZ_OBJECTS = BranchTypeIdentification.$(OBJEXT) \
	DiagramWindow.$(OBJEXT) StatsWindow.$(OBJEXT) \
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
	cp $(BINARY_OBJECT) /usr/local/bin

BranchTypeIdentification.$(OBJEXT): RNAStructure.h BranchTypeIdentification.h\
	BranchTypeIdentification.cpp
	$(CXX) $(CXXFLAGS_FULL) -c BranchTypeIdentification.cpp

DiagramWindow.$(OBJEXT): DiagramWindow.h RNAStructViz.h \
	BranchTypeIdentification.h RNAStructure.h DiagramWindow.cpp
	$(CXX) $(CXXFLAGS_FULL) -c DiagramWindow.cpp

StatsWindow.$(OBJEXT): StatsWindow.h StructureManager.h RNAStructViz.h\
	RNAStructure.h InputWindow.h StatsWindow.cpp
	$(CXX) $(CXXFLAGS_FULL) -c StatsWindow.cpp

GLWindow.$(OBJEXT): GLWindow.h
	$(CXX) $(CXXFLAGS_FULL) -c GLWindow.cpp

InputWindow.$(OBJEXT): InputWindow.h InputWindow.cpp
	$(CXX) $(CXXFLAGS_FULL) -c InputWindow.cpp

FolderWindow.$(OBJEXT): FolderWindow.h StructureManager.h RNAStructViz.h\
	MainWindow.h FolderWindow.cpp
	$(CXX) $(CXXFLAGS_FULL) -c FolderWindow.cpp

Main.$(OBJEXT): MainWindow.h RNAStructViz.h Main.cpp
	$(CXX) $(CXXFLAGS_FULL) -c Main.cpp

MainWindow.$(OBJEXT): MainWindow.h RNAStructViz.h MainWindow.cpp
	$(CXX) $(CXXFLAGS_FULL) -c MainWindow.cpp

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

