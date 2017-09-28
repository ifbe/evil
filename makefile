all:
	gcc main.c \
	0.heaven/learn.c \
	0.heaven/check.c \
	1.funland/connect.c \
	1.funland/filedata.c \
	1.funland/filetrav.c \
	1.funland/funcindx.c \
	1.funland/funcdata.c \
	1.funland/stringhash.c \
	1.funland/stringdata.c \
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
