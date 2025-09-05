UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
 OS = linux
else ifeq ($(UNAME_S),Darwin)
 OS = macos
else ifeq ($(OS),Windows_NT)
 OS = win
endif


src_base = \
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
source/evil.c


src_rawanalyse = \
source/lv0/crlf.c \
source/lv0/format.c


src_binanalyse = \
source/lv1/0format/conv.c \
source/lv1/assembly/cpu/arm64.c \
source/lv1/assembly/cpu/x8664.c \
source/lv1/assembly/cpu/mips64.c \
source/lv1/assembly/cpu/riscv64.c \
source/lv1/assembly/exe/elf.c \
source/lv1/assembly/exe/mac.c \
source/lv1/assembly/exe/pe.c \
source/lv1/assembly/obj/obj.c \
source/lv1/assembly/assembly.c \
source/lv1/assembly/disasm.c \
source/lv1/analyse/follow/arm64.c \
source/lv1/analyse/follow/x8664.c \
source/lv1/analyse/travel/arm64.c \
source/lv1/analyse/travel/x8664.c


src_codeanalyse = \
source/lv2/compile/compile.c \
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
source/lv2/operate/substr/substr.c


#/usr/local/opt/llvm/bin/clang
CC:=gcc
CF:=-g
DIR_INC:=-Isource/libunit -Isource
DIR_LIB:=
LIB:=-lm
SRC = $(src_base) $(src_rawanalyse) $(src_binanalyse) $(src_codeanalyse)


#web=cli
src_web_none = source/lv2/operate/serve/none.c
src_web_iocp = source/lv2/operate/serve/iocp.c
src_web_kqueue = source/lv2/operate/serve/kqueue.c
src_web_epoll = source/lv2/operate/serve/epoll.c
lib_web_win = -lgdi32 -lws2_32 -lpthread
lib_web_macos = 
lib_web_linux = 
ifdef web
	LIB+=$(lib_web_$(web)_$(OS))
	SRC += $(src_web_$(web))
else
	SRC += $(src_web_none)
endif


#ui=cli
src_ui_cli = source/lv2/operate/render/cli.c
#ui=qt
src_ui_qt = qt.o
inc_ui_qt_win=-LC:\Qt\6.0.2\mingw81_64\lib
inc_ui_qt_macos=-F/usr/local/opt/qt@6/lib
inc_ui_qt_linux=
lib_ui_qt_win=-lQt6Core -lQt6Gui -lQt6Widgets -lstdc++
lib_ui_qt_macos=-framework QtCore -framework QtGui -framework QtWidgets -lc++
lib_ui_qt_linux=-lc++
#ui=glfw
src_ui_glfw = source/lv2/operate/render/glfw.c
lib_ui_glfw_win=-lglfw3 -lglew32 -lglu32 -lopengl32 -lgdi32 -lws2_32 -lstrmiids -lpthread
lib_ui_glfw_macos=-lGLEW -lglfw -lm -framework OpenGL
lib_ui_glfw_linux=-lgdi32 -lglu32 -lws2_32 -lglfw3 -lglew32 -lopengl32 -lpthread -lm
#ui=glut
src_ui_glut = source/lv2/operate/render/glut.c
lib_ui_glut_win=-lgdi32 -lws2_32 -lpthread -lfreeglut -lglu32 -lglew32 -lopengl32 -lm
lib_ui_glut_macos=-lglut -framework OpenGL
lib_ui_glut_linux=
ifdef ui
	LIB+=$(lib_ui_$(ui)_$(OS))
	DIR_INC+=$(lib_inc_$(ui)_$(OS))
	SRC += $(src_ui_$(ui))
else
	SRC += $(src_ui_cli)
endif


#mnist=1
ifneq ($(mnist),)
CF += -DMNIST_ENABLE=1
SRC += \
source/lv3/mnist/detail/data.c \
source/lv3/mnist/detail/infer.c \
source/lv3/mnist/detail/train.c \
source/lv3/mnist/mnist.c
endif


