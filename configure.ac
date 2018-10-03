#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

# AC_PREREQ([2.67])
AC_INIT([RNAStructViz], [1.0], [BUG-REPORT-ADDRESS])
AC_CONFIG_SRCDIR([README])
AC_CONFIG_HEADERS([rnastructviz_config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for libraries.

CXXFLAGS_EXTRA="-std=c++11 -DFLTK_HAVE_CAIRO `fltk-config --use-images --cflags` `pkg-config --cflags cairo`"
#LDFLAGS_EXTRA="/usr/lib/x86_64-linux-gnu/libfltk_forms.so /usr/lib/x86_64-linux-gnu/libfltk_gl.so /usr/lib/x86_64-linux-gnu/libfltk_images.so /usr/lib/x86_64-linux-gnu/libfltk.so /usr/lib/x86_64-linux-gnu/mesa/libGL.so.1 -lX11 `pkg-config --libs cairo`"
LDFLAGS_EXTRA="-L/usr/local/lib -std=c++11 `fltk-config --libs` -lfltk -lfltk_gl -lfltk_cairo -lfltk_images -lstdc++ -lXfixes -lXext -lpthread -ldl -lm -lX11 `pkg-config --libs gl` `pkg-config --libs glu` `pkg-config --libs xext` `pkg-config --libs xfixes` -ldl `pkg-config --libs fontconfig` `pkg-config --libs xrender` `pkg-config --libs cairo-xlib-xrender` `pkg-config --libs xft` `pkg-config --libs xinerama` `pkg-config --libs xcursor` -lX11 `pkg-config --libs cairo`"

AC_SUBST([CXXFLAGS_EXTRA])
AC_SUBST([LDFLAGS_EXTRA])

AM_INIT_AUTOMAKE([RNAStructViz], [1.0])
AC_OUTPUT(Makefile src/Makefile)

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([getcwd memset sqrt strdup strrchr])

AC_OUTPUT
