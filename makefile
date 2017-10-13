all:
	gcc main.c \
	0.heaven/analyse.c \
	0.heaven/check.c \
	0.heaven/learn.c \
	0.heaven/search.c \
	1.funland/connect.c \
	1.funland/filedata.c \
	1.funland/filemd5.c \
	1.funland/funcindex.c \
	1.funland/funcdata.c \
	1.funland/strhash.c \
	1.funland/strdata.c \
	2.nadir/foreman.c \
	2.nadir/circuit/dsn.c \
	2.nadir/circuit/brd.c \
	2.nadir/prog/c.c \
	2.nadir/prog/cpp.c \
	2.nadir/data/dts.c \
	2.nadir/data/struct.c \
	2.nadir/string/name.c \
	2.nadir/string/utf8.c \
	2.nadir/test/count.c \
	2.nadir/test/include.c \
	2.nadir/test/none.c \
	-o a.exe
clean:
	rm -f *.exe *.out
