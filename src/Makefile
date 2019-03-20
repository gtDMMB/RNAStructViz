#### Makefile 
#### Author: Maxie D. Schmidt (maxieds@gmail.com)
#### Updated: 2019.03.05 (V2)

CXX=g++
CXXFLAGS= -g -Wall -Wextra -pedantic -O1 #-lstdc++

LD=g++
LDFLAGS=-Wall -Wextra -g1 -lm

.DEFAULT: default

# Optional compile with clang compiler:
BUILD_WITH_COMPILER := $(shell env | grep STRUCTVIZ_COMPILER | sed -e 's/^.*=//')
ifneq "$(BUILD_WITH_COMPILER)" ""
	# add necessary CXX/LDFLAGS for clang++:
	CLANG_EXTRA_CXXFLAGS=  -std=c++17 -stdlib=libc++ \
		-I$(shell ls -d /usr/include | tail -n1)
	CLANG_EXTRA_LDFLAGS= -std=c++17 -stdlib=libc++ -lm \
		-L$(shell ls -d /usr/lib/gcc/x86_64-linux-gnu/7* | tail -n1) \
		-lgcc_s -lpthread -lrt -lsupc++
	CXX=$(BUILD_WITH_COMPILER)
	CXXFLAGS+=$(CLANG_EXTRA_CXXFLAGS)
	LD=$(BUILD_WITH_COMPILER)
	LDFLAGS+=$(CLANG_EXTRA_LDFLAGS)
endif

# OSX-specific locations and utility names:
TARGET_PLATFORM=$(shell uname -s)
ifeq "$(TARGET_PLATFORM)" "Darwin"
	FLTKCONFIG=/usr/local/opt/fltkwithcairo/bin/fltk-config
	READLINK=greadlink
	#CXX=g++-4.9
	#LD=g++-4.9
else
	FLTKCONFIG=fltk-config
	READLINK=readlink
	LDFLAGS+= -Wl,--no-as-needed
endif

.EXPORT_ALL_VARIABLES:
# option to build FLTK without sudo on Linux:
CFLAGS_FLTK_OVERRIDES=
BUILD_FLTK_LOCAL=$(shell env | grep FLTK_INSTALL_DIR | sed -e 's/^.*=//')
ifneq "$(BUILD_FLTK_LOCAL)" ""
	BUILD_FLTK_LOCAL_ABS=$(shell $(READLINK) -f $(BUILD_FLTK_LOCAL))
	FLTKCONFIG=$(BUILD_FLTK_LOCAL_ABS)/fltk-config
        CFLAGS_FLTK_OVERRIDES=-isystem $(BUILD_FLTK_LOCAL_ABS)
	CPLUS_INCLUDE_PATH=$(BUILD_FLTK_LOCAL_ABS)
	LIBRARY_PATH=$(BUILD_FLTK_LOCAL_ABS)/lib
endif

BINARY_OUTPUT=RNAStructViz
OBJEXT=o

CXXFLAGS_PKGCONFIG=`../scripts/pkg-config-flags.sh --cflags`
CXXFLAGS_FLTK=`$(FLTKCONFIG) --use-gl --use-images --use-glut --use-forms --use-cairo --cxxflags`
CXXFLAGS_EXTRA=$(CFLAGS_FLTK_OVERRIDES) $(CXXFLAGS_PKGCONFIG) $(CXXFLAGS_FLTK) \
	       -std=c++17 -DFLTK_HAVE_CAIRO -DFLTKSTATIC -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE \
	       -funsigned-char -Wno-c++11-narrowing 

CXXFLAGS_FULL=$(CXXFLAGS_EXTRA) $(CXXFLAGS) 

LDFLAGS_PKGCONFIG=`../scripts/pkg-config-flags.sh --libs`
LDFLAGS_FLTK=`$(FLTKCONFIG) --use-gl --use-images --use-glut --use-forms --use-cairo --ldstaticflags`
LDFLAGS_EXTRA=$(LDFLAGS_FLTK) $(LDFLAGS_PKGCONFIG)
LDFLAGS_FULL=$(LDFLAGS_EXTRA) $(LDFLAGS) 

