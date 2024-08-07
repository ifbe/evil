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
source/parser/1assembly/disasm/disasm.c \
source/parser/1assembly/disasm/exe/elf.c \
source/parser/1assembly/disasm/exe/mac.c \
source/parser/1assembly/disasm/exe/pe.c \
source/parser/1assembly/disasm/obj/obj.c \
source/parser/1assembly/disasm/cpu/arm64.c \
source/parser/1assembly/disasm/cpu/x8664.c \
source/parser/1assembly/reverse/follow/arm64.c \
source/parser/1assembly/reverse/follow/x8664.c \
source/parser/1assembly/reverse/travel/arm64.c \
source/parser/1assembly/reverse/travel/x8664.c \
source/parser/2cfamily/compile/compile.c \
source/parser/2cfamily/compile/asm/nasm.c \
source/parser/2cfamily/compile/asm/gas.c \
source/parser/2cfamily/compile/c/c.c \
source/parser/2cfamily/learn/learn.c \
source/parser/2cfamily/learn/circuit/cir.c \
source/parser/2cfamily/learn/circuit/dsn.c \
source/parser/2cfamily/learn/circuit/brd.c \
source/parser/2cfamily/learn/data/dts.c \
source/parser/2cfamily/learn/data/utf8.c \
source/parser/2cfamily/learn/map/map.c \
source/parser/2cfamily/learn/model/3d.c \
source/parser/2cfamily/learn/model/stl.c \
source/parser/2cfamily/learn/prog/asm.c \
source/parser/2cfamily/learn/prog/c.c \
source/parser/2cfamily/learn/prog/h.c \
source/parser/2cfamily/learn/prog/cpp.c \
source/parser/2cfamily/learn/prog/hpp.c \
source/parser/2cfamily/learn/prog/java.c \
source/parser/2cfamily/learn/test/none.c \
source/parser/2cfamily/learn/test/count.c \
source/parser/2cfamily/learn/test/include.c \
source/operator/0format/conv.c \
source/operator/0format/format.c \
source/operator/1easyop/create.c \
source/operator/1easyop/delete.c \
source/operator/1easyop/search.c \
source/operator/1easyop/modify.c \
source/operator/2indite/render/render.c \
source/operator/2indite/kirchhoff/kirchhoff.c \
source/operator/2indite/route/route.c \
source/operator/2indite/serve/serve.c \
source/operator/2indite/substr/substr.c \
source/operator/3highlevel/llama/llama2.c \
source/operator/3highlevel/llama/detail/quanti.c \
source/operator/3highlevel/llama/detail/infer.c \
source/operator/3highlevel/llama/detail/train.c \
source/operator/3highlevel/mnist/mnist.c \
source/operator/3highlevel/mnist/detail/data.c \
source/operator/3highlevel/mnist/detail/infer.c \
source/operator/3highlevel/mnist/detail/train.c \
source/evil.c
OBJ:=$(patsubst %.c,%.o,$(SRC))

cli:
	$(CC) $(CF) -o a.exe $(SRC) \
	source/operator/2indite/render/cli.c \
	source/operator/2indite/serve/none.c \
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
	${WINVULKAN}\Bin\glslc.exe shader.comp -o shader.comp.spv
	$(CC) -Ofast -DBACKEND_VULKAN -o a.exe \
	$(SRC) \
	source/operator/3highlevel/llama/backend/vulkan.c \
	source/operator/2indite/render/cli.c \
	source/operator/2indite/serve/none.c \
	-I${WINVULKAN}\Include -L${WINVULKAN}\Lib -lvulkan-1 \
	-Isource/libunit -Isource -lm
cli-nativeomp-winvulkan:
	${WINVULKAN}\Bin\glslc.exe shader.comp -o shader.comp.spv
	$(CC) -march=native -Ofast -fopenmp -DBACKEND_VULKAN -o a.exe \
	$(SRC) \
	source/operator/3highlevel/llama/backend/vulkan.c \
	source/operator/2indite/render/cli.c \
	source/operator/2indite/serve/none.c \
	-I${WINVULKAN}\Include -L${WINVULKAN}\Lib -lvulkan-1 \
	-Isource/libunit -Isource -lm
cli-nativeomp-macvulkan:
	glslangValidator --target-env vulkan1.2 shader.comp -o shader.comp.spv
	$(CC) -march=native -Ofast -fopenmp -DBACKEND_VULKAN -o a.exe \
	$(SRC) \
	source/operator/3highlevel/llama/backend/vulkan.c \
	source/operator/2indite/render/cli.c \
	source/operator/2indite/serve/none.c \
	-I$(MACVULKAN)/include -L$(MACVULKAN)/lib -lvulkan \
	-Isource/libunit -Isource -lm
cli-nativeomp-linuxvulkan:
	${LINUXVULKAN}/x86_64/bin/glslc shader.comp -o shader.comp.spv
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
