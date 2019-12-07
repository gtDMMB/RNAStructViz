default:
	cd src && make -j3 -f Makefile default

help:
	cd src && make -j3 -f Makefile help

diagnostic:
	cd src && make -j3 -f Makefile diagnostic

run:
	cd src && make -j3 -f Makefile run

clean:
	cd src && make -j3 -f Makefile clean

install:
	cd src && make -j3 -f Makefile install
