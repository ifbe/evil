all:
	gcc main.c \
	0.heaven/learn/circuit/dsn.c \
	0.heaven/learn/circuit/brd.c \
	0.heaven/learn/data/dts.c \
	0.heaven/learn/data/struct.c \
	0.heaven/learn/prog/c.c \
	0.heaven/learn/prog/cpp.c \
	0.heaven/learn/test/count.c \
	0.heaven/learn/test/include.c \
	0.heaven/learn/test/none.c \
	0.heaven/learn/learn.c \
	0.heaven/think/string/name.c \
	0.heaven/think/string/utf8.c \
	0.heaven/think/think.c \
	0.heaven/foreman.c \
	1.funland/insert/insert.c \
	1.funland/delete/delete.c \
	1.funland/modify/modify.c \
	1.funland/search/search.c \
	2.nadir/connect.c \
	2.nadir/filedata.c \
	2.nadir/filemd5.c \
	2.nadir/funcindex.c \
	2.nadir/funcdata.c \
	2.nadir/strhash.c \
	2.nadir/strdata.c \
	-o a.exe
clean:
	rm -f *.exe *.out
