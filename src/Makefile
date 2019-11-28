#### Makefile 
#### Author: Maxie D. Schmidt (maxieds@gmail.com)
#### Updated: 2019.03.05 (V2)

TRUE=1
FALSE=0

SEDCMD=`which sed`

TARGET_PLATFORM := $(shell uname -s)
ifeq "$(TARGET_PLATFORM)" "Darwin"
	SEDCMD=`which gsed`
endif

define GetBuildConfigOption =
$(shell cat ../scripts/BuildConfig.cfg | $(SEDCMD) -n 's/^$(1)\=\([0-9][0-9]*\)/\1/p')
endef

DEBUGGING_ARG=DEBUGGING
VERBOSE_ARG=VERBOSE
OPTLEVEL_ARG=OPTLEVEL
DEBUGLEVEL_ARG=DEBUGGING_LEVEL
BRANCHID_ARG=BRANCH_TYPE_ID

DEBUG=$(call GetBuildConfigOption,${DEBUGGING_ARG})
VERBOSE=$(call GetBuildConfigOption, ${VERBOSE_ARG})
OPTLEVEL=$(call GetBuildConfigOption,${OPTLEVEL_ARG})
DEBUGLEVEL=$(call GetBuildConfigOption,${DEBUGLEVEL_ARG})
BRANCHID=$(call GetBuildConfigOption,${BRANCHID_ARG})

## fallback values since Mac is broken as a platform:
ifeq "$(TARGET_PLATFORM)" "Darwin"
	DEBUG=0
	OPTLEVEL=0
	DEBUGLEVEL=3
	BRANCHID=0
endif

CXX=g++
CXXFLAGS= -g$(DEBUGLEVEL) -O$(OPTLEVEL) -march=native
LD=g++
LDFLAGS= -g$(DEBUGLEVEL) -lm -fsanitize=address -fno-omit-frame-pointer

ifeq "$(VERBOSE)" "$(TRUE)"
	CXXFLAGS += -Wall -Wextra 
	LDFLAGS += -Wall -Wextra 
endif
ifeq "$(DEBUG)" "$(TRUE)"
	CXXFLAGS += -fsanitize=address -fno-omit-frame-pointer
	LDFLAGS += -fsanitize=address -fno-omit-frame-pointer#-pedantic-errors
endif

.DEFAULT: default

# Optional compile with clang compiler:
BUILD_WITH_COMPILER := $(shell env | grep STRUCTVIZ_COMPILER | sed -e 's/^.*=//')
ifneq "$(BUILD_WITH_COMPILER)" ""
	# add necessary CXX/LDFLAGS for clang++:
	CLANG_EXTRA_CXXFLAGS= -std=c++17 -stdlib=libc++ \
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
ifeq "$(TARGET_PLATFORM)" "Darwin"
	FLTKCONFIG=/usr/local/opt/fltkwithcairo/bin/fltk-config
	READLINK=$(shell which greadlink)
	DEBUGGER=$(shell which lldb)
	CXXFLAGS+= -std=c++17 -Wno-c++11-narrowing -Wno-c++17-narrowing -I/usr/local/opt/openssl/include 
	LDFLAGS+= -L/usr/local/opt/openssl/lib
else
	FLTKCONFIG=$(shell which fltk-config)
	READLINK=$(shell which readlink)
	DEBUGGER=$(shell which gdb)
	CXXFLAGS+= -std=gnu++17 -I/usr/include
	LDFLAGS+= -Wl,--no-as-needed
endif

.EXPORT_ALL_VARIABLES:
CFLAGS_FLTK_OVERRIDES=
OTHER_CFLAGS_OVERRIDES=
OTHER_LDFLAGS_OVERRIDES=

