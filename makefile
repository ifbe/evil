SRC = \
unitlib/load.c \
unitlib/rel/rel.c \
unitlib/chip/chipdata.c \
unitlib/chip/chipid.c \
unitlib/chip/chiplib.c \
unitlib/file/filedata.c \
unitlib/file/filemd5.c \
unitlib/file/filelib.c \
unitlib/func/funcdata.c \
unitlib/func/funcindex.c \
unitlib/func/funclib.c \
unitlib/pin/pindata.c \
unitlib/pin/pinid.c \
unitlib/pin/pinlib.c \
unitlib/point/pointdata.c \
unitlib/point/pointindex.c \
unitlib/point/pointlib.c \
unitlib/shape/shapedata.c \
unitlib/shape/shapeindex.c \
unitlib/shape/shapelib.c \
unitlib/str/strdata.c \
unitlib/str/strhash.c \
unitlib/str/strlib.c \
source/extra/2d.c \
source/extra/3d.c \
source/extra/force.c \
source/extra/inout.c \
source/0robot/conv.c \
source/0robot/format.c \
source/1human/disasm/disasm.c \
source/1human/disasm/exe/elf.c \
source/1human/disasm/exe/mac.c \
source/1human/disasm/exe/pe.c \
source/1human/disasm/cpu/arm64.c \
source/1human/disasm/cpu/x8664.c \
source/1human/reverse/follow/arm64.c \
source/1human/reverse/follow/x8664.c \
source/1human/reverse/travel/arm64.c \
source/1human/reverse/travel/x8664.c \
source/1human/compile/compile.c \
source/1human/compile/asm/nasm.c \
source/1human/compile/asm/gas.c \
source/1human/compile/c/c.c \
source/1human/learn/learn.c \
source/1human/learn/circuit/cir.c \
source/1human/learn/circuit/dsn.c \
source/1human/learn/circuit/brd.c \
source/1human/learn/data/dts.c \
source/1human/learn/data/utf8.c \
source/1human/learn/map/map.c \
source/1human/learn/model/3d.c \
source/1human/learn/model/stl.c \
source/1human/learn/prog/asm.c \
source/1human/learn/prog/c.c \
source/1human/learn/prog/cpp.c \
source/1human/learn/prog/h.c \
source/1human/learn/prog/hpp.c \
source/1human/learn/test/none.c \
source/1human/learn/test/count.c \
source/1human/learn/test/include.c \
source/2easyop/create.c \
source/2easyop/delete.c \
source/2easyop/search.c \
source/2easyop/modify.c \
source/3indite/render/render.c \
source/3indite/kirchhoff/kirchhoff.c \
source/3indite/route/route.c \
source/3indite/serve/serve.c \
source/3indite/substr/substr.c \
source/evil.c

cli:
	gcc -o a.exe $(SRC) \
	source/3indite/render/cli.c \
	source/3indite/serve/none.c \
	-Iunitlib -Isource -lm
win:
	gcc -o a.exe $(SRC) \
	source/3indite/render/cli.c \
	source/3indite/serve/iocp.c \
	-Iunitlib -Isource -lgdi32 -lws2_32 -lpthread -lm
mac:
	gcc -o a.exe $(SRC) \
	source/3indite/render/cli.c \
	source/3indite/serve/kqueue.c \
	-Iunitlib -Isource -lm
linux:
	gcc -o a.exe $(SRC) \
	source/3indite/render/cli.c \
	source/3indite/serve/epoll.c \
	-Iunitlib -Isource -lm

winqt:
	moc -i source/3indite/render/qt.cpp -o source/3indite/render/qt.moc.cpp
	g++ -std=c++17 -IC:\Qt\6.0.2\mingw81_64\include -c source/3indite/render/qt.cpp -o qt.o
	gcc -o a.exe $(SRC) \
	qt.o \
	source/3indite/serve/none.c \
	-Iunitlib -Isource \
	-LC:\Qt\6.0.2\mingw81_64\lib -lQt6Core -lQt6Gui -lQt6Widgets \
	-lm -lstdc++
macqt:
	moc -i source/3indite/render/qt.cpp -o source/3indite/render/qt.moc.cpp
	clang++ -std=c++17 -I/usr/local/opt/qt@6/include -c source/3indite/render/qt.cpp -o qt.o
	gcc -o a.exe $(SRC) \
	qt.o \
	source/3indite/serve/none.c \
	-Iunitlib -Isource \
	-F/usr/local/opt/qt@6/lib -framework QtCore -framework QtGui -framework QtWidgets \
	-lm -lc++
linuxqt:
	clang++ -std=c++17 -I/usr/local/opt/qt@6/include -c source/3indite/render/qt.cpp -o qt.o
	gcc -o a.exe $(SRC) \
	qt.o \
	source/3indite/serve/none.c \
	-Iunitlib -Isource \
	-lm -lc++


winglut:
	gcc -o a.exe $(SRC) \
	source/3indite/render/glut.c \
	source/3indite/serve/none.c \
	-Iunitlib -Isource \
	-lgdi32 -lws2_32 -lpthread -lfreeglut -lglu32 -lglew32 -lopengl32 -lm
macglut:
	gcc -o a.exe $(SRC) \
	source/3indite/render/glut.c \
	source/3indite/serve/none.c \
	-Iunitlib -Isource \
	-lglut -lm -lGLEW -framework OpenGL
linuxglut:
	gcc -o a.exe $(SRC) \
	source/3indite/render/glut.c \
	source/3indite/serve/none.c \
	-Iunitlib -Isource \
	-lm

winglfw:
	gcc -o a.exe $(SRC) \
	source/3indite/render/glfw.c \
	source/3indite/serve/none.c \
	-Iunitlib -Isource \
	-lglfw3 -lglew32 -lglu32 -lopengl32 \
	-lgdi32 -lws2_32 -lstrmiids -lpthread -lm
macglfw:
	gcc -o a.exe $(SRC) \
	source/3indite/render/glfw.c \
	source/3indite/serve/none.c \
	-Iunitlib -Isource \
	-lGLEW -lglfw -lm -framework OpenGL
linuxglfw:
	gcc -o a.exe $(SRC) \
	source/3indite/render/glfw.c \
	source/3indite/serve/none.c \
	-Iunitlib -Isource \
	-lgdi32 -lglu32 -lws2_32 -lglfw3 -lglew32 -lopengl32 -lpthread -lm

clean:
	rm -f *.exe *.out
	rm -rf .42
