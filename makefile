LINUXVULKAN:=/opt/vulkansdk/1.3.268.0/x86_64
MACVULKAN:=/Users/ifbe/VulkanSDK/1.3.268.1/macOS
WINVULKAN:=C:\VulkanSDK\1.3.268.0

CC:=gcc		#/usr/local/opt/llvm/bin/clang
CF:=

SRC = \
source/libunit/load.c \
source/libunit/rel/rel.c \
source/libunit/chip/chipdata.c \
source/libunit/chip/chipid.c \
source/libunit/chip/chiplib.c \
source/libunit/file/filedata.c \
source/libunit/file/filemd5.c \
source/libunit/file/filelib.c \
source/libunit/func/funcdata.c \
source/libunit/func/funcindex.c \
source/libunit/func/funclib.c \
source/libunit/pin/pindata.c \
source/libunit/pin/pinid.c \
source/libunit/pin/pinlib.c \
source/libunit/point/pointdata.c \
source/libunit/point/pointindex.c \
source/libunit/point/pointlib.c \
source/libunit/shape/shapedata.c \
source/libunit/shape/shapeindex.c \
source/libunit/shape/shapelib.c \
source/libunit/str/strdata.c \
source/libunit/str/strhash.c \
source/libunit/str/strlib.c \
source/libextra/2d.c \
source/libextra/3d.c \
source/libextra/force.c \
source/libextra/inout.c \
source/1parser/0format/conv.c \
source/1parser/0format/format.c \
source/1parser/1assembly/analyse/follow/arm64.c \
source/1parser/1assembly/analyse/follow/x8664.c \
source/1parser/1assembly/analyse/travel/arm64.c \
source/1parser/1assembly/analyse/travel/x8664.c \
source/1parser/1assembly/assembly/cpu/arm64.c \
source/1parser/1assembly/assembly/cpu/x8664.c \
source/1parser/1assembly/assembly/exe/elf.c \
source/1parser/1assembly/assembly/exe/mac.c \
source/1parser/1assembly/assembly/exe/pe.c \
source/1parser/1assembly/assembly/obj/obj.c \
source/1parser/1assembly/assembly/assembly.c \
source/1parser/1assembly/assembly/disasm.c \
source/1parser/2cfamily/compile/asm/gas.c \
source/1parser/2cfamily/compile/asm/nasm.c \
source/1parser/2cfamily/compile/c/c.c \
source/1parser/2cfamily/compile/compile.c \
source/1parser/3human/novel/novel.c \
source/1parser/3human/poetry/poetry.c \
source/2learn/learn/circuit/brd.c \
source/2learn/learn/circuit/cir.c \
source/2learn/learn/circuit/dsn.c \
source/2learn/learn/data/dts.c \
source/2learn/learn/data/html.c \
source/2learn/learn/data/json.c \
source/2learn/learn/data/utf8.c \
source/2learn/learn/data/xaml.c \
source/2learn/learn/data/xml.c \
source/2learn/learn/learn.c \
source/2learn/learn/map/map.c \
source/2learn/learn/model/3d.c \
source/2learn/learn/model/stl.c \
source/2learn/learn/prog/asm.c \
source/2learn/learn/prog/c.c \
source/2learn/learn/prog/cpp.c \
source/2learn/learn/prog/go.c \
source/2learn/learn/prog/h.c \
source/2learn/learn/prog/hpp.c \
source/2learn/learn/prog/java.c \
source/2learn/learn/prog/js.c \
source/2learn/learn/prog/perl.c \
source/2learn/learn/prog/php.c \
source/2learn/learn/prog/python.c \
source/2learn/learn/prog/ruby.c \
source/2learn/learn/script/bash.c \
source/2learn/learn/script/bat.c \
source/2learn/learn/script/make.c \
source/2learn/learn/script/marco.c \
source/2learn/learn/test/count.c \
source/2learn/learn/test/include.c \
source/2learn/learn/test/none.c \
source/2learn/operate/easyop/create.c \
source/2learn/operate/easyop/delete.c \
source/2learn/operate/easyop/modify.c \
source/2learn/operate/easyop/search.c \
source/2learn/operate/kirchhoff/kirchhoff.c \
source/2learn/operate/render/render.c \
source/2learn/operate/route/route.c \
source/2learn/operate/serve/serve.c \
source/2learn/operate/substr/substr.c \
source/3highlevel/llama/backend/remotegpu.c \
source/3highlevel/llama/detail/infer.c \
source/3highlevel/llama/detail/quanti.c \
source/3highlevel/llama/detail/train.c \
source/3highlevel/llama/llama2.c \
source/3highlevel/mnist/detail/data.c \
source/3highlevel/mnist/detail/infer.c \
source/3highlevel/mnist/detail/train.c \
source/3highlevel/mnist/mnist.c \
source/evil.c
OBJ:=$(patsubst %.c,%.o,$(SRC))

