# Makefile

include ../make/make.config

all: cast-pv$(EXE) cast-hart$(EXE) cast-sub$(EXE) test-pv$(EXE) test-hart$(EXE) test-sub$(EXE)

.SUFFIXES: .c .o

.c$(O):
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ -c $<

cast-pv$(EXE):  cast-pv$(O) common$(O)
	$(CC) $(LDFLAGS) -o $@ cast-pv$(O) common$(O) $(PGMLIBS) $(LIBS)

cast-hart$(EXE):  cast-hart$(O) common$(O)
	$(CC) $(LDFLAGS) -o $@ cast-hart$(O) common$(O) $(PGMLIBS) $(LIBS)

cast-sub$(EXE):  cast-sub$(O) common$(O)
	$(CC) $(LDFLAGS) -o $@ cast-sub$(O) common$(O) $(PGMLIBS) $(LIBS)

test-pv$(EXE):  test-pv$(O) common$(O)
	$(CC) $(LDFLAGS) -o $@ test-pv$(O) common$(O) $(PGMLIBS) $(LIBS)

test-hart$(EXE):  test-hart$(O) common$(O)
	$(CC) $(LDFLAGS) -o $@ test-hart$(O) common$(O) $(PGMLIBS) $(LIBS)

test-sub$(EXE):  test-sub$(O) common$(O)
	$(CC) $(LDFLAGS) -o $@ test-sub$(O) common$(O) $(PGMLIBS) $(LIBS)

test: cast-pv$(EXE) cast-hart$(EXE) cast-sub$(EXE) test-pv$(EXE) test-hart$(EXE) test-sub$(EXE)
	cast-pv < ../images/lena.pgm > ../watermarked/foto-pv_lena.pgm
	cast-hart  < ../images/lena.pgm > ../watermarked/foto-hart_lena.pgm
	cast-sub  < ../images/lena.pgm > ../watermarked/foto-sub_lena.pgm

	test-pv < ../watermarked/foto-pv_lena.pgm > ../wms/foto-pv.wm
	test-hart < ../watermarked/foto-hart_lena.pgm > ../wms/foto-hart.wm
	test-sub < ../watermarked/foto-sub_lena.pgm > ../wms/foto-sub.wm

install: cast-pv$(EXE) cast-hart$(EXE) cast-sub$(EXE) test-pv$(EXE) test-hart$(EXE) test-sub$(EXE)
	$(CP) cast-pv$(EXE) cast-hart$(EXE) cast-sub$(EXE) test-pv$(EXE) test-hart$(EXE) test-sub$(EXE) $(INSTALLDIR)

clean:
	$(RM) *$(O) cast-pv$(EXE) cast-hart$(EXE) cast-sub$(EXE) test-pv$(EXE) test-hart$(EXE) test-sub$(EXE)
	$(RM) ../watermarked/* ../wms/*