RNASTRUCTVIZ_OBJECTS = \
	BranchTypeIdentification.$(OBJEXT) CairoDrawingUtils.$(OBJEXT) \
	ConfigParser.$(OBJEXT) DiagramWindow.$(OBJEXT) DisplayConfigWindow.$(OBJEXT)\
	StatsWindow.$(OBJEXT) InputWindow.$(OBJEXT) \
	FolderWindow.$(OBJEXT) Main.$(OBJEXT) MainWindow.$(OBJEXT) \
	RadialLayoutImage.$(OBJEXT) RNAStructure.$(OBJEXT) RNAStructViz.$(OBJEXT) \
	StructureManager.$(OBJEXT) Fl_Rotated_Text.$(OBJEXT)\
	rocbox.$(OBJEXT) 

default: prelims $(RNASTRUCTVIZ_OBJECTS)
	@$(LD) -o $(BINARY_OUTPUT) $(RNASTRUCTVIZ_OBJECTS) $(LDFLAGS_FULL)

default_with_gperf: prelims $(RNASTRUCTVIZ_OBJECTS)
	$(LD) -o $(BINARY_OUTPUT) $(RNASTRUCTVIZ_OBJECTS) $(LDFLAGS_FULL) -lprofiler -ltcmalloc

prelims:
	/bin/bash `$(READLINK) -f ../scripts/build-platform-header.sh` \
		`$(READLINK) -f ./BuildTargetInfo.h`\
		`$(READLINK) -f $(FLTKCONFIG)`

diagnostic:
	#echo "BuildFLTKLocal: " $(BUILD_FLTK_LOCAL)
	#echo "FLTK-CONFIG PATH: " $(FLTKCONFIG)
	$(CXX) --version -v $(CXXFLAGS_FULL)

run: default
	./RNAStructViz --about

clean:
	@rm -f *.o *.code *.log *.callgrind *.out *.ps *.pdf \
		BuildTargetInfo.h* $(BINARY_OUTPUT) $(RNASTRUCTVIZ_OBJECTS)