cli:
	$(CC) $(CF) -o a.exe $(SRC) \
	source/2learn/operate/render/cli.c \
	source/2learn/operate/serve/none.c \
	-Isource/libunit -Isource -lm
cli-fast:
	make -s cli CF="-Ofast"
cli-fastnative:
	make -s cli CF="-march=native -Ofast"
cli-fastomp:
	make -s cli CF="-Ofast -fopenmp"
cli-fastnativeomp:
	make -s cli CF="-march=native -Ofast -fopenmp"

cli-fastnative-wincuda:
	nvcc --default-stream per-thread --shared source/operator/3highlevel/llama/backend/cuda.cu -o cuda.dll
	$(CC) -march=native -Ofast -DBACKEND_CUDA -o a.exe \
	source/operator/2indite/render/cli.c \
	source/operator/2indite/serve/none.c \
	$(SRC) cuda.dll \
	-Isource/libunit -Isource -lm

cli-fastnative-remotegpu:
	$(CC) -march=native -Ofast -fopenmp -DBACKEND_REMOTEGPU -o a.exe \
	$(SRC) \
	source/operator/3highlevel/llama/backend/remotegpu.c \
	source/operator/2indite/render/cli.c \
	source/operator/2indite/serve/none.c \
	-Isource/libunit -Isource -lm

cli-fast-winvulkan:
	${WINVULKAN}\Bin\glslc.exe source/operator/3highlevel/llama/backend/shader.comp -o shader.comp.spv
	$(CC) -Ofast -DBACKEND_VULKAN -o a.exe \
	$(SRC) \
	source/operator/3highlevel/llama/backend/vulkan.c \
	source/operator/2indite/render/cli.c \
	source/operator/2indite/serve/none.c \
	-I${WINVULKAN}\Include -L${WINVULKAN}\Lib -lvulkan-1 \
	-Isource/libunit -Isource -lm
cli-nativeomp-winvulkan:
	${WINVULKAN}\Bin\glslc.exe source/operator/3highlevel/llama/backend/shader.comp -o shader.comp.spv
	$(CC) -march=native -Ofast -fopenmp -DBACKEND_VULKAN -o a.exe \
	$(SRC) \
	source/operator/3highlevel/llama/backend/vulkan.c \
	source/operator/2indite/render/cli.c \
	source/operator/2indite/serve/none.c \
	-I${WINVULKAN}\Include -L${WINVULKAN}\Lib -lvulkan-1 \
	-Isource/libunit -Isource -lm
cli-nativeomp-macvulkan:
	glslangValidator --target-env vulkan1.2 source/operator/3highlevel/llama/backend/shader.comp -o shader.comp.spv
	$(CC) -march=native -Ofast -fopenmp -DBACKEND_VULKAN -o a.exe \
	$(SRC) \
	source/operator/3highlevel/llama/backend/vulkan.c \
	source/operator/2indite/render/cli.c \
	source/operator/2indite/serve/none.c \
	-I$(MACVULKAN)/include -L$(MACVULKAN)/lib -lvulkan \
	-Isource/libunit -Isource -lm
cli-nativeomp-linuxvulkan:
	${LINUXVULKAN}/x86_64/bin/glslc source/operator/3highlevel/llama/backend/shader.comp -o shader.comp.spv
	$(CC) -march=native -Ofast -fopenmp -DBACKEND_VULKAN -o a.exe \
	$(SRC) \
	source/operator/3highlevel/llama/backend/vulkan.c \
	source/operator/2indite/render/cli.c \
	source/operator/2indite/serve/none.c \
	-I${LINUXVULKAN}/x86_64/include -L${LINUXVULKAN}/x86_64/lib -lvulkan \
	-Isource/libunit -Isource -lm