# option to build FLTK without sudo on Linux:
BUILD_FLTK_LOCAL := $(shell env | grep FLTK_INSTALL_DIR | sed -e 's/^.*=//')
ifneq "$(BUILD_FLTK_LOCAL)" ""
	BUILD_FLTK_LOCAL_ABS=$(shell $(READLINK) -f $(BUILD_FLTK_LOCAL))
	FLTKCONFIG=$(BUILD_FLTK_LOCAL_ABS)/fltk-config
        CFLAGS_FLTK_OVERRIDES=-isystem $(BUILD_FLTK_LOCAL_ABS)
	CPLUS_INCLUDE_PATH=$(BUILD_FLTK_LOCAL_ABS)
	LIBRARY_PATH=$(BUILD_FLTK_LOCAL_ABS)/lib
endif

BUILD_CFLAGS_USER_EXTRAS := $(shell env | grep STRUCTVIZ_EXTRA_CFLAGS | sed -e 's/^.*=//')
ifneq "$(BUILD_CFLAGS_USER_EXTRAS)" ""
	OTHER_CFLAGS_OVERRIDES=-isystem $(BUILD_CFLAGS_USER_EXTRAS)
endif

BUILD_LDFLAGS_USER_EXTRAS := $(shell env | grep STRUCTVIZ_EXTRA_LDFLAGS | sed -e 's/^.*=//')
ifneq "$(BUILD_LDFLAGS_USER_EXTRAS)" ""
	OTHER_LDFLAGS_OVERRIDES=$(BUILD_LDFLAGS_USER_EXTRAS)
endif

BINARY_OUTPUT=RNAStructViz
OBJEXT=o

CXXFLAGS_PKGCONFIG=$(shell ../scripts/pkg-config-flags.sh --cflags)
CXXFLAGS_FLTK=$(shell $(FLTKCONFIG) --use-gl --use-images --use-glut --use-forms --use-cairo --cxxflags)
CXXFLAGS_EXTRA=$(CFLAGS_FLTK_OVERRIDES) $(CXXFLAGS_PKGCONFIG) $(CXXFLAGS_FLTK) \
	       -DFLTK_HAVE_CAIRO -DFLTKSTATIC -D_LARGEFILE_SOURCE \
	       -D_LARGEFILE64_SOURCE -funsigned-char -fwrapv -ffloat-store -freciprocal-math -ffinite-math-only \
		$(shell ../scripts/generate-build-config-cflags.sh $(shell $(READLINK) -f ../scripts/BuildConfig.cfg))

CXXFLAGS_FULL=$(BUILD_CFLAGS_USER_EXTRAS) $(CXXFLAGS_EXTRA) $(CXXFLAGS) 

LDFLAGS_PKGCONFIG=$(shell ../scripts/pkg-config-flags.sh --libs)
LDFLAGS_FLTK=$(shell $(FLTKCONFIG) --use-gl --use-images --use-glut --use-forms --use-cairo --ldstaticflags)
LDFLAGS_EXTRA=$(LDFLAGS_FLTK) $(LDFLAGS_PKGCONFIG) -lssl -lcrypto -lboost_system -lboost_filesystem
LDFLAGS_FULL=$(BUILD_LDFLAGS_USER_EXTRAS) $(LDFLAGS_EXTRA) $(LDFLAGS) 

RNASTRUCTVIZ_OBJECTS = \
	BaseSequenceIDs.$(OBJEXT) BranchTypeIdentification.$(OBJEXT) CairoDrawingUtils.$(OBJEXT) \
	CommonDialogs.$(OBJEXT) ConfigParser.$(OBJEXT) DiagramWindow.$(OBJEXT) \
	DisplayConfigWindow.$(OBJEXT)\
	StatsWindow.$(OBJEXT) InputWindow.$(OBJEXT) LoadFileSelectAllButton.$(OBJEXT) \
	FolderWindow.$(OBJEXT) Main.$(OBJEXT) MainWindow.$(OBJEXT) \
	OptionParser.$(OBJEXT) \
	RadialLayoutImage.$(OBJEXT) RNAStructure.$(OBJEXT) RNAStructViz.$(OBJEXT) \
	StructureManager.$(OBJEXT) StructureType.$(OBJEXT) TerminalPrinting.$(OBJEXT) \
	Fl_Rotated_Text.$(OBJEXT) 

