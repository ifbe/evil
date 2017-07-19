all:
	gcc main.c \
	0.heaven/0.learn.c \
	0.heaven/1.check.c \
	0.heaven/1.hash.c \
	0.heaven/2.create.c \
	0.heaven/2.delete.c \
	0.heaven/2.modify.c \
	0.heaven/2.search.c \
	1.funland/hash.c \
	1.funland/string.c \
	1.funland/traverse.c \
	1.funland/worker.c \
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