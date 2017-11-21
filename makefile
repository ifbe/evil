all:
	gcc main.c -o a.exe \
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
	0.heaven/think/mergechip.c \
	0.heaven/think/substr.c \
	0.heaven/think/think.c \
	0.heaven/serve/cli.c \
	0.heaven/graph/cli.c \
	0.heaven/graph/graph.c \
	1.funland/insert/insert.c \
	1.funland/delete/delete.c \
	1.funland/modify/modify.c \
	1.funland/search/search.c \
	1.funland/kirchhoff.c \
	1.funland/mechanics.c \
	2.nadir/chip/chipdata.c \
	2.nadir/chip/chipid.c \
	2.nadir/chip/chiplib.c \
	2.nadir/file/filedata.c \
	2.nadir/file/filemd5.c \
	2.nadir/file/filelib.c \
	2.nadir/func/funcdata.c \
	2.nadir/func/funcindex.c \
	2.nadir/func/funclib.c \
	2.nadir/pin/pindata.c \
	2.nadir/pin/pinid.c \
	2.nadir/pin/pinlib.c \
	2.nadir/point/pointdata.c \
	2.nadir/point/pointindex.c \
	2.nadir/point/pointlib.c \
	2.nadir/shape/shapedata.c \
	2.nadir/shape/shapeindex.c \
	2.nadir/shape/shapelib.c \
	2.nadir/str/strdata.c \
	2.nadir/str/strhash.c \
	2.nadir/str/strlib.c \
	2.nadir/rel/rel.c \
	2.nadir/load.c
gl:
	gcc main.c -o a.exe \
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
	0.heaven/think/mergechip.c \
	0.heaven/think/substr.c \
	0.heaven/think/think.c \
	0.heaven/serve/cli.c \
	0.heaven/graph/opengl.c \
	0.heaven/graph/graph.c \
	1.funland/insert/insert.c \
	1.funland/delete/delete.c \
	1.funland/modify/modify.c \
	1.funland/search/search.c \
	1.funland/kirchhoff.c \
	1.funland/mechanics.c \
	2.nadir/chip/chipdata.c \
	2.nadir/chip/chipid.c \
	2.nadir/chip/chiplib.c \
	2.nadir/file/filedata.c \
	2.nadir/file/filemd5.c \
	2.nadir/file/filelib.c \
	2.nadir/func/funcdata.c \
	2.nadir/func/funcindex.c \
	2.nadir/func/funclib.c \
	2.nadir/pin/pindata.c \
	2.nadir/pin/pinid.c \
	2.nadir/pin/pinlib.c \
	2.nadir/point/pointdata.c \
	2.nadir/point/pointindex.c \
	2.nadir/point/pointlib.c \
	2.nadir/shape/shapedata.c \
	2.nadir/shape/shapeindex.c \
	2.nadir/shape/shapelib.c \
	2.nadir/str/strdata.c \
	2.nadir/str/strhash.c \
	2.nadir/str/strlib.c \
	2.nadir/rel/rel.c \
	2.nadir/load.c \
	-lglew32 -lfreeglut -lglu32 -lopengl32 -lpthread -lm
epoll:
	gcc main.c -o a.exe \
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
	0.heaven/think/mergechip.c \
	0.heaven/think/substr.c \
	0.heaven/think/think.c \
	0.heaven/serve/epoll.c \
	0.heaven/graph/cli.c \
	0.heaven/graph/graph.c \
	1.funland/insert/insert.c \
	1.funland/delete/delete.c \
	1.funland/modify/modify.c \
	1.funland/search/search.c \
	1.funland/kirchhoff.c \
	1.funland/mechanics.c \
	2.nadir/chip/chipdata.c \
	2.nadir/chip/chipid.c \
	2.nadir/chip/chiplib.c \
	2.nadir/file/filedata.c \
	2.nadir/file/filemd5.c \
	2.nadir/file/filelib.c \
	2.nadir/func/funcdata.c \
	2.nadir/func/funcindex.c \
	2.nadir/func/funclib.c \
	2.nadir/pin/pindata.c \
	2.nadir/pin/pinid.c \
	2.nadir/pin/pinlib.c \
	2.nadir/point/pointdata.c \
	2.nadir/point/pointindex.c \
	2.nadir/point/pointlib.c \
	2.nadir/shape/shapedata.c \
	2.nadir/shape/shapeindex.c \
	2.nadir/shape/shapelib.c \
	2.nadir/str/strdata.c \
	2.nadir/str/strhash.c \
	2.nadir/str/strlib.c \
	2.nadir/rel/rel.c \
	2.nadir/load.c \
clean:
	rm -f *.exe *.out
