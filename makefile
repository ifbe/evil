all:
	gcc \
	0.heaven/main.c \
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
	2.nadir/c.c \
	2.nadir/count.c \
	2.nadir/cpp.c \
	2.nadir/dts.c \
	2.nadir/include.c \
	2.nadir/none.c \
	2.nadir/struct.c
clean:
	rm -f *.exe *.out
	rm -f *.seed *.tree
	rm -f *.hash *.table