default: prelims $(RNASTRUCTVIZ_OBJECTS)
	@$(LD) -o $(BINARY_OUTPUT) $(RNASTRUCTVIZ_OBJECTS) $(LDFLAGS_FULL)

help: 
	@echo "RNAStructViz Makefile targets and options:"
	@echo "  >> default, e.g., run \'make\'     : Build the application, but do not clean sources or run it"
	@echo "  >> default_with_gperf              : Debugging target for developers"
	@echo "  >> prelims                         : Copy the local build header using a script (called by some targets automatically)"
	@echo "  >> diagnostic                      : Print diagnostic information about the compiler and library source locations"
	@echo "  >> run                             : Build and run RNAStructViz"
	@echo "  >> run-debug                       : Build and run RNAStructViz in the local debugger"
	@echo "  >> clean                           : Standard target"
	@echo "  >> install                         : (On Linux / Unix, NOT Mac OSX) Install with sudo"
	@echo "  >> profile_mem                     : Debugging target for developers"
	@echo "  >> git-add                         : Add all relevant (NOTE: NOT all files) source files"
	@echo "  >> git-commit                      : Usage is 'make git commit \"<COMMIT-MSG>\"'"
	@echo "  >> git-push                        : Usage is 'make git-push \"<COMMIT-MSG>\"'"
	@echo ""
	@echo "RNAStructViz Makefile hidden options and environment config settings:"
	@echo "  >> STRUCTVIZ_COMPILER      [exported env variable] : Set a non-standard compiler for the build (default is g++)"
	@echo "  >> FLTK_INSTALL_DIR        [exported env variable] : For local installs (FLTK build location)"
	@echo "  >> STRUCTVIZ_EXTRA_CFLAGS  [exported env variable] : For local installs and local include paths"
	@echo "  >> STRUCTVIZ_EXTRA_LDFLAGS [exported env variable] : For local installs and local library paths"

default_with_gperf: prelims $(RNASTRUCTVIZ_OBJECTS)
	$(LD) -o $(BINARY_OUTPUT) $(RNASTRUCTVIZ_OBJECTS) $(LDFLAGS_FULL) -lprofiler -ltcmalloc

prelims:
	@rm -f BuildTargetInfo.h
	@/bin/bash $(shell $(READLINK) -f ../scripts/build-platform-header.sh) \
		$(shell $(READLINK) -f ./BuildTargetInfo.h) \
		$(shell $(READLINK) -f $(FLTKCONFIG))

diagnostic:
	@echo "BuildFLTKLocal: " $(BUILD_FLTK_LOCAL)
	@echo "FLTK-CONFIG PATH: " $(FLTKCONFIG)
	@$(CXX) --version -v $(CXXFLAGS_FULL)

run: clean default 
	@echo ""
	@./RNAStructViz --about
	@echo ""
	./RNAStructViz

run-debug: clean default
	@echo ""
	@./RNAStructViz --about
	@echo ""
	$(DEBUGGER) $(shell $(READLINK) -f ./RNAStructViz)

clean:
	@rm -f *.o *.code *.log *.callgrind *.out *.ps *.pdf \
		BuildTargetInfo.h* $(BINARY_OUTPUT) $(RNASTRUCTVIZ_OBJECTS)

