SRC = \
0.learn/conv.c \
0.learn/learn.c \
0.learn/circuit/cir.c \
0.learn/circuit/dsn.c \
0.learn/circuit/brd.c \
0.learn/data/dts.c \
0.learn/data/utf8.c \
0.learn/model/3d.c \
0.learn/model/stl.c \
0.learn/prog/asm.c \
0.learn/prog/c.c \
0.learn/prog/cpp.c \
0.learn/prog/h.c \
0.learn/prog/hpp.c \
0.learn/test/none.c \
0.learn/test/count.c \
0.learn/test/include.c \
1.think/think/think.c \
1.think/think/mergechip.c \
1.think/think/substr.c \
1.think/insert/insert.c \
1.think/delete/delete.c \
1.think/modify/modify.c \
1.think/search/search.c \
2.indite/graph/graph.c \
2.indite/serve/serve.c \
2.indite/trace/trace.c \
2.indite/kirchhoff/kirchhoff.c \
3.store/load.c \
3.store/chip/chipdata.c \
3.store/chip/chipid.c \
3.store/chip/chiplib.c \
3.store/file/filedata.c \
3.store/file/filemd5.c \
3.store/file/filelib.c \
3.store/func/funcdata.c \
3.store/func/funcindex.c \
3.store/func/funclib.c \
3.store/pin/pindata.c \
3.store/pin/pinid.c \
3.store/pin/pinlib.c \
3.store/point/pointdata.c \
3.store/point/pointindex.c \
3.store/point/pointlib.c \
3.store/shape/shapedata.c \
3.store/shape/shapeindex.c \
3.store/shape/shapelib.c \
3.store/str/strdata.c \
3.store/str/strhash.c \
3.store/str/strlib.c \
3.store/rel/rel.c

all:
	gcc main.c -o a.exe \
	$(SRC) \
	2.indite/graph/cli.c \
	2.indite/serve/cli.c \
	-lm

linuxgl:
	gcc main.c -o a.exe \
	$(SRC) \
	2.indite/graph/opengl.c \
	2.indite/serve/cli.c \
	-lglut -lGLEW -lGLU -lGL -lpthread -lm
epoll:
	gcc main.c -o a.exe \
	$(SRC) \
	2.indite/graph/cli.c \
	2.indite/serve/epoll.c \
	-lm

wingl:
	gcc main.c -o a.exe \
	$(SRC) \
	2.indite/graph/opengl.c \
	2.indite/serve/cli.c \
	-lglew32 -lfreeglut -lglu32 -lopengl32 -lpthread -lm
iocp:
	gcc main.c -o a.exe \
	$(SRC) \
	2.indite/graph/cli.c \
	2.indite/serve/iocp.c \
	-lm

clean:
	rm -f *.exe *.out
