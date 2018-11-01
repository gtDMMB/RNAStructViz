# Welcome to the RNAStructViz wiki!

<img src="https://github.com/gtDMMB/RNAStructViz/blob/with-cairo/wiki-images/microscope256.png" width="200"/><img src="https://github.com/gtDMMB/RNAStructViz/blob/with-cairo/wiki-images/dna256v3.png" width="200" /><img src="https://github.com/gtDMMB/RNAStructViz/blob/with-cairo/wiki-images/dna256v5.png" width="200" /><img src="https://github.com/gtDMMB/RNAStructViz/blob/with-cairo/wiki-images/file256.png" width="200" />

## Application Description 

RNAStructViz is an open source GUI for viewing and visualizing RNA structures encoded in the [CT and FASTA](https://rna.urmc.rochester.edu/Text/File_Formats.html) file formats (see also [here](http://projects.binf.ku.dk/pgardner/bralibase/RNAformats.html) and [here](http://www.ibi.vu.nl/programs/k2nwww/static/data_formats.html) for more information on the CT file format). Examples of such files can be found in online databases including the [NCBI databases](https://www.ncbi.nlm.nih.gov/guide/dna-rna/), [here (example.ct)](https://software.broadinstitute.org/software/igv/sites/cancerinformatics.org.igv/files/example.ct), [here (RA7680.ct)](http://rna.urmc.rochester.edu/RNAstructureWeb/Examples/RA7680.ct), and in the following databases (FASTA formatted): [RefSeq](ftp://ftp.ncbi.nlm.nih.gov/refseq/release/), [BLAST](ftp://ftp.ncbi.nlm.nih.gov/blast/db/FASTA), [TraceDB](ftp://ftp.ncbi.nlm.nih.gov/pub/TraceDB), [UniGene](ftp://ftp.ncbi.nlm.nih.gov/repository/UniGene/), and [UniVec](ftp://ftp.ncbi.nlm.nih.gov/pub/UniVec/). There are also external programs for conversions between the DOT bracket and CT file formats: [ct2dot](https://rna.urmc.rochester.edu/Text/ct2dot.html) ([online interface](http://rna.urmc.rochester.edu/RNAstructureWeb/Servers/ct2dot/ct2dot.html)) and [dot2ct](https://rna.urmc.rochester.edu/Text/dot2ct.html) ([online interface](http://rna.urmc.rochester.edu/RNAstructureWeb/Servers/dot2ct/dot2ct.html)). 

# Installation on Unix-based systems (Linux, OSX, BSD)

The stock binary install of modern FLTK versions will in all likelihood not have Cairo 
support enabled by default. We require for parts of the windowing and enhanced drawing 
routines used in this branch of ``RNAStructViz``. Therefore we will need to build FLTK from 
source with the Cairo configure-time options. Fortunately, this is easy enough with a 
terminal and a little know-how. Let's expand on this concept below. Note that our FLTK 
install (read: build from source) is *very* customized on the OSX platform and hence will 
require the user to have administrator, or ``sudo``, rights to get this necessary library 
installed. 

## Installing the Cairo and FLTK libraries on Linux (Debian-based, e.g., Ubuntu or Linux Mint)

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
#### Building FLTK from source 

Now comes the main step in this configuration process: building FLTK *with Cairo support enabled*. 
First, fetch a copy of the most recent FLTK source: 
```
$ cd ~
$ wget http://fltk.org/pub/fltk/1.3.4/fltk-1.3.4-2-source.tar.bz2 
$ tar xvjf fltk-1.3.4-2-source.tar.bz2
$ cd fltk-1.3.4-2
```
Now we need to enable the configure-time options which will enable Cairo support by default in 
our FLTK build: 
```
$ ./configure --enable-cairo --enable-threads
$ make 
$ sudo make install
```
You can verify that the install was successful by verifying that the following output is sane:
```
$ which fltk-config
/usr/local/bin/fltk-config
```
That's it! Now on to building RNAStructViz from source.

## Installing the Cairo and FLTK libraries on a Mac 

### Prerequisites 

It appears based on my recent testing that reasonably modern enough 
versions of Max OSX will allow for our libraries and the ``RNAStructViz`` 
source code to be built all with the default xcode compilers (and a little 
ingenuity :smile:). Thanks to the grant support of Chirstine Heitsch at 
Georgia Tech, I was able to test this on the most recent Mac OSX codenamed *Mojave*. 
The test *development space* (as Anna has suggested we nicely term it) is running the 
following particular architecture:
```
$ uname -a
Darwin Maxi-871 18.0.0 Darwin Kernel Version 18.0.0: Wed Aug 22 20:13:40 PDT 2018; root:xnu-4903.201.2~1/RELEASE_X86_64 x86_64
$ uname -m
x86_64
```
First, some prerequisites from the test machine's xcode installation: 
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
[Homebrew](https://brew.sh) installed on your machine (NOTE: The second command will 
require approximately 5-20 minutes to complete depending on the speed of your system):
```
$ brew install cairo pkg-config git coreutils
$ brew install --build-from-source gtDMMB/core/fltkwithcairo
$ brew install --build-from-source gtDMMB/core/RNAStructViz
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
That's it! Now on to building RNAStructViz from source.

## Building RNAStructViz from source

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
$ make clean && make
```
At this point if all goes well you should have a working copy of the RNAStructViz binary in the src/ directory. Let's make an alias for this command so it's easier to find and use on the system:
```
$ echo "alias RNAStructViz=\'~/RNAStructViz/src/RNAStructViz\'" >> ~/.bashrc
$ source ~/.bashrc
$ RNAStructViz
```
Alternately, you can choose to just install the application globally on 
your system by running the following command:
```
$ sudo make install
```
