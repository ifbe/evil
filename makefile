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
source/lv1/0format/conv.c \
source/lv1/0format/format.c \
source/lv1/1assembly/analyse/follow/arm64.c \
source/lv1/1assembly/analyse/follow/x8664.c \
source/lv1/1assembly/analyse/travel/arm64.c \
source/lv1/1assembly/analyse/travel/x8664.c \
source/lv1/1assembly/assembly/cpu/arm64.c \
source/lv1/1assembly/assembly/cpu/x8664.c \
source/lv1/1assembly/assembly/exe/elf.c \
source/lv1/1assembly/assembly/exe/mac.c \
source/lv1/1assembly/assembly/exe/pe.c \
source/lv1/1assembly/assembly/obj/obj.c \
source/lv1/1assembly/assembly/assembly.c \
source/lv1/1assembly/assembly/disasm.c \
source/lv1/2cfamily/compile/asm/gas.c \
source/lv1/2cfamily/compile/asm/nasm.c \
source/lv1/2cfamily/compile/c/c.c \
source/lv1/2cfamily/compile/compile.c \
source/lv1/3human/novel/novel.c \
source/lv1/3human/poetry/poetry.c \
source/lv2/learn/circuit/brd.c \
source/lv2/learn/circuit/cir.c \
source/lv2/learn/circuit/dsn.c \
source/lv2/learn/data/dts.c \
source/lv2/learn/data/html.c \
source/lv2/learn/data/json.c \
source/lv2/learn/data/utf8.c \
source/lv2/learn/data/xaml.c \
source/lv2/learn/data/xml.c \
source/lv2/learn/learn.c \
source/lv2/learn/map/map.c \
source/lv2/learn/model/3d.c \
source/lv2/learn/model/stl.c \
source/lv2/learn/prog/asm.c \
source/lv2/learn/prog/c.c \
source/lv2/learn/prog/cpp.c \
source/lv2/learn/prog/go.c \
source/lv2/learn/prog/h.c \
source/lv2/learn/prog/hpp.c \
source/lv2/learn/prog/java.c \
source/lv2/learn/prog/js.c \
source/lv2/learn/prog/perl.c \
source/lv2/learn/prog/php.c \
source/lv2/learn/prog/python.c \
source/lv2/learn/prog/ruby.c \
source/lv2/learn/script/bash.c \
source/lv2/learn/script/bat.c \
source/lv2/learn/script/make.c \
source/lv2/learn/script/marco.c \
source/lv2/learn/test/count.c \
source/lv2/learn/test/include.c \
source/lv2/learn/test/none.c \
source/lv2/operate/easyop/create.c \
source/lv2/operate/easyop/delete.c \
source/lv2/operate/easyop/modify.c \
source/lv2/operate/easyop/search.c \
source/lv2/operate/kirchhoff/kirchhoff.c \
source/lv2/operate/render/render.c \
source/lv2/operate/route/route.c \
source/lv2/operate/serve/serve.c \
source/lv2/operate/substr/substr.c \
source/lv3/llama/backend/remotegpu.c \
source/lv3/llama/detail/infer.c \
source/lv3/llama/detail/quanti.c \
source/lv3/llama/detail/train.c \
source/lv3/llama/llama2.c \
source/lv3/mnist/detail/data.c \
source/lv3/mnist/detail/infer.c \
source/lv3/mnist/detail/train.c \
source/lv3/mnist/mnist.c \
source/evil.c
OBJ:=$(patsubst %.c,%.o,$(SRC))

cli:
	$(CC) $(CF) -o a.exe $(SRC) \
	source/lv2/operate/render/cli.c \
	source/lv2/operate/serve/none.c \
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
	nvcc --default-stream per-thread --shared source/operator/lv3/llama/backend/cuda.cu -o cuda.dll
	$(CC) -march=native -Ofast -DBACKEND_CUDA -o a.exe \
	source/operator/2indite/render/cli.c \
	source/operator/2indite/serve/none.c \
	$(SRC) cuda.dll \
	-Isource/libunit -Isource -lm

cli-fastnative-remotegpu:
	$(CC) -march=native -Ofast -fopenmp -DBACKEND_REMOTEGPU -o a.exe \
	$(SRC) \
	source/operator/lv3/llama/backend/remotegpu.c \
	source/operator/2indite/render/cli.c \
	source/operator/2indite/serve/none.c \
	-Isource/libunit -Isource -lm

cli-fast-winvulkan:
	${WINVULKAN}\Bin\glslc.exe source/operator/lv3/llama/backend/shader.comp -o shader.comp.spv
	$(CC) -Ofast -DBACKEND_VULKAN -o a.exe \
	$(SRC) \
	source/operator/lv3/llama/backend/vulkan.c \
	source/operator/2indite/render/cli.c \
	source/operator/2indite/serve/none.c \
	-I${WINVULKAN}\Include -L${WINVULKAN}\Lib -lvulkan-1 \
	-Isource/libunit -Isource -lm
cli-nativeomp-winvulkan:
	${WINVULKAN}\Bin\glslc.exe source/operator/lv3/llama/backend/shader.comp -o shader.comp.spv
	$(CC) -march=native -Ofast -fopenmp -DBACKEND_VULKAN -o a.exe \
	$(SRC) \
	source/operator/lv3/llama/backend/vulkan.c \
	source/operator/2indite/render/cli.c \
	source/operator/2indite/serve/none.c \
	-I${WINVULKAN}\Include -L${WINVULKAN}\Lib -lvulkan-1 \
	-Isource/libunit -Isource -lm
cli-nativeomp-macvulkan:
	glslangValidator --target-env vulkan1.2 source/operator/lv3/llama/backend/shader.comp -o shader.comp.spv
	$(CC) -march=native -Ofast -fopenmp -DBACKEND_VULKAN -o a.exe \
	$(SRC) \
	source/operator/lv3/llama/backend/vulkan.c \
	source/operator/2indite/render/cli.c \
	source/operator/2indite/serve/none.c \
	-I$(MACVULKAN)/include -L$(MACVULKAN)/lib -lvulkan \
	-Isource/libunit -Isource -lm
cli-nativeomp-linuxvulkan:
	${LINUXVULKAN}/x86_64/bin/glslc source/operator/lv3/llama/backend/shader.comp -o shader.comp.spv
	$(CC) -march=native -Ofast -fopenmp -DBACKEND_VULKAN -o a.exe \
	$(SRC) \
	source/operator/lv3/llama/backend/vulkan.c \
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