install: default
	if test -z "$$INSTALL_PREFIX"; then INSTALL_PREFIX=/usr/local/; fi;\
	echo "Install Prefix: $$INSTALL_PREFIX";\
	mkdir -p $(INSTALL_PREFIX)/bin;\
	mkdir -p $(INSTALL_PREFIX)/share/RNAStructViz;\
	mkdir -p $(INSTALL_PREFIX)/share/RNAStructViz/sample-structures;\
	cp -ap $(BINARY_OUTPUT) $(INSTALL_PREFIX)/bin/;\
	cp -ap ../sample-structures/* \
		$(INSTALL_PREFIX)/share/RNAStructViz/sample-structures/
	chmod 666 $(INSTALL_PREFIX)/share/RNAStructViz/sample-structures/
	chmod 666 $(INSTALL_PREFIX)/share/RNAStructViz/sample-structures/*

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

git-add: 
	@echo -n $(git add --ignore-errors ./*.cpp ./*.h ./*.H ./Interfaces/*.h ./Interfaces/*.cpp ./pixmaps/*.c Makefile ../scripts/* ../Makefile)

git-commit: git-add
	@git commit -m '$("arg=$(filter-out $@,$(MAKECMDGOALS))" && echo $${arg:-${1}})'

git-push: git-commit
	@git push origin master
	@git push fork master

%: #accept not explicitly defined targets
@: #ditto

BaseSequenceIDs.$(OBJEXT): BaseSequenceIDs.h TerminalPrinting.h \
	ConfigParser.h BaseSequenceIDs.cpp
	$(CXX) $(CXXFLAGS_FULL) -c BaseSequenceIDs.cpp

BranchTypeIdentification.$(OBJEXT): RNAStructure.h BranchTypeIdentification.h\
	BranchTypeIdentification.cpp
	$(CXX) $(CXXFLAGS_FULL) -c BranchTypeIdentification.cpp

CairoDrawingUtils.$(OBJEXT): CairoDrawingUtils.h TerminalPrinting.h CairoDrawingUtils.cpp
	$(CXX) $(CXXFLAGS_FULL) -c CairoDrawingUtils.cpp

CommonDialogs.$(OBJEXT): CommonDialogs.h ConfigOptions.h ConfigParser.h \
	RNAStructViz.h ConfigExterns.h CommonDialogs.cpp
	$(CXX) $(CXXFLAGS_FULL) -c CommonDialogs.cpp

ConfigOptions.h: ThemesConfig.h ConfigExterns.h RNACUtils.cpp ../scripts/BuildTargetInfo.h.in

ConfigParser.$(OBJEXT): ConfigOptions.h ConfigParser.h \
	TerminalPrinting.h ConfigOptions.h CommonDialogs.h RNAStructViz.h \
	DisplayConfigWindow.h ConfigExterns.h ThemesConfig.h \
	ConfigParser.cpp
	$(CXX) $(CXXFLAGS_FULL) -c ConfigParser.cpp

DiagramWindow.$(OBJEXT): DiagramWindow.h RNAStructViz.h \
	BranchTypeIdentification.h RNAStructure.h TerminalPrinting.h \
	pixmaps/FivePrimeThreePrimeStrandEdgesMarker.c DiagramWindow.cpp
	$(CXX) $(CXXFLAGS_FULL) -c DiagramWindow.cpp

DisplayConfigWindow.$(OBJEXT): DisplayConfigWindow.h ConfigOptions.h \
	pixmaps/ConfigPathsIcon.c pixmaps/ConfigThemesIcon.c \
	pixmaps/PNGNewPathIcon.c pixmaps/ConfigWindowIcon.xbm \
	pixmaps/ConfigCheckBoxParams.c \
	ThemesConfig.h DisplayConfigWindow.cpp
	$(CXX) $(CXXFLAGS_FULL) -c DisplayConfigWindow.cpp

Fl_Rotated_Text.$(OBJEXT): Fl_Rotated_Text.H Fl_Rotated_Text.cpp
	$(CXX) $(CXXFLAGS_FULL) -c Fl_Rotated_Text.cpp

FolderWindow.$(OBJEXT): FolderWindow.h StructureManager.h RNAStructViz.h \
	ConfigOptions.h MainWindow.h ThemesConfig.h \
	pixmaps/StructureOperationIcon.c \
	FolderWindow.cpp
	$(CXX) $(CXXFLAGS_FULL) -c FolderWindow.cpp

InputWindow.$(OBJEXT): InputWindow.h MainWindow.h ConfigOptions.h \
	ConfigParser.h RNAStructViz.h StructureManager.h \
	FolderStructure.h BaseSequenceIDs.h ConfigExterns.h \
	InputWindow.cpp
	$(CXX) $(CXXFLAGS_FULL) -c InputWindow.cpp

LoadFileSelectAllButton.$(OBJEXT): LoadFileSelectAllButton.h ConfigOptions.h\
	ConfigExterns.h LoadFileSelectAllButton.cpp
	$(CXX) $(CXXFLAGS_FULL) -c LoadFileSelectAllButton.cpp

Main.$(OBJEXT): MainWindow.h ConfigOptions.h RNAStructViz.h\
	TerminalPrinting.h OptionParser.h ConfigExterns.h \
	Main.cpp
	$(CXX) $(CXXFLAGS_FULL) -c Main.cpp

MainWindow.$(OBJEXT): MainWindow.h ConfigOptions.h RNAStructViz.h\
	TerminalPrinting.h ThemesConfig.h \
	pixmaps/RNAWindowIcon.xbm pixmaps/HelpIcon.c MainWindow.cpp
	$(CXX) $(CXXFLAGS_FULL) -c MainWindow.cpp

OptionParser.$(OBJEXT): OptionParser.h ConfigOptions.h TerminalPrinting.h \
	OptionParser.cpp
	$(CXX) $(CXXFLAGS_FULL) -c OptionParser.cpp

RadialLayoutImage.$(OBJEXT): ConfigOptions.h CairoDrawingUtils.h DiagramWindow.h\
	RNAStructure.h ThemesConfig.h ConfigOptions.h ConfigParser.h\
	RadialLayoutImage.h RadialLayoutImage.cpp
	$(CXX) $(CXXFLAGS_FULL) -c RadialLayoutImage.cpp

RNAStructure.$(OBJEXT): RNAStructure.h ConfigOptions.h \
	BranchTypeIdentification.h pixmaps/RNAStructVizLogo.c\
	ThemesConfig.h TerminalPrinting.h BaseSequenceIDs.h InputWindow.h \
	ConfigParser.h RNAStructure.cpp
	$(CXX) $(CXXFLAGS_FULL) -c RNAStructure.cpp

RNAStructViz.$(OBJEXT): StructureManager.h RNAStructViz.h\
	MainWindow.h FolderStructure.h DiagramWindow.h\
	StatsWindow.h TerminalPrinting.h BaseSequenceIDs.h RNAStructViz.cpp
	$(CXX) $(CXXFLAGS_FULL) -c RNAStructViz.cpp

StatsWindow.$(OBJEXT): StatsWindow.h StructureManager.h ConfigOptions.h \
	RNAStructViz.h RNAStructure.h InputWindow.h TerminalPrinting.h \
	pixmaps/StatsFormula.c pixmaps/StatsWindowIcon.xbm \
	ConfigParser.h StatsWindow.cpp
	$(CXX) $(CXXFLAGS_FULL) -c StatsWindow.cpp

StructureManager.$(OBJEXT): StructureManager.h FolderStructure.h\
	FolderWindow.h MainWindow.h RNAStructViz.h InputWindow.h\
	RNAStructure.h TerminalPrinting.h StructureManager.cpp
	$(CXX) $(CXXFLAGS_FULL) -c StructureManager.cpp

StructureType.$(OBJEXT): ConfigOptions.h ImageNavButton.h ConfigExterns.h \
	RNAStructVizTypes.h StructureType.h StructureType.cpp
	$(CXX) $(CXXFLAGS_FULL) -c StructureType.cpp

TerminalPrinting.$(OBJEXT): TerminalPrinting.h ConfigOptions.h ConfigExterns.h \
	TerminalPrinting.cpp
	$(CXX) $(CXXFLAGS_FULL) -c TerminalPrinting.cpp

