all:
	gcc main.c \
	0.heaven/learn/circuit/cir.c \
	0.heaven/learn/circuit/dsn.c \
	0.heaven/learn/circuit/brd.c \
	0.heaven/learn/data/dts.c \
	0.heaven/learn/data/utf8.c \
	0.heaven/learn/model/3d.c \
	0.heaven/learn/model/stl.c \
	0.heaven/learn/prog/c.c \
	0.heaven/learn/prog/cpp.c \
	0.heaven/learn/prog/h.c \
	0.heaven/learn/prog/hpp.c \
	0.heaven/learn/test/none.c \
	0.heaven/learn/test/count.c \
	0.heaven/learn/test/include.c \
	0.heaven/learn/learn.c \
	0.heaven/think/think.c \
	0.heaven/think/kirchhoff.c \
	0.heaven/foreman.c \
	1.funland/insert/insert.c \
	1.funland/delete/delete.c \
	1.funland/modify/modify.c \
	1.funland/search/search.c \
	1.funland/filelib.c \
	1.funland/strlib.c \
	2.nadir/chipdata.c \
	2.nadir/chipid.c \
	2.nadir/filedata.c \
	2.nadir/filemd5.c \
	2.nadir/funcindex.c \
	2.nadir/funcdata.c \
	2.nadir/pindata.c \
	2.nadir/pinid.c \
	2.nadir/pointdata.c \
	2.nadir/pointindex.c \
	2.nadir/shapedata.c \
	2.nadir/shapeindex.c \
	2.nadir/strdata.c \
	2.nadir/strhash.c \
	2.nadir/wire.c \
	-o a.exe
clean:
	rm -f *.exe *.out
