# RNAStructViz source code hacking guide

## Source code individual per-file functionality and description 

### Overview (and happy hacking!) 

If any of these descriptions are unclear, or if any would-be RNAStructViz hackers still have trouble understanding the source given the relevant functionality text and/or C/C++ code logic itself, note that the developer **@maxieds** is happy to help explaining the mechanisms of any proposed modifications. 

Note that there have been a number of obvious and some less obvious changes to the historical code base for RNAStructViz beginning in the summer of 2018 for Professor Heitsch's gtDMMB group. In particular, the build process is now automake-less and is completely custom, many features have been added to make RNAStructViz more interesting to users, requisite bug fixes inherited base any large code base over time have been amended, and some under-the-hood reorganization to manage previously disjoint GUI displays and data structure storage has been merged. Much of the excellent graphics code to perform the primary arc diagram computations and drawing in FLTK remains intact. See the WIKI listing of [credits for collaboration on the source code](https://github.com/gtDMMB/RNAStructViz/wiki/CreditsAndCitations) for a better view of who is responsible for crafting RNAStructViz from first principles in C++. 

If you are considering hacking and/or significantly modifying the RNAStructViz source code for your group's needs, we of course, request to know what and how you have done it! Please post a [new issue](https://github.com/gtDMMB/RNAStructViz/issues/) with links to your fork and work on our GitHub codebase page. 

### List of active C/C++ file descriptions in the RNAStructViz source (src) directory (as of 2020.01.17)