win:
	gcc -o a.exe $(SRC) \
	source/operator/2indite/render/cli.c \
	source/operator/2indite/serve/iocp.c \
	-Isource/libunit -Isource -lgdi32 -lws2_32 -lpthread -lm
mac:
	gcc -o a.exe $(SRC) \
	source/operator/2indite/render/cli.c \
	source/operator/2indite/serve/kqueue.c \
	-Isource/libunit -Isource -lm
linux:
	gcc -o a.exe $(SRC) \
	source/operator/2indite/render/cli.c \
	source/operator/2indite/serve/epoll.c \
	-Isource/libunit -Isource -lm

winqt:
	moc -i source/operator/2indite/render/qt.cpp -o source/operator/2indite/render/qt.moc.cpp
	g++ -std=c++17 -IC:\Qt\6.0.2\mingw81_64\include -c source/operator/2indite/render/qt.cpp -o qt.o
	gcc -o a.exe $(SRC) \
	qt.o \
	source/operator/2indite/serve/none.c \
	-Isource/libunit -Isource \
	-LC:\Qt\6.0.2\mingw81_64\lib -lQt6Core -lQt6Gui -lQt6Widgets \
	-lm -lstdc++
macqt:
	moc -i source/operator/2indite/render/qt.cpp -o source/operator/2indite/render/qt.moc.cpp
	clang++ -std=c++17 -I/usr/local/opt/qt@6/include -c source/operator/2indite/render/qt.cpp -o qt.o
	gcc -o a.exe $(SRC) \
	qt.o \
	source/operator/2indite/serve/none.c \
	-Isource/libunit -Isource \
	-F/usr/local/opt/qt@6/lib -framework QtCore -framework QtGui -framework QtWidgets \
	-lm -lc++
linuxqt:
	clang++ -std=c++17 -I/usr/local/opt/qt@6/include -c source/operator/2indite/render/qt.cpp -o qt.o
	gcc -o a.exe $(SRC) \
	qt.o \
	source/operator/2indite/serve/none.c \
	-Isource/libunit -Isource \
	-lm -lc++


winglut:
	gcc -o a.exe $(SRC) \
	source/operator/2indite/render/glut.c \
	source/operator/2indite/serve/none.c \
	-Isource/libunit -Isource \
	-lgdi32 -lws2_32 -lpthread -lfreeglut -lglu32 -lglew32 -lopengl32 -lm
macglut:
	gcc -o a.exe $(SRC) \
	source/operator/2indite/render/glut.c \
	source/operator/2indite/serve/none.c \
	-Isource/libunit -Isource \
	-lglut -lm -lGLEW -framework OpenGL
linuxglut:
	gcc -o a.exe $(SRC) \
	source/operator/2indite/render/glut.c \
	source/operator/2indite/serve/none.c \
	-Isource/libunit -Isource \
	-lm

winglfw:
	gcc -o a.exe $(SRC) \
	source/operator/2indite/render/glfw.c \
	source/operator/2indite/serve/none.c \
	-Isource/libunit -Isource \
	-lglfw3 -lglew32 -lglu32 -lopengl32 \
	-lgdi32 -lws2_32 -lstrmiids -lpthread -lm
macglfw:
	gcc -o a.exe $(SRC) \
	source/operator/2indite/render/glfw.c \
	source/operator/2indite/serve/none.c \
	-Isource/libunit -Isource \
	-lGLEW -lglfw -lm -framework OpenGL
linuxglfw:
	gcc -o a.exe $(SRC) \
	source/operator/2indite/render/glfw.c \
	source/operator/2indite/serve/none.c \
	-Isource/libunit -Isource \
	-lgdi32 -lglu32 -lws2_32 -lglfw3 -lglew32 -lopengl32 -lpthread -lm

%.o: %.c
	if [ $(notdir $@) -nt $< ]; then \
		true;\
	else \
		echo libuser1/$<;\
		$(CC) $(CF) -c -Isource/libunit -Isource -o $(notdir $@) $<;\
	fi
clean:
	rm -f *.exe *.out
	rm -rf .42
