default:
	cd src && make -f Makefile default

diagnostic:
	cd src && make -f Makefile diagnostic

run:
	cd src && make -f Makefile run

clean:
	cd src && make -f Makefile clean

install:
	cd src && make -f Makefile install
