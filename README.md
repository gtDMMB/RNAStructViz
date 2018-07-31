# Welcome to the RNAStructViz wiki!

<img src="https://github.com/gtDMMB/RNAStructViz/blob/master/wiki-images/microscope256.png" width="200"/><img src="https://github.com/gtDMMB/RNAStructViz/blob/master/wiki-images/dna256v3.png" width="200" /><img src="https://github.com/gtDMMB/RNAStructViz/blob/master/wiki-images/dna256v5.png" width="200" /><img src="https://github.com/gtDMMB/RNAStructViz/blob/master/wiki-images/file256.png" width="200" />

## Application Description 

RNAStructViz is an open source GUI for viewing and visualizing RNA structures encoded in the [CT and FASTA](https://rna.urmc.rochester.edu/Text/File_Formats.html) file formats (see also [here](http://projects.binf.ku.dk/pgardner/bralibase/RNAformats.html) and [here](http://www.ibi.vu.nl/programs/k2nwww/static/data_formats.html) for more information on the CT file format). Examples of such files can be found in online databases including the [NCBI databases](https://www.ncbi.nlm.nih.gov/guide/dna-rna/), [here (example.ct)](https://software.broadinstitute.org/software/igv/sites/cancerinformatics.org.igv/files/example.ct), [here (RA7680.ct)](http://rna.urmc.rochester.edu/RNAstructureWeb/Examples/RA7680.ct), and in the following databases (FASTA formatted): [RefSeq](ftp://ftp.ncbi.nlm.nih.gov/refseq/release/), [BLAST](ftp://ftp.ncbi.nlm.nih.gov/blast/db/FASTA), [TraceDB](ftp://ftp.ncbi.nlm.nih.gov/pub/TraceDB), [UniGene](ftp://ftp.ncbi.nlm.nih.gov/repository/UniGene/), and [UniVec](ftp://ftp.ncbi.nlm.nih.gov/pub/UniVec/). There are also external programs for conversions between the DOT bracket and CT file formats: [ct2dot](https://rna.urmc.rochester.edu/Text/ct2dot.html) ([online interface](http://rna.urmc.rochester.edu/RNAstructureWeb/Servers/ct2dot/ct2dot.html)) and [dot2ct](https://rna.urmc.rochester.edu/Text/dot2ct.html) ([online interface](http://rna.urmc.rochester.edu/RNAstructureWeb/Servers/dot2ct/dot2ct.html)). 

## Installation on Unix-based systems (Linux, OSX, BSD)

### Building FLTK with Cairo support 

The stock binary install of modern FLTK versions will in all likelihood not have Cairo 
support enabled by default. We require for parts of the windowing and enhanced drawing 
routines used in this branch of ``RNAStructViz``. Therefore we will need to build FLTK from 
source with the Cairo configure-time options. Fortunately, this is easy enough with a 
terminal and a little know-how. Let's expand on this concept below.

#### Installing the Cairo libraries on Linux (Debian-based)

First install the Mesa/GL libs with apt if you do not already have them on your system:
```
$ sudo apt-get install libmesa-dev mesa-common-dev
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

#### Installing the Cairo libraries on a Mac 

Using [MacPorts](http://www.macports.org/) we can easily install Cairo on OS X: 
```
$ sudo port install cairo
```
See [this page](https://www.cairographics.org/download/) for more help with the 
library installation if you have any trouble with the above steps. 

#### Building FLTK from source 

Now comes the main step in this configuration process: building FLTK *with Cairo support enabled*. 
First, fetch a copy of the most recent FLTK source: 
```
$ cd ~
$ wget http://www.fltk.org/software.php?VERSION=1.3.4&FILE=fltk/1.3.4/fltk-1.3.4-2-source.tar.bz2
$ tar xvjf fltk-1.3.4-2-source.tar.bz2
$ cd fltk-1.3.4-2
```
Now we need to enable the configure-time options which will enable Cairo support by default in 
our FLTK build: 
```
$ ./configure --enable-cairo
$ make 
$ sudo make install
```
You can verify that the install was successful by verifying that the following output is sane:
```
$ which fltk-config
/usr/local/bin/fltk-config
```
That's it! Now on to building RNAStructViz from source.

### Building RNAStructViz from source

We need to fetch a local copy of the RNAStructViz source code (using the new ``with-cairo`` 
branch) so that we can build it for ourselves:
```
$ cd ~
$ git clone git@github.com:gtDMMB/RNAStructViz.git
$ cd RNAStructViz
```
Now we are ready to build the source: 
```
$ ./configure
$ make clean && make
```
At this point if all goes well you should have a working copy of the RNAStructViz binary in the src/ directory. Let's make an alias for this command so it's easier to find and use on the system:
```
$ echo "alias RNAStructViz=\'~/RNAStructViz/src/RNAStructViz\'" >> ~/.bashrc
$ source ~/.bashrc
$ RNAStructViz
```

