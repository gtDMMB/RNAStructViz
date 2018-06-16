# Welcome to the RNAStructViz wiki!

<img src="https://github.com/gtDMMB/RNAStructViz/blob/master/wiki-images/microscope256.png" width="200"/><img src="https://github.com/gtDMMB/RNAStructViz/blob/master/wiki-images/dna256v3.png" width="200" /><img src="https://github.com/gtDMMB/RNAStructViz/blob/master/wiki-images/dna256v5.png" width="200" /><img src="https://github.com/gtDMMB/RNAStructViz/blob/master/wiki-images/file256.png" width="200" />

## Application Description 

RNAStructViz is an open source GUI for viewing and visualizing RNA structures encoded in the [CT and FASTA](https://rna.urmc.rochester.edu/Text/File_Formats.html) file formats (see also [here](http://projects.binf.ku.dk/pgardner/bralibase/RNAformats.html) and [here](http://www.ibi.vu.nl/programs/k2nwww/static/data_formats.html) for more information on the CT file format). Examples of such files can be found in online databases including the [NCBI databases](https://www.ncbi.nlm.nih.gov/guide/dna-rna/), [here (example.ct)](https://software.broadinstitute.org/software/igv/sites/cancerinformatics.org.igv/files/example.ct), [here (RA7680.ct)](http://rna.urmc.rochester.edu/RNAstructureWeb/Examples/RA7680.ct), and in the following databases (FASTA formatted): [RefSeq](ftp://ftp.ncbi.nlm.nih.gov/refseq/release/), [BLAST](ftp://ftp.ncbi.nlm.nih.gov/blast/db/FASTA), [TraceDB](ftp://ftp.ncbi.nlm.nih.gov/pub/TraceDB), [UniGene](ftp://ftp.ncbi.nlm.nih.gov/repository/UniGene/), and [UniVec](ftp://ftp.ncbi.nlm.nih.gov/pub/UniVec/). There are also external programs for conversions between the DOT bracket and CT file formats: [ct2dot](https://rna.urmc.rochester.edu/Text/ct2dot.html) ([online interface](http://rna.urmc.rochester.edu/RNAstructureWeb/Servers/ct2dot/ct2dot.html)) and [dot2ct](https://rna.urmc.rochester.edu/Text/dot2ct.html) ([online interface](http://rna.urmc.rochester.edu/RNAstructureWeb/Servers/dot2ct/dot2ct.html)). 

## Installation

### Default from source

Note that Linux-specific installation instructions are found [below](https://github.com/gtDMMB/RNAStructViz/wiki/_new#on-debian-based-linux-eg-ubuntu-mint-etc). First, we need to clone the GitHub repo:
```
$ cd ~
$ git clone https://github.com/gtDMMB/RNAStructViz.git
```
We can then compile the source in the usual way by running the following sequence of commands:
```
$ aclocal
$ ./configure
$ make
```
At this point if all goes well you should have a working copy of the RNAStructViz binary in the src/ directory. Let's make an alias for this command so it's easier to find and use on the system:
```
$ echo "alias RNAStructVizBinary=\'~/RNAStructViz/src/RNAStructViz\'" >> ~/.bashrc
$ source ~/.bashrc
$ RNAStructVizBinary
```

### On Debian-based Linux (e.g., Ubuntu, Mint, etc.)

First install fltk and the Mesa/GL libs with apt if you do not already have them on your system:
```
$ sudo apt-get install libmesa-dev mesa-common-dev libfltk1.1-dev
```
Next, get yourself a copy of the lastest RNAStructViz source repo on GitHub:
```
$ cd ~
$ git clone https://github.com/gtDMMB/RNAStructViz.git
$ cd RNAStructViz
```
Now we need to modify the linker flags included with the stock source to use 
the precompiled shared (.so) libraries on the system to avoid linker errors later on:
```
$ cp configure.ac configure.ac-dist
$ export FLTKLIBS=`find /usr/ -iname '*fltk*.so' | tr "\n" " " | sed 's/\/usr/-l\/usr/g'`
$ sed -e 's/\(LDFLAGS_FLTK=.*$\)/#\1\nLDFLAGS_FLTK_SUBST/' configure.ac-dist > configure.ac
$ sed -i "s|LDFLAGS_FLTK_SUBST|LDFLAGS_FLTK=\"${FLTKLIBS}/usr/lib/x86_64-linux-gnu/mesa/libGL.so.1 -lX11\"|" configure.ac
```
Next, we can compile the source in the usual way by running the following sequence of commands:
```
$ aclocal
$ ./configure
$ make
```
At this point if all goes well you should have a working copy of the RNAStructViz binary in the src/ directory. Let's make an alias for this command so it's easier to find and use on the system:
```
$ echo "alias RNAStructVizBinary=\'~/RNAStructViz/src/RNAStructViz\'" >> ~/.bashrc
$ source ~/.bashrc
$ RNAStructVizBinary
```
And we are done!

### GT Math Campus Instructions

These are installation instruction from SOM computer support.  These should work on any School of Math Red Hat machine. The instructions below are to install in your own home directory, but you can substitute a projects directory if desired.  These may be helpful for installing on your personal machine, but the dependency issues may be different, so proceed with caution. (*I have verified these instructions as of 4/27/18.  --Anna Kirkpatrick*)

#### 1) Setup a working directory, and a directory to install dependencies.
For easy of access I'm going to use ~/rna and ~/rna/deps for all
further steps. Replace as needed.
```
$ mkdir -p ~/rna/deps
```
#### 2) Navigate to the directory and grab the sources. 
We're grabbing the latest release of fltk 1.3.
```
$ cd ~/rna
$ git clone https://github.com/gtfold/RNAStructViz.git
$ wget http://fltk.org/pub/fltk/1.3.4/fltk-1.3.4-1-source.tar.gz
```

#### 3) Extract the tar file and enter the directory
```
$ tar -xf fltk-1.3.4-1-source.tar.gz
$ cd fltk-1.3.4-1
```

#### 4) Run configure with the prefix tag, make, make install. 
Note that the path needs to be an absolute path.
```
$ ./configure --prefix=nethome/$USERNAME/rna/deps
$ make
$ make install
```

#### 5) Navigate to RNAStructViz folder and modify the autoconfigure file.
```
$ cd ~/rna/RNAStructViz
```
Open the file "configure.ac" and look for the following line:
``LDFLAGS_FLTK="`fltk-config --use-gl --ldflags`"``.
Replace it with ``LDFLAGS_FLTK="`~/nethome/$USERNAME/rna/deps/bin/fltk-config --use-gl --ldstaticflags`"``.

#### 6) Set environment variable flags and run configure.
The following commands need to be run in order to let configure know
where the dependency files are located. Note that the paths need to be absolute.
```
$ export CXXFLAGS='-I/nethome/$USERNAME/rna/deps/include'
$ export LDFLAGS='-L/nethome/$USERNAME/rna/deps/lib'
$ ./configure
````

#### 7) Run make
```
make
```

#### 8) Run the program
```
$ ~/rna/RNAStructViz/src/RNAStructViz 
```
