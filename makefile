SRC = \
0learn/conv.c \
0learn/learn.c \
0learn/circuit/cir.c \
0learn/circuit/dsn.c \
0learn/circuit/brd.c \
0learn/data/dts.c \
0learn/data/utf8.c \
0learn/map/map.c \
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
2indite/kirchhoff/kirchhoff.c \
2indite/route/route.c \
2indite/serve/serve.c \
3library/load.c \
3library/rel/rel.c \
3library/chip/chipdata.c \
3library/chip/chipid.c \
3library/chip/chiplib.c \
3library/file/filedata.c \
3library/file/filemd5.c \
3library/file/filelib.c \
3library/func/funcdata.c \
3library/func/funcindex.c \
3library/func/funclib.c \
3library/pin/pindata.c \
3library/pin/pinid.c \
3library/pin/pinlib.c \
3library/point/pointdata.c \
3library/point/pointindex.c \
3library/point/pointlib.c \
3library/shape/shapedata.c \
3library/shape/shapeindex.c \
3library/shape/shapelib.c \
3library/str/strdata.c \
3library/str/strhash.c \
3library/str/strlib.c \
3library/extra/ascii.c \
3library/extra/inout.c

winglut:
	gcc main.c -o a.exe \
	$(SRC) \
	2indite/graph/opengl.c \
	2indite/serve/iocp.c \
	-lglew32 -lfreeglut -lglu32 -lopengl32 -lws2_32 -lpthread -lm
winapi:
	gcc main.c -o a.exe \
	$(SRC) \
	2indite/graph/winapi.c \
	2indite/serve/iocp.c \
	-lgdi32 -lws2_32 -lpthread -lm
win:
	gcc main.c -o a.exe \
	$(SRC) \
	2indite/graph/cli.c \
	2indite/serve/iocp.c \
	-lgdi32 -lws2_32 -lpthread -lm

linuxglut:
	gcc main.c -o a.exe \
	$(SRC) \
	2indite/graph/opengl.c \
	2indite/serve/epoll.c \
	-lglut -lGLEW -lGLU -lGL -lpthread -lm
linuxxlib:
	gcc main.c -o a.exe \
	$(SRC) \
	2indite/graph/xlib.c \
	2indite/serve/epoll.c \
	-lX11 -lpthread -lm
linux:
	gcc main.c -o a.exe \
	$(SRC) \
	2indite/graph/cli.c \
	2indite/serve/epoll.c \
	-lm

clean:
	rm -f *.exe *.out
