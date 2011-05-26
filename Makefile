INCLUDE_PATH = -I/sw/include -I/Users/Shel/Downloads/fltk-1.1.10 -I/Developer/SDKs/MacOSX10.5.sdk/usr/X11/include/
LIBRARY_PATH = -L/sw/lib -L/Developer/SDKs/MacOSX10.5.sdk/usr/X11/lib/ -L/Users/Shel/Downloads/fltk-1.1.10/lib/ -L/Users/Shel/Downloads/fltk-1.1.10/
CPPFLAGS = $(INCLUDE_PATH) -arch i386 -g -Wall -U__APPLE__
LDFLAGS = -arch i386 -g -Wall -U__APPLE__
CC = g++

SRC = DiagramWindow.cpp \
      GLWindow.cpp \
      Main.cpp \
      MainWindow.cpp \
      RNAStructure.cpp \
      RNAStructViz.cpp \
      StructureManager.cpp

LIBS = -lfltk -lfltk_gl -lX11 -lGL -lGLu -framework Carbon
#LIBS =  -lX11 -lGL -lGLu -framework Carbon

OBJS = $(SRC:.cpp=.o)

%.o: %.cpp
	$(CC) -c $(CPPFLAGS) $< -o $@

RNAStructViz: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBRARY_PATH) $(LIBS)

clean:
	rm -f $(OBJS)

depends:
	makedepend $(INCLUDE_PATH) $(SRC)

# DO NOT DELETE
