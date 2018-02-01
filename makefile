SRC = \
0learn/conv.c \
0learn/learn.c \
0learn/circuit/cir.c \
0learn/circuit/dsn.c \
0learn/circuit/brd.c \
0learn/data/dts.c \
0learn/data/utf8.c \
0learn/model/3d.c \
0learn/model/stl.c \
0learn/prog/asm.c \
0learn/prog/c.c \
0learn/prog/cpp.c \
0learn/prog/h.c \
0learn/prog/hpp.c \
0learn/test/none.c \
0learn/test/count.c \
0learn/test/include.c \
1think/think.c \
1think/stupid/insert.c \
1think/stupid/delete.c \
1think/stupid/modify.c \
1think/stupid/search.c \
1think/normal/associate.c \
1think/normal/substring.c \
1think/clever/deduction.c \
1think/clever/induction.c \
2indite/graph/force.c \
2indite/graph/graph.c \
2indite/serve/serve.c \
2indite/route/route.c \
2indite/kirchhoff/kirchhoff.c \
3store/load.c \
3store/chip/chipdata.c \
3store/chip/chipid.c \
3store/chip/chiplib.c \
3store/file/filedata.c \
3store/file/filemd5.c \
3store/file/filelib.c \
3store/func/funcdata.c \
3store/func/funcindex.c \
3store/func/funclib.c \
3store/pin/pindata.c \
3store/pin/pinid.c \
3store/pin/pinlib.c \
3store/point/pointdata.c \
3store/point/pointindex.c \
3store/point/pointlib.c \
3store/shape/shapedata.c \
3store/shape/shapeindex.c \
3store/shape/shapelib.c \
3store/str/strdata.c \
3store/str/strhash.c \
3store/str/strlib.c \
3store/rel/rel.c

cli:
	gcc main.c -o a.exe \
	$(SRC) \
	2indite/graph/cli.c \
	2indite/serve/cli.c \
	-lm

linuxxlib:
	gcc main.c -o a.exe \
	$(SRC) \
	2indite/graph/ascii.c \
	2indite/graph/xlib.c \
	2indite/serve/cli.c \
	-lX11 -lpthread -lm
linuxglut:
	gcc main.c -o a.exe \
	$(SRC) \
	2indite/graph/ascii.c \
	2indite/graph/opengl.c \
	2indite/serve/cli.c \
	-lglut -lGLEW -lGLU -lGL -lpthread -lm
epoll:
	gcc main.c -o a.exe \
	$(SRC) \
	2indite/graph/cli.c \
	2indite/serve/epoll.c \
	-lm

winapi:
	gcc main.c -o a.exe \
	$(SRC) \
	2indite/graph/ascii.c \
	2indite/graph/winapi.c \
	2indite/serve/cli.c \
	-lgdi32 -lpthread -lm
winglut:
	gcc main.c -o a.exe \
	$(SRC) \
	2indite/graph/ascii.c \
	2indite/graph/opengl.c \
	2indite/serve/cli.c \
	-lglew32 -lfreeglut -lglu32 -lopengl32 -lpthread -lm
iocp:
	gcc main.c -o a.exe \
	$(SRC) \
	2indite/graph/cli.c \
	2indite/serve/iocp.c \
	-lws2_32 -lpthread -lm

clean:
	rm -f *.exe *.out
