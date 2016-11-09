# @(#) mca2nbt:Makefile v1.00 (2014-05-05) / Hubert Tournier

VERSION=1.00
LDFLAGS=-lz

HTTP_CLIENT=fetch
HTTP_CLIENT_ARGS="-q"
#HTTP_CLIENT=curl
#HTTP_CLIENT_ARGS="-sO"

mca2nbt: mca2nbt.o myzpipe.o
	cc ${LDFLAGS} mca2nbt.o myzpipe.o -o mca2nbt

myzpipe.c: zpipe.c
	patch -i zpipe.patch -o myzpipe.c

zpipe.c:
	@${HTTP_CLIENT} ${HTTP_CLIENT_ARGS} http://www.zlib.net/zpipe.c

zpipe.patch:
	# just for memory...
	diff -ruN zpipe.c myzpipe.c > zpipe.patch

tarball:
	tar czf mca2nbt-${VERSION}.tar.gz License Makefile ReadMe mca2nbt.c minecraft.h zpipe.patch

install: mca2nbt
	install -s -m 0755 -o root -g wheel mca2nbt /usr/local/bin

uninstall:
	rm -f /usr/local/bin/mca2nbt

clean:
	rm -f *.o

fullclean: clean
	rm -f mca2nbt zpipe.c myzpipe.c mca2nbt-${VERSION}.tar.gz