#llama=1
ifneq ($(llama),)
CF += -DLLAMA_ENABLE=1
SRC += \
source/lv3/llama/detail/infer.c \
source/lv3/llama/detail/quanti.c \
source/lv3/llama/detail/train.c \
source/lv3/llama/llama2.c
	#backend=vulkan
	LINUXVULKAN:=/opt/vulkansdk/1.3.268.0/x86_64
	MACVULKAN:=/Users/ifbe/VulkanSDK/1.3.268.1/macOS
	WINVULKAN:=C:\VulkanSDK\1.3.268.0
	src_backend_vulkan=source/lv3/llama/backend/vulkan.c
	cf_backend_vulkan=-DBACKEND_VULKAN
	inc_backend_vulkan_win=-I${WINVULKAN}\Include -L${WINVULKAN}\Lib
	lib_backend_vulkan_win=-lvulkan-1
	inc_backend_vulkan_macos=-I$(MACVULKAN)/include -L$(MACVULKAN)/lib
	lib_backend_vulkan_macos=-lvulkan
	inc_backend_vulkan_linux=-I${LINUXVULKAN}/x86_64/include -L${LINUXVULKAN}/x86_64/lib
	lib_backend_vulkan_linux=-lvulkan
	#backend=cuda
	src_backend_cuda=cuda.dll
	cf_backend_cuda=-DBACKEND_CUDA
	#backend=remotegpu
	src_backend_remotegpu=source/lv3/llama/backend/remotegpu.c
	cf_backend_remotegpu=-DBACKEND_REMOTEGPU
	ifneq ($(backend),)
		SRC+=$(src_backend_$(backend))
		CF+=$(cf_$(backend))
		DIR_INC+=$(inc_backend_$(backend)_$(OS))
		LIB+=$(lib_backend_$(backend)_$(OS))
	else
		SRC+=$(src_backend_remotegpu)
		CF+=$(cf_backend_remotegpu)
	endif	#backend
endif	#llama


#speedup=fast
ifeq ($(speedup),fast)
 CF+=-Ofast
else ifeq ($(speedup),fastnative)
 CF+=-march=native -Ofast
else ifeq ($(speedup),nativeomp)
 CF+=-march=native -fopenmp
else ifeq ($(speedup),fastomp)
 CF+=-Ofast -fopenmp
else ifeq ($(speedup),fastnativeomp)
 CF+=-march=native -Ofast -fopenmp
endif


#build
cli:debug depend_qt depend_cuda depend_vulkan
	@echo
	@echo 'build{'
	$(CC) -o a.exe \
$(CF) \
$(SRC) \
$(DIR_INC) \
$(DIR_LIB) \
$(LIB)
	@echo '}build'
	@echo


debug:
	@echo
	@echo 'debug{'
	@echo CC=$(CC)
	@echo CF=$(CF)
	@echo DIR_INC=$(DIR_INC)
	@echo DIR_LIB=$(DIR_LIB)
	@echo LIB=$(LIB)
	@echo '}debug'
	@echo


depend_qt:
	@echo
	@echo 'depend_qt{'
ifeq ($(ui)_$(OS),qt_win)
	moc -i source/lv2/operate/render/qt.cpp -o source/lv2/operate/render/qt.moc.cpp
	g++ -std=c++17 -IC:\Qt\6.0.2\mingw81_64\include -c source/lv2/operate/render/qt.cpp -o qt.o
else ifeq ($(ui)_$(OS),qt_macos)
	moc -i source/lv2/operate/render/qt.cpp -o source/lv2/operate/render/qt.moc.cpp
	clang++ -std=c++17 -I/usr/local/opt/qt@6/include -c source/lv2/operate/render/qt.cpp -o qt.o
else ifeq ($(ui)_$(OS),qt_linux)
	g++ -std=c++17 -I/usr/local/opt/qt@6/include -c source/lv2/operate/render/qt.cpp -o qt.o
else
	@echo ui_os=$(ui)_$(OS)
endif
	@echo '}depend_qt'
	@echo


depend_cuda:
	@echo
	@echo 'depend_cuda{'
ifeq ($(backend),cuda)
	nvcc --default-stream per-thread --shared source/lv3/llama/backend/cuda.cu -o cuda.dll
else
	@echo backend=$(backend)
endif
	@echo '}depend_cuda'
	@echo


depend_vulkan:
	@echo
	@echo 'depend_vulkan{'
ifeq ($(backend)_$(OS),vulkan_win)
	${WINVULKAN}\Bin\glslc.exe source/lv3/llama/backend/shader.comp -o shader.comp.spv
else ifeq ($(backend)_$(OS),vulkan_macos)
	glslangValidator --target-env vulkan1.2 source/lv3/llama/backend/shader.comp -o shader.comp.spv
else ifeq ($(backend)_$(OS),vulkan_linux)
	${LINUXVULKAN}/x86_64/bin/glslc source/lv3/llama/backend/shader.comp -o shader.comp.spv
else
	@echo backend=$(backend)
endif
	@echo '}depend_vulkan'


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
