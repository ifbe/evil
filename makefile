all:
	gcc main.c \
	0.heaven/connect.c \
	0.heaven/hash.c \
	0.heaven/string.c \
	0.heaven/traverse.c \
	1.funland/0.learn.c \
	1.funland/1.check.c \
	1.funland/1.hash.c \
	1.funland/2.create.c \
	1.funland/2.delete.c \
	1.funland/2.modify.c \
	1.funland/2.search.c \
	2.nadir/foreman.c \
	2.nadir/prog/c.c \
	2.nadir/prog/cpp.c \
	2.nadir/data/dts.c \
	2.nadir/data/struct.c \
	2.nadir/test/count.c \
	2.nadir/test/include.c \
	2.nadir/test/none.c \
	-o a.exe
clean:
	rm -f *.exe *.out