# Welcome to the RNAStructViz install documentation!

## Application Description 

RNAStructViz is an open source GUI for viewing and visualizing RNA structures encoded in the [CT and FASTA](https://rna.urmc.rochester.edu/Text/File_Formats.html) file formats (see also [here](http://projects.binf.ku.dk/pgardner/bralibase/RNAformats.html) and [here](http://www.ibi.vu.nl/programs/k2nwww/static/data_formats.html) for more information on the CT file format). Examples of such files can be found in online databases including the [NCBI databases](https://www.ncbi.nlm.nih.gov/guide/dna-rna/), [here (example.ct)](https://software.broadinstitute.org/software/igv/sites/cancerinformatics.org.igv/files/example.ct), [here (RA7680.ct)](http://rna.urmc.rochester.edu/RNAstructureWeb/Examples/RA7680.ct), and in the following databases (FASTA formatted): [RefSeq](ftp://ftp.ncbi.nlm.nih.gov/refseq/release/), [BLAST](ftp://ftp.ncbi.nlm.nih.gov/blast/db/FASTA), [TraceDB](ftp://ftp.ncbi.nlm.nih.gov/pub/TraceDB), [UniGene](ftp://ftp.ncbi.nlm.nih.gov/repository/UniGene/), and [UniVec](ftp://ftp.ncbi.nlm.nih.gov/pub/UniVec/). There are also external programs for conversions between the DOT bracket and CT file formats: [ct2dot](https://rna.urmc.rochester.edu/Text/ct2dot.html) ([online interface](http://rna.urmc.rochester.edu/RNAstructureWeb/Servers/ct2dot/ct2dot.html)) and [dot2ct](https://rna.urmc.rochester.edu/Text/dot2ct.html) ([online interface](http://rna.urmc.rochester.edu/RNAstructureWeb/Servers/dot2ct/dot2ct.html)). 

## More documentation (WIKI site)

Our WIKI site with user documentation and may other topics is available 
[here](https://github.com/gtDMMB/RNAStructViz/wiki) 
(with *screenshots* [here](https://github.com/gtDMMB/RNAStructViz/wiki/Screenshots)). 
Note that this WIKI is a good reference for users **after** the application has been installed on your 
target system of choice. The remainder of this document focuses on installation procedures. 
We currently support brew formula installs of this software based on *tap* recipes from our 
[local Homebrew repository](https://github.com/gtDMMB/homebrew-core) on recent Mac OSX 
(including *Mojave* where FLTK builds can be notably broken). 
We also, of course, support *from-source* installs of ``RNAStructViz`` on Linux, Mac OSX, *BSD, and any other 
sane Unix-like platforms. Unfortunately, we do not have the resources to support users running only in 
Windows (10) environments.

## Install recipes

We have the following specific instructions for several common operating system variants. 
If you do not see your platform supported below, or have issues installing with our instructions, 
consider contacting us -- *or especially* -- posting a 
[new issue]() in this repository.
* [Mac OSX using Homebrew only](https://github.com/gtDMMB/RNAStructViz/tree/with-cairo#mac-osx-using-homebrew-only)
* [Mac OSX from source](https://github.com/gtDMMB/RNAStructViz/tree/with-cairo#mac-osx-using-homebrew-fltk-with-cairo-and-rnastructviz-from-source) (see also [RNAStructViz from-source instructions](https://github.com/gtDMMB/RNAStructViz/tree/with-cairo#rnastructviz-from-source-install-instructions))
* [Debian-based Linux from source (requires sudo)](https://github.com/gtDMMB/RNAStructViz/tree/with-cairo#debian-based-linux-from-source-requires-sudo) (see also [RNAStructViz from-source instructions](https://github.com/gtDMMB/RNAStructViz/tree/with-cairo#rnastructviz-from-source-install-instructions))
* [Generic Linux from source (local FLTK library build)](https://github.com/gtDMMB/RNAStructViz/tree/with-cairo#generic-linux-from-source-local-fltk-library-build) (see also [RNAStructViz from-source instructions](https://github.com/gtDMMB/RNAStructViz/tree/with-cairo#rnastructviz-from-source-install-instructions))
* [RNAStructViz from-source install instructions](https://github.com/gtDMMB/RNAStructViz/tree/with-cairo#rnastructviz-from-source-install-instructions) (generic Unix assuming the correct libraries are installed)


# Installation on Mac OSX

## Mac OSX using Homebrew only

Note that the following command *should* be all that you need to run on Mac OSX to install
``RNAStructViz``with the userland [Homebrew](https://brew.sh) installed on your machine:
```
$ brew install --build-from-source gtDMMB/core/RNAStructViz
```
If you run into problems executing this command, we may first 
need to install some other basic libraries and utilities by running the following:
```
$ brew install cairo pkg-config git coreutils
$ brew install --build-from-source gtDMMB/core/fltkwithcairo
$ brew install --build-from-source gtDMMB/core/RNAStructViz
```
Now to make sure that this application is always in your path, add the following to your 
``~/.bashrc`` config file and run the following from your terminal:
```
$ echo -e "export PATH=/usr/local/opt/rnastructviz/bin:$(PATH)" >> ~/.bashrc
$ source ~/.bashrc
$ which RNAStructViz
/usr/local/opt/rnastructviz/bin/RNAStructViz
```
Assuming all went smoothly with the above instructions, you can now run the application by 
typing ``RNAStructViz`` at your terminal.

## Mac OSX using Homebrew FLTK (with-cairo) and RNAStructViz from-source

### Prerequisites 

It appears based on my recent testing that reasonably modern enough 
versions of Max OSX will allow for our libraries and the ``RNAStructViz`` 
source code to be built all with the default xcode compilers (and a little 
ingenuity :smile:).
First, some prerequisites from the test machine's xcode installation. Make sure that 
you can at least run the following commands for reference: 
```
$ which clang
/usr/bin/clang
$ which gcc
/usr/bin/gcc
$ clang --version -v
Apple LLVM version 10.0.0 (clang-1000.10.44.2)
Target: x86_64-apple-darwin18.0.0
Thread model: posix
InstalledDir: /Library/Developer/CommandLineTools/usr/bin
```
If your output does not match the above, still proceed with the install instructions. 
We are working to test the install process on as many recent versions of OSX as 
possible, so your setup may still very well be sufficient to install our required 
libraries to run the application.

### Installing applications with Homebrew

We need to install some other basic libraries and utilities with the userland 
[Homebrew](https://brew.sh) installed on your machine:
```
$ brew install cairo pkg-config git coreutils
$ brew install --build-from-source gtDMMB/core/fltkwithcairo
```
Now we need to add the new FLTK libraries into our ``PATH``:
```
$ export PATH="/usr/local/opt/fltkwithcairo/bin:$PATH"
$ echo $PATH
```
You can verify that the install was successful by verifying that the following output is sane:
```
$ which fltk-config
/usr/local/opt/fltkwithcairo/bin/fltk-config
$ fltk-config --use-gl --use-glut --use-forms --use-images --use-cairo --ldstaticflags --cxxflags
-I/usr/local/Cellar/fltkwithcairo/1.4.x-r13071/include -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_THREAD_SAFE -D_REENTRANT
/usr/local/Cellar/fltkwithcairo/1.4.x-r13071/lib/libfltk_cairo.a -lcairo -lpixman-1 /usr/local/Cellar/fltkwithcairo/1.4.x-r13071/lib/libfltk_images.a -ljpeg -lpng -lz /usr/local/Cellar/fltkwithcairo/1.4.x-r13071/lib/libfltk_gl.a -framework OpenGL /usr/local/Cellar/fltkwithcairo/1.4.x-r13071/lib/libfltk_forms.a /usr/local/Cellar/fltkwithcairo/1.4.x-r13071/lib/libfltk.a -lpthread -framework Cocoa
```
That's it! Now on to building [RNAStructViz from source](https://github.com/gtDMMB/RNAStructViz/tree/with-cairo#rnastructviz-from-source-install-instructions).

# Installation on Linux

## Debian-based Linux from source (requires sudo)

First install the Mesa/GL libs with apt if you do not already have them on your system:
```
$ sudo apt-get install libmesa-dev mesa-common-dev libxinerama-dev
```
If you are running an older version, of say Ubuntu 14.xx, the install of the package ``libmesa-dev`` may fail. If this happens, try installing the required packages in the above line by running:
```
$ sudo apt-get install libglu1-mesa-dev
```
If problems with the package names still arise, you can try searching for the correct ``mesa-dev`` package to install by issuing the following command:
```
apt-cache search mesa-dev
```
Once these libraries are installed make sure that you have the development packages for 
Cairo installed by running:
```
$ sudo apt-get install libcairo2-dev
```
Finally, install the ``apt`` packages for FLTK *with cairo support*:
```
$ sudo apt-get install libfltk1.3 libfltk1.3-dev libfltk-cairo1.3 libfltk-gl1.3
```
You can verify that the install was successful by verifying that the following output is sane:
```
$ which fltk-config
/usr/local/bin/fltk-config
$ fltk-config --libs --use-cairo
```
Now we need to build ``RNAStructViz`` from source using 
[these instructions](https://github.com/gtDMMB/RNAStructViz/tree/with-cairo#rnastructviz-from-source-install-instructions).

## Generic Linux from source (local FLTK library build)

The stock binary install of modern FLTK versions will in all likelihood not have Cairo 
support enabled by default. We require for parts of the windowing and enhanced drawing 
routines used in this branch of ``RNAStructViz``. 
We assume that you will have *GL*, *Cairo*, etc. along with other dependencies needed on your system to 
build the FLTK windowing library (*with-cairo support*) on your local system. If this is already the 
case, the you will not need ``sudo``, or superuser, privileges to install ``RNAStructViz`` from source.
If for some reason these dependencies are not met on your Linux system, then we recommend that you 
contact your system administrator to install these necessary packages for you before proceeding to the 
next step. You can verify that the appropriate libraries are installed by running the following:
```
$ pkg-config glu --cflags --libs
-I/usr/include/libdrm -lGLU -lGL
$ pkg-config cairo --cflags --libs
-I/usr/include/cairo -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/pixman-1 -I/usr/include/freetype2 -I/usr/include/libpng16 -I/usr/include/freetype2 -I/usr/include/libpng16 -lcairo
$ glxinfo | grep "OpenGL version"
OpenGL version string: 3.0 Mesa 18.0.5
```
Note that the output of these commands does not need to exactly match what appears above. The sample 
output from the previous commands is intended to give the user a good sense if you're on the right track. 

### Installing FLTK from source

First, we fetch and extract a recent stable version of the FLTK library source (v1.3.x) and extract it 
to our local home directory:
```
$ cd ~
$ wget http://fltk.org/pub/fltk/1.3.4/fltk-1.3.4-2-source.tar.bz2 
$ if [[ "$(echo $(sha256sum < fltk-1.3.4-2-source.tar.bz2) | sed -e 's/[[:blank:]-]*$//')" == "8cfe7690d70f9a3db5cd88748a82aa7958a9dc7ec3d7e94eef9063e107864150" ]]; then echo "SHA256 SUM OK"; else echo "SHA256 SUM IS MALFORMED ... ABORT!"; fi
$ tar xvjf fltk-1.3.4-2-source.tar.bz2
$ cd fltk-1.3.4-2
```
Now we need to enable the configure-time options which will enable Cairo support by default in 
our FLTK build: 
```
$ ./configure --enable-cairo --enable-threads
$ make 
```
That's almost it! 

### Setting export variables

Next, to tell the ``RNAStructViz`` *Makefile* where to find your local install of FLTK, we need to 
export the following variable:
```
$ export FLTK_INSTALL_DIR=$(readlink -f ./)
```
Finally, continue on to the building ``RNAStructViz`` from source step 
[below](https://github.com/gtDMMB/RNAStructViz/tree/with-cairo#rnastructviz-from-source-install-instructions).

# RNAStructViz from-source install instructions

Now on to building RNAStructViz from source.
We need to fetch a local copy of the RNAStructViz source code (using the new ``with-cairo`` 
branch) so that we can build it for ourselves:
```
$ cd ~
$ git clone git@github.com:gtDMMB/RNAStructViz.git
$ cd RNAStructViz
```
Alternately, without the need for the public-key-based checkout, run: 
```
$ cd ~
$ git clone https://github.com/gtDMMB/RNAStructViz.git
$ cd RNAStructViz
```
Now we must switch to the current *with-cairo* branch source: 
```
$ git checkout with-cairo
```
Now we are ready to build the source: 
```
$ make clean && make && make run
```
At this point if all goes well you should have a working copy of the RNAStructViz binary in the src/ directory. Let's make an alias for this command so it's easier to find and use on the system:
```
$ echo "alias RNAStructViz=\'$(readlink -f src/RNAStructViz)'" >> ~/.bashrc
$ source ~/.bashrc
$ RNAStructViz
```
Alternately, if you want your recent build of ``RNAStructViz`` innstalled in your default 
system search path, you can choose to just install the application globally on 
your system by running the following command:
```
$ sudo make install
```
Enjoy the application -- and don't forget to stop by the 
[WIKI site](https://github.com/gtDMMB/RNAStructViz/wiki) for more 
user documentation!
