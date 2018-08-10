SRC = \
0learn/conv.c \
0learn/disasm.c \
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
2indite/graph/graph.c \
2indite/kirchhoff/kirchhoff.c \
2indite/route/route.c \
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
3library/extra/2d.c \
3library/extra/3d.c \
3library/extra/force.c \
3library/extra/inout.c \
evil.c

cli:
	gcc -o a.exe $(SRC) 2indite/graph/cli.c -I. -lm
win:
	gcc -o a.exe $(SRC) \
	2indite/graph/cli.c \
	2indite/serve/iocp.c \
	2indite/serve/serve.c \
	-I. -lgdi32 -lws2_32 -lpthread -lm
mac:
	gcc -o a.exe $(SRC) \
	2indite/graph/cli.c \
	2indite/serve/kqueue.c \
	2indite/serve/serve.c \
	-I. -lm
linux:
	gcc -o a.exe $(SRC) \
	2indite/graph/cli.c \
	2indite/serve/epoll.c \
	2indite/serve/serve.c \
	-I. -lm

clean:
	rm -f *.exe *.out
