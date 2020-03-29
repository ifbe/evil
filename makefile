SRC = \
library/load.c \
library/rel/rel.c \
library/chip/chipdata.c \
library/chip/chipid.c \
library/chip/chiplib.c \
library/file/filedata.c \
library/file/filemd5.c \
library/file/filelib.c \
library/func/funcdata.c \
library/func/funcindex.c \
library/func/funclib.c \
library/pin/pindata.c \
library/pin/pinid.c \
library/pin/pinlib.c \
library/point/pointdata.c \
library/point/pointindex.c \
library/point/pointlib.c \
library/shape/shapedata.c \
library/shape/shapeindex.c \
library/shape/shapelib.c \
library/str/strdata.c \
library/str/strhash.c \
library/str/strlib.c \
library/extra/2d.c \
library/extra/3d.c \
library/extra/force.c \
library/extra/inout.c \
source/0robot/conv.c \
source/0robot/arm64.c \
source/0robot/x8664.c \
source/1human/learn.c \
source/1human/circuit/cir.c \
source/1human/circuit/dsn.c \
source/1human/circuit/brd.c \
source/1human/data/dts.c \
source/1human/data/utf8.c \
source/1human/map/map.c \
source/1human/model/3d.c \
source/1human/model/stl.c \
source/1human/prog/asm.c \
source/1human/prog/c.c \
source/1human/prog/cpp.c \
source/1human/prog/h.c \
source/1human/prog/hpp.c \
source/1human/test/none.c \
source/1human/test/count.c \
source/1human/test/include.c \
source/2easyop/create.c \
source/2easyop/delete.c \
source/2easyop/search.c \
source/2easyop/modify.c \
source/3indite/graph/graph.c \
source/3indite/kirchhoff/kirchhoff.c \
source/3indite/route/route.c \
source/3indite/serve/serve.c \
source/3indite/string/think.c \
source/3indite/string/substring.c \
source/evil.c

cli:
	gcc -o a.exe $(SRC) \
	source/3indite/graph/cli.c \
	source/3indite/serve/none.c \
	-Ilibrary -Isource -lm

win:
	gcc -o a.exe $(SRC) \
	source/3indite/graph/cli.c \
	source/3indite/serve/iocp.c \
	-Ilibrary -Isource -lgdi32 -lws2_32 -lpthread -lm
mac:
	gcc -o a.exe $(SRC) \
	source/3indite/graph/cli.c \
	source/3indite/serve/kqueue.c \
	-Ilibrary -Isource -lm
linux:
	gcc -o a.exe $(SRC) \
	source/3indite/graph/cli.c \
	source/3indite/serve/epoll.c \
	-Ilibrary -Isource -lm

wingl:
	gcc -o a.exe $(SRC) \
	source/3indite/graph/glut.c \
	source/3indite/serve/none.c \
	-Ilibrary -Isource -lgdi32 -lws2_32 -lpthread -lfreeglut -lglu32 -lglew32 -lopengl32 -lm
macgl:
	gcc -o a.exe $(SRC) \
	source/3indite/graph/glut.c \
	source/3indite/serve/none.c \
	-Ilibrary -Isource -lglut -lm -lGLEW -framework OpenGL
linuxgl:
	gcc -o a.exe $(SRC) \
	source/3indite/graph/glut.c \
	source/3indite/serve/none.c \
	-Ilibrary -Isource -lm

clean:
	rm -f *.exe *.out
	rm -rf .42
