NPROCS = 9

.PHONY: all

all: run

libhttpserver-build: microhttpd libhttpserver-checkout
	(cd libhttpserver; make -f Makefile.cvs; mkdir build; cd build; \
	LIBMICROHTTPD_CFLAGS="-I `pwd`/../../libmicrohttpd-0.9.34/src/include/" LIBMICROHTTPD_LIBS="-L`pwd`/../../libmicrohttpd-0.9.34/src/microhttpd/.libs/" ../configure --enable-static && \
	make -j $(NPROCS))
	touch libhttpserver-build

libhttpserver-checkout:
	git clone https://github.com/etr/libhttpserver.git && (cd libhttpserver && git checkout fa5a2593ad5c6567465979548112a7e2e0c39d3e)
	touch libhttpserver-checkout

microhttpd: libmicrohttpd-0.9.34/
	(cd libmicrohttpd-0.9.34; ./configure --enable-static && make -j $(NPROCS))
	touch microhttpd

libmicrohttpd-0.9.34/: libmicrohttpd-0.9.34.tar.gz
	tar xf libmicrohttpd-0.9.34.tar.gz

libmicrohttpd-0.9.34.tar.gz:
	wget 'ftp://ftp.gnu.org/gnu/libmicrohttpd/libmicrohttpd-0.9.34.tar.gz'

highlight: source-highlight-3.1.7/
	(cd source-highlight-3.1.7; ./configure --enable-static && make -j $(NPROCS))
	touch highlight

source-highlight-3.1.7/: source-highlight-3.1.7.tar.gz
	tar xf source-highlight-3.1.7.tar.gz

source-highlight-3.1.7.tar.gz:
	wget 'ftp://ftp.gnu.org/gnu/src-highlite/source-highlight-3.1.7.tar.gz'

.PHONY: clean
clean:
	rm -Rfv source-highlight-3.1.7.tar.gz source-highlight-3.1.7/ libmicrohttpd-0.9.34/ libmicrohttpd-0.9.34.tar.gz libhttpserver/ microhttpd highlight libhttpserver-checkout libhttpserver-build
	rm -vf pastebin pastebin.db config.h

pastebin: libhttpserver-build highlight pastebin.cpp storage.cpp pastebin.hpp storage.hpp config.h
	g++ pastebin.cpp storage.cpp -I libhttpserver/src/ \
		-Isource-highlight-3.1.7/lib/ -Llibhttpserver/build/src/.libs \
		-Lsource-highlight-3.1.7/lib/srchilite/.libs \
		-Ilibmicrohttpd-0.9.34/src/include/ \
		-lsource-highlight -lhttpserver -lsqlite3 -Wall -o pastebin -ggdb -lpthread

run: pastebin
	LD_LIBRARY_PATH=libhttpserver/build/src/.libs:source-highlight-3.1.7/lib/srchilite/.libs:libmicrohttpd-0.9.34/src/microhttpd/.libs  ./pastebin

config.h:
	echo "#ifndef __CONFIG_H" > config.h
	echo "#define __CONFIG_H" >> config.h
	echo "#define DATADIR \"`pwd`/source-highlight-3.1.7/src\"" >> config.h
	echo "#define PORT 8080" >> config.h
	echo "#endif" >> config.h