install: default
	if test -z "$$INSTALL_PREFIX"; then INSTALL_PREFIX=/usr/local/; fi;\
	echo "Install Prefix: $$INSTALL_PREFIX";\
	mkdir -p $(INSTALL_PREFIX)/bin;\
	mkdir -p $(INSTALL_PREFIX)/share/RNAStructViz;\
	mkdir -p $(INSTALL_PREFIX)/share/RNAStructViz/sample_structures;\
	cp -ap $(BINARY_OUTPUT) $(INSTALL_PREFIX)/bin/;\
	cp -ap ../sample_structures/* \
		$(INSTALL_PREFIX)/share/RNAStructViz/sample_structures/

profile_mem: clean CXXFLAGS+=" -DWITHGPERFTOOLS" default_with_gperf
	./RNAStructViz #--inuse_space --inuse_objects --alloc_space --alloc_objects
	google-pprof -lines\
		-add_lib=/usr/local/lib/libfltk.a\
		-add_lib=/usr/local/lib/libfltk_cairo.a\
		-add_lib=/usr/local/lib/libfltk_images.a\
		-text RNAStructViz RNAStructViz.log | less
	google-pprof -functions\
		-add_lib=/usr/local/lib/libfltk.a\
		-add_lib=/usr/local/lib/libfltk_cairo.a\
		-add_lib=/usr/local/lib/libfltk_images.a\
		-pdf RNAStructViz RNAStructViz.log > RNAStructViz-HeapProfile.pdf
	xreader RNAStructViz-HeapProfile.pdf

gitadd:
	git add ./*.{cpp,h} pixmaps/*.{c,xbm} ../scripts/* ../Makefile

BranchTypeIdentification.$(OBJEXT): RNAStructure.h BranchTypeIdentification.h\
	BranchTypeIdentification.cpp
	$(CXX) $(CXXFLAGS_FULL) -c BranchTypeIdentification.cpp

CairoDrawingUtils.$(OBJEXT): CairoDrawingUtils.h CairoDrawingUtils.cpp
	$(CXX) $(CXXFLAGS_FULL) -c CairoDrawingUtils.cpp

ConfigParser.$(OBJEXT): ConfigOptions.h ConfigParser.h ConfigParser.cpp
	$(CXX) $(CXXFLAGS_FULL) -c ConfigParser.cpp

DiagramWindow.$(OBJEXT): DiagramWindow.h RNAStructViz.h \
	BranchTypeIdentification.h RNAStructure.h\
	pixmaps/FivePrimeThreePrimeStrandEdgesMarker.c DiagramWindow.cpp
	$(CXX) $(CXXFLAGS_FULL) -c DiagramWindow.cpp

DisplayConfigWindow.$(OBJEXT): DisplayConfigWindow.h ConfigOptions.h \
	pixmaps/ConfigPathsIcon.c pixmaps/ConfigThemesIcon.c\
	pixmaps/PNGNewPathIcon.c pixmaps/ConfigWindowIcon.xbm\
	ThemesConfig.h DisplayConfigWindow.cpp
	$(CXX) $(CXXFLAGS_FULL) -c DisplayConfigWindow.cpp

StatsWindow.$(OBJEXT): StatsWindow.h StructureManager.h ConfigOptions.h \
	RNAStructViz.h RNAStructure.h InputWindow.h \
	pixmaps/StatsFormula.c pixmaps/StatsWindowIcon.xbm StatsWindow.cpp
	$(CXX) $(CXXFLAGS_FULL) -c StatsWindow.cpp

GLWindow.$(OBJEXT): GLWindow.h
	$(CXX) $(CXXFLAGS_FULL) -c GLWindow.cpp

InputWindow.$(OBJEXT): InputWindow.h InputWindow.cpp
	$(CXX) $(CXXFLAGS_FULL) -c InputWindow.cpp

Fl_Rotated_Text.$(OBJEXT): Fl_Rotated_Text.H Fl_Rotated_Text.cpp
	$(CXX) $(CXXFLAGS_FULL) -c Fl_Rotated_Text.cpp

FolderWindow.$(OBJEXT): FolderWindow.h StructureManager.h RNAStructViz.h\
	ConfigOptions.h MainWindow.h pixmaps/StructureOperationIcon.c\
	FolderWindow.cpp
	$(CXX) $(CXXFLAGS_FULL) -c FolderWindow.cpp

Main.$(OBJEXT): MainWindow.h ConfigOptions.h RNAStructViz.h\
	Debugging.h Main.cpp
	$(CXX) $(CXXFLAGS_FULL) -c Main.cpp

MainWindow.$(OBJEXT): MainWindow.h ConfigOptions.h RNAStructViz.h\
	pixmaps/RNAWindowIcon.xbm pixmaps/HelpIcon.c MainWindow.cpp
	$(CXX) $(CXXFLAGS_FULL) -funsigned-char -c MainWindow.cpp

RadialLayoutImage.$(OBJEXT): ConfigOptions.h CairoDrawingUtils.h DiagramWindow.h\
	RadialLayoutImage.h RadialLayoutImage.cpp
	$(CXX) $(CXXFLAGS_FULL) -c RadialLayoutImage.cpp

RNAStructure.$(OBJEXT): RNAStructure.h ConfigOptions.h \
	BranchTypeIdentification.h pixmaps/RNAStructVizLogo.c\
	ThemesConfig.h RNAStructure.cpp
	$(CXX) $(CXXFLAGS_FULL) -c RNAStructure.cpp

RNAStructViz.$(OBJEXT): StructureManager.h RNAStructViz.h\
	MainWindow.h FolderStructure.h DiagramWindow.h\
	StatsWindow.h RNAStructViz.cpp
	$(CXX) $(CXXFLAGS_FULL) -c RNAStructViz.cpp

StructureManager.$(OBJEXT): StructureManager.h FolderStructure.h\
	FolderWindow.h MainWindow.h RNAStructViz.h InputWindow.h\
	RNAStructure.h StructureManager.cpp
	$(CXX) $(CXXFLAGS_FULL) -c StructureManager.cpp

rocbox.$(OBJEXT): rocbox.h rocbox.cpp
	$(CXX) $(CXXFLAGS_FULL) -c rocbox.cpp