| Filename | Description / Functionality within the RNAStructViz Application | 
| :--------: | :----------------------------------------------------------- |
| *AutoloadIndicatorButton.cpp* | Implements a custom image button in the RHS folder window which can be toggled by clicking on it to either autoload a currently loaded structure file (or not) when the application first starts. See the symlinked files generated in (on Unix) ``~/.RNAStructViz/AutoLoad/*``.| 
| *BaseSequenceIDs.cpp* | Code to implement so called "*sticky folder name*" saving to a configuration file in the users home diectory. A sticky folder name is a good description of the base sequence (used to label any associated structures) that the user has saved before to re-use in multiple instances, or launchings, of RNAStructViz. The default location of the saved sticky folder names is found (on Unix) in ``~/.RNAStructViz/sequence-folder-names.dat``. This file contains the folder name text and a modified hash of the base sequence to keep integrity and distinguish accuarately between long base sequences to which we keep folder name descriptions of. |
| *BranchTypeIdentification.cpp* | Old (non-default) implementation to color-code the 4 branches, or connected groupings, found in 16S structures. N.b., one of the first recent customizations of RNAStructViz for Professor Heitsch completed by *maxieds* in 2018. |
| *CairoDrawingUtils.cpp* | Implements extra drawing functions and conversions from FLTK colors and image types to native libcairo vector graphics representations. Primarily used to draw the customized radial layout images (initiated from the *arc diagram* window), but also utilized elsewhere in the source code. |
| *CommonDialogs.cpp* | |
| *ConfigExterns.h* | Pre-definitions of the old C-style externs used to load, define, and configure configuration settings used across multiple source files in the application. |
| *ConfigParser.cpp* | Code to implement and (re)store (aka read/write) the local user saved configuration settings located (on Unix) in the file ``~/.RNAStructViz/config.cfg``. Allows us to save and re-use modified themes, color schemes, and file save/load paths on disk between successive launches of the RNAStructViz application. |
| *DiagramWindow.cpp* | Implements the GUI frontend and functional backend code to generate the images in the *arc diagrams* sub-window that can be opened from the RHS folder window to compare loaded structures. |
| *DisplayConfigWindow.cpp* | Custom FLTK GUI dialog window used to display and change the configuration settings that can be altered on-the-fly by users at runtime. | 
| *Fl_Rotated_Text.cpp* | Primary FLTK-drawing-based implementation of sideways text (historical code intact). Only used by the graph drawing procedures in the *statistics window* that can be launched from the RHS folder pane. | 
| *FolderStructure.h* | All-in-one inlined definitions and code to support the GUI widgets (including managing their creation and deletion), per-individual-structure folder information, and to support other live operations on these RHS window paned structure displays. Note that compared to the historical code for RNAStructViz, this code allows us to integrate and manage the GUI display and the data structure usage of the ``Folder`` type (class), which used to just correspond to a non-extern-C'ed structure. This is all to say that significant modifications were required compared to the historical implementation to organize this part of the code. | 
| *FolderWindow.cpp* | Implementation of the RHS main window pane GUI display and FLTK callback mechanism logic in the application. |
| *InputWindow.cpp* | A multi-dialog implementation used in various places to gather input from the user. | 
| *LoadFileSelectAllButton.cpp* | The solitary FLTK multi-widget wrapper appended to the bottom display of the ``Fl_File_Chooser`` widget (built-in FLTK widget sub-type) shown to users after clicking the "*Load Files...*" button on the LHS ``MainWindow`` display pane in the application. It allows a non-native "hack" to load all files in directories (including possible recursive searches of subdirectories) without FLTK supporting a **CTRL+A** button press scheme as is allowed on most OSes (e.g., Linux, Mac, and even Windows). (It's also a learning good reference for FLTK (in C++) custom-coded widgetry in the view of the developer, *maxieds*, which can be difficult and painful to novices and expert C++ hackers alike ...) |
| *Main.cpp* | Contains the ``main(int argc, char **argv)`` C-style runner function. Some settings are also added here. Includes initialization of other once-per-runtime FLTK initializations are spawned from this file as well. |
| *MainWindow.cpp* | Implements the GUI and FLTK callback logic found in the LHS pane of the main RNAStructViz application window. | 
| *OptionParser.cpp* | GNU built-in C/C++ long-argument-style command line option parser implementation. Allows parsing and interpretation of runtime settings passed to RNAStructViz on the command line as [documented here](). |
| *RadialLayoutImage.cpp* | |
| *RNACUtils.cpp* | |
| *RNAStructure.cpp* | |
| *RNAStructViz.cpp* | |
| *RNAStructVizTypes.h* | |
| *StatsWindow.cpp* | |
| *StructureManager.cpp* | |
| *StructureType.cpp* | |
| *TerminalPrinting.cpp* | |
| *ThemesConfig.h* | |
| *ViennaBoltzmannSampling.cpp* | |
| *XMLExportButton.cpp* | |

## Descriptions of the newest build process (Makefiles, helper shell scripts, and rudimentary configuration docs)

### Overview and the scheme of things 

The historical application utilized an automake/autoconf-based scheme to build RNAStructViz from its source. 
Recent modifications, especially since we began to require the ``libcairo`` library an its prerequisite configurations to 
improve the visual appeal of the older-style FLTK drawing of diagrams, removed this hard to configure and understand build process 
in favor of a totally custom, and hence much simpler, from-scratch build process. This is to say that we started with a simple 
``src/Makefile`` implementation suitable for semi-smallish source code bases (as opposed to, let's say ``XOrg`` or the Linux kernel, with respectively 1000's or more of source files and configurations and dependencies to dynamically manage). As we added in more library configurations, a few new C++ library dependencies (e.g., OpenSSL, ViennaRNA, or libboost), and strived to make the build process automatically cross platform for Linux/Mac/generic-Unix without the need of multiple files to actively maintain and keep working, helper shell scripts and configuration in ``RNAStructViz/build-scripts/*`` have been custom written and added in to the build process. 

Notice in particular that the header file ``src/BuildInclude/BuildTargetInfo.h`` is automatically (re)generated with up-to-date system and git credentials and active library configuration / build information. This allows us to install a trapdoor ``SIGSEGV`` handler that prints detailed per-user RNAStructViz build and install information as any unexpected catastrophic crashes happen in the wild -- no really, you're welcome, here! The differences in syntactic sugar for standard GNU-style command options on Linux, and their pathological and uglier differences to invoke the same functionality on Apple's botched (these days) Mac command line instantiation, are the only real barrier to understanding what the relevant helper shell scripts are doing. 

### Listing of relevant files and their purposes (TODO) 

| Filaname (with sub-path) | Description and functionality | Links to WIKI documentation | 
| :----------------------: | :---------------------------- | :-------------------------- | 
| ``build-scripts/BuildConfig.cfg`` | Build-time configuration and enablers / disablers for non-default options and special settings. | 
| ``build-scripts/build-platform-header.sh`` | | |
| ``build-scripts/generate-build-config-cflags.sh`` | | |
| ``build-scripts/get-build-config-setting.sh`` | | |
| ``build-scripts/pkg-config-flags.sh`` | | |
| ``build-scripts/BuildTargetInfo.h.in`` | | |
| ``src/Makefile`` | | |

 
