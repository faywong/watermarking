# Makefile

# chose build plattform
include ../make/make.config

OPTIONS=-DPARAM_STUFF -DPOLLEN_STUFF

all: 	tools \
	bruyn \
	koch \
	corvi \
	xia \
	xie \
	xie2 \
	cox \
	zhu \
	dugad \
	wang \
	frid2 \
	kim \
	kund2 \
	kund3

.SUFFIXES: .c .o .1 .ps

.c$(O):
	$(CC) $(CFLAGS) $(INCLUDES) $(OPTIONS) -o $@ -c $<

.1.ps:
	$(GROFF) $< > $@ 

# library containing general stuff

$(LIBPREFIX)wm$(LIB): dct$(O) coeff$(O) gray$(O) sort$(O) signature$(O) coord$(O) wm$(O)
	$(RM) $@
	ar -rc $@ dct$(O) coeff$(O) gray$(O) sort$(O) signature$(O) coord$(O) wm$(O)

libraryclean:
	$(RM) $(LIBPREFIX)wm$(LIB)

# library containing wavelet transform stuff

$(LIBPREFIX)wavelet$(LIB): wavelet$(O) dwt$(O) dwt_util$(O)
	$(RM) $@
	ar -rc $@ wavelet$(O) dwt$(O) dwt_util$(O)

waveletclean:
	$(RM) $(LIBPREFIX)wavelet$(LIB)

# some general tools to compute difference image, PSNR, ...

tools: cmp_pgm$(EXE) cmp_dct8x8$(EXE) cmp_dct$(EXE) cmp_dwt$(EXE) cmp_ppm$(EXE)

toolstest:

toolsinstall: tools
	$(CP) cmp_ppm$(EXE) cmp_pgm$(EXE) cmp_dct8x8$(EXE) cmp_dct$(EXE) cmp_dwt$(EXE) $(INSTALLDIR)

toolsman: cmp_ppm.ps cmp_pgm.ps cmp_dct8x8.ps cmp_dct.ps

toolsclean:
	$(RM) cmp_ppm$(EXE) cmp_pgm$(EXE) cmp_dct$(EXE) cmp_dct8x8$(EXE) cmp_dwt$(EXE)

cmp_pgm$(EXE):  cmp_pgm$(O) $(LIBPREFIX)wm$(LIB)
	$(CC) $(LDFLAGS) -o $@ cmp_pgm$(O) $(WMLIB) $(PGMLIBS) $(LIBS)

cmp_ppm$(EXE):  cmp_ppm$(O) $(LIBPREFIX)wm$(LIB)
	$(CC) $(LDFLAGS) -o $@ cmp_ppm$(O) $(WMLIB) $(PGMLIBS) $(LIBS)

cmp_dct$(EXE):  cmp_dct$(O) $(LIBPREFIX)wm$(LIB)
	$(CC) $(LDFLAGS) -o $@ cmp_dct$(O) $(WMLIB) $(PGMLIBS)

cmp_dwt$(EXE):  cmp_dwt$(O) $(LIBPREFIX)wavelet$(LIB)  $(LIBPREFIX)wm$(LIB)
	$(CC) $(LDFLAGS) -o $@ cmp_dwt$(O) $(WAVELIB) $(WMLIB) $(PGMLIBS)

cmp_dct8x8$(EXE): cmp_dct8x8$(O) $(LIBPREFIX)wm$(LIB)
	$(CC) $(LDFLAGS) -o $@ cmp_dct8x8$(O) $(WMLIB) $(PGMLIBS)


# Koch's algorithm (8x8 block DCT, blind, binary)

koch: gen_koch_sig$(EXE) wm_koch_e$(EXE) wm_koch_d$(EXE) cmp_koch_sig$(EXE)

kochtest: koch
	gen_koch_sig$(EXE) -n 150 < gen_koch_sig.c > ../sigs/koch.sig
	wm_koch_e$(EXE) -s ../sigs/koch.sig -o ../watermarked/koch_lena.pgm ../images/lena.pgm
	wm_koch_d$(EXE) -s ../sigs/koch.sig -o ../wms/koch.wm ../watermarked/koch_lena.pgm
	cmp_koch_sig$(EXE) -s ../sigs/koch.sig ../wms/koch.wm

kochinstall: koch
	$(CP) gen_koch_sig$(EXE) wm_koch_e$(EXE) wm_koch_d$(EXE) cmp_koch_sig$(EXE) $(INSTALLDIR)

kochclean:
	$(RM) gen_koch_sig$(EXE) wm_koch_e$(EXE) wm_koch_d$(EXE) cmp_koch_sig$(EXE)

kochman:

wm_koch_e$(EXE): wm_koch_e$(O) $(LIBPREFIX)wm$(LIB)
	$(CC) $(LDFLAGS) -o $@ wm_koch_e$(O) $(WMLIB) $(LIBS) $(PGMLIBS)

wm_koch_d$(EXE): wm_koch_d$(O) $(LIBPREFIX)wm$(LIB)
	$(CC) $(LDFLAGS) -o $@  wm_koch_d$(O) $(WMLIB) $(LIBS) $(PGMLIBS)

gen_koch_sig$(EXE): gen_koch_sig$(O) wm$(O)
	$(CC) $(LDFLAGS) -o $@ gen_koch_sig$(O) wm$(O) $(LIBS)

cmp_koch_sig$(EXE): cmp_koch_sig$(O) $(LIBPREFIX)wm$(LIB)
	$(CC) $(LDFLAGS) -o $@ cmp_koch_sig$(O) $(WMLIB) $(LIBS)

# Fridrich's 2. scheme (full-frame DCT, blind, binary)

frid2: gen_frid2_sig$(EXE) wm_frid2_e$(EXE) wm_frid2_d$(EXE) cmp_frid2_sig$(EXE)

frid2test: frid2
	gen_frid2_sig$(EXE) -n 250 < gen_frid2_sig.c > ../sigs/frid2.sig
	wm_frid2_e$(EXE) -s ../sigs/frid2.sig -o ../watermarked/frid2_lena.pgm ../images/lena.pgm
	wm_frid2_d$(EXE) -s ../sigs/frid2.sig -o ../wms/frid2.wm ../watermarked/frid2_lena.pgm
	cmp_frid2_sig$(EXE) -s ../sigs/frid2.sig ../wms/frid2.wm

frid2install: frid2
	$(CP) gen_frid2_sig$(EXE) wm_frid2_e$(EXE) wm_frid2_d$(EXE) cmp_frid2_sig$(EXE) $(INSTALLDIR)

frid2clean:
	$(RM) gen_frid2_sig$(EXE) wm_frid2_e$(EXE) wm_frid2_d$(EXE) cmp_frid2_sig$(EXE)

frid2man:

wm_frid2_e$(EXE): wm_frid2_e$(O) $(LIBPREFIX)wm$(LIB) frid2_common$(O)
	$(CC) $(LDFLAGS) -o $@ wm_frid2_e$(O) frid2_common$(O) $(WMLIB) $(LIBS) $(PGMLIBS)

wm_frid2_d$(EXE): wm_frid2_d$(O) $(LIBPREFIX)wm$(LIB) frid2_common$(O)
	$(CC) $(LDFLAGS) -o $@  wm_frid2_d$(O) frid2_common$(O) $(WMLIB) $(LIBS) $(PGMLIBS)

gen_frid2_sig$(EXE): gen_frid2_sig$(O) wm$(O)
	$(CC) $(LDFLAGS) -o $@ gen_frid2_sig$(O) wm$(O) $(LIBS)

cmp_frid2_sig$(EXE): cmp_frid2_sig$(O) $(LIBPREFIX)wm$(LIB)
	$(CC) $(LDFLAGS) -o $@ cmp_frid2_sig$(O) $(WMLIB) $(LIBS)


# Bruyndonckx's algorithm (spatial domain, block classification, blind)

bruyn: gen_bruyn_sig$(EXE) wm_bruyn_e$(EXE) wm_bruyn_d$(EXE) cmp_bruyn_sig$(EXE)

bruyntest: bruyn
	gen_bruyn_sig$(EXE) -n 400 < gen_bruyn_sig.c > ../sigs/bruyn.sig
	wm_bruyn_e$(EXE) -s ../sigs/bruyn.sig -o ../watermarked/bruyn_lena.pgm ../images/lena.pgm
	wm_bruyn_d$(EXE) -s ../sigs/bruyn.sig -o ../wms/bruyn.wm ../watermarked/bruyn_lena.pgm
	cmp_bruyn_sig$(EXE) -s ../sigs/bruyn.sig ../wms/bruyn.wm

bruyninstall: bruyn
	$(CP) gen_bruyn_sig$(EXE) wm_bruyn_e$(EXE) wm_bruyn_d$(EXE) cmp_bruyn_sig$(EXE) $(INSTALLDIR)

bruynclean:
	$(RM) gen_bruyn_sig$(EXE) wm_bruyn_e$(EXE) wm_bruyn_d$(EXE) cmp_bruyn_sig$(EXE)

bruynman:

wm_bruyn_e$(EXE): wm_bruyn_e$(O) bruyn_common$(O) $(LIBPREFIX)wm$(LIB)
	$(CC) $(LDFLAGS) -o $@ wm_bruyn_e$(O) bruyn_common$(O) $(WMLIB) $(LIBS) $(PGMLIBS)

wm_bruyn_d$(EXE): wm_bruyn_d$(O)  bruyn_common$(O) $(LIBPREFIX)wm$(LIB)
	$(CC) $(LDFLAGS) -o $@  wm_bruyn_d$(O) bruyn_common$(O) $(WMLIB) $(LIBS) $(PGMLIBS)

gen_bruyn_sig$(EXE): gen_bruyn_sig$(O) wm$(O)
	$(CC) $(LDFLAGS) -o $@ gen_bruyn_sig$(O) wm$(O) $(LIBS)

cmp_bruyn_sig$(EXE): cmp_bruyn_sig$(O) $(LIBPREFIX)wm$(LIB)
	$(CC) $(LDFLAGS) -o $@ cmp_bruyn_sig$(O) $(WMLIB) $(LIBS)


# Cox's algorithm (full-frame DCT, non-blind, spread-spectrum)

cox: gen_cox_sig$(EXE) wm_cox_e$(EXE) wm_cox_d$(EXE) cmp_cox_sig$(EXE)

coxtest: cox
	gen_cox_sig$(EXE) > ../sigs/cox.sig
	wm_cox_e$(EXE) -s ../sigs/cox.sig -o ../watermarked/cox_lena.pgm ../images/lena.pgm
	wm_cox_d$(EXE) -s ../sigs/cox.sig -o ../wms/cox.wm -i ../images/lena.pgm ../watermarked/cox_lena.pgm 
	cmp_cox_sig$(EXE) -s ../sigs/cox.sig ../wms/cox.wm

coxinstall: cox
	$(CP) gen_cox_sig$(EXE) wm_cox_e$(EXE) wm_cox_d$(EXE) cmp_cox_sig$(EXE) $(INSTALLDIR)

coxman: gen_cox_sig.ps wm_cox_e.ps wm_cox_d.ps

wm_cox_e$(EXE): wm_cox_e$(O) $(LIBPREFIX)wm$(LIB)
	$(CC) $(LDFLAGS) -o $@ wm_cox_e$(O) $(WMLIB) $(LIBS) $(PGMLIBS)

wm_cox_d$(EXE): wm_cox_d$(O) $(LIBPREFIX)wm$(LIB)
	$(CC) $(LDFLAGS) -o $@  wm_cox_d$(O) $(WMLIB) $(LIBS) $(PGMLIBS)

gen_cox_sig$(EXE): gen_cox_sig$(O)
	$(CC) $(LDFLAGS) -o $@ gen_cox_sig$(O) $(LIBS)

cmp_cox_sig$(EXE): cmp_cox_sig$(O)
	$(CC) $(LDFLAGS) -o $@ cmp_cox_sig$(O) $(LIBS)

coxclean:
	$(RM) gen_cox_sig$(EXE) wm_cox_e$(EXE) wm_cox_d$(EXE) cmp_cox_sig$(EXE)


# Corvi's algorithm (DWT, non-blind, spread-spectrum, approximation image)

corvi: gen_corvi_sig$(EXE) wm_corvi_e$(EXE) wm_corvi_d$(EXE) cmp_corvi_sig$(EXE)

corvitest: corvi
	gen_corvi_sig$(EXE) > ../sigs/corvi.sig
	wm_corvi_e$(EXE) -s ../sigs/corvi.sig -o ../watermarked/corvi_lena.pgm ../images/lena.pgm
	wm_corvi_d$(EXE) -s ../sigs/corvi.sig -o ../wms/corvi.wm -i ../images/lena.pgm ../watermarked/corvi_lena.pgm 
	cmp_corvi_sig$(EXE) -s ../sigs/corvi.sig ../wms/corvi.wm

corviinstall: corvi
	$(CP) gen_corvi_sig$(EXE) wm_corvi_e$(EXE) wm_corvi_d$(EXE) cmp_corvi_sig$(EXE) $(INSTALLDIR)

corviman: gen_corvi_sig.ps wm_corvi_e.ps wm_corvi_d.ps

wm_corvi_e$(EXE): wm_corvi_e$(O) $(LIBPREFIX)wm$(LIB) $(LIBPREFIX)wavelet$(LIB)
	$(CC) $(LDFLAGS) -o $@ wm_corvi_e$(O) $(WMLIB) $(WAVELIB) $(LIBS) $(PGMLIBS)

wm_corvi_d$(EXE): wm_corvi_d$(O) $(LIBPREFIX)wm$(LIB) $(LIBPREFIX)wavelet$(LIB)
	$(CC) $(LDFLAGS) -o $@  wm_corvi_d$(O) $(WMLIB) $(WAVELIB) $(LIBS) $(PGMLIBS)

gen_corvi_sig$(EXE): gen_corvi_sig$(O)
	$(CC) $(LDFLAGS) -o $@ gen_corvi_sig$(O) $(LIBS)

cmp_corvi_sig$(EXE): cmp_corvi_sig$(O)
	$(CC) $(LDFLAGS) -o $@ cmp_corvi_sig$(O) $(LIBS)

corviclean:
	$(RM) gen_corvi_sig$(EXE) wm_corvi_e$(EXE) wm_corvi_d$(EXE) cmp_corvi_sig$(EXE)


# Xia's algorithm (DWT, non-blind, spread-spectrum, detail subbands)

xia: gen_xia_sig$(EXE) wm_xia_e$(EXE) wm_xia_d$(EXE) cmp_xia_sig$(EXE)

xiatest: xia
	gen_xia_sig$(EXE) > ../sigs/xia.sig
	wm_xia_e$(EXE) -s ../sigs/xia.sig -o ../watermarked/xia_lena.pgm ../images/lena.pgm
	wm_xia_d$(EXE) -s ../sigs/xia.sig -o ../wms/xia.wm -i ../images/lena.pgm ../watermarked/xia_lena.pgm 
	cmp_xia_sig$(EXE) -s ../sigs/xia.sig ../wms/xia.wm

xiainstall: xia
	$(CP) gen_xia_sig$(EXE) wm_xia_e$(EXE) wm_xia_d$(EXE) cmp_xia_sig$(EXE) $(INSTALLDIR)

xiaman: gen_xia_sig.ps wm_xia_e.ps wm_xia_d.ps

wm_xia_e$(EXE): wm_xia_e$(O) $(LIBPREFIX)wm$(LIB) $(LIBPREFIX)wavelet$(LIB)
	$(CC) $(LDFLAGS) -o $@ wm_xia_e$(O) $(WMLIB) $(WAVELIB) $(LIBS) $(PGMLIBS)

wm_xia_d$(EXE): wm_xia_d$(O) $(LIBPREFIX)wm$(LIB) $(LIBPREFIX)wavelet$(LIB)
	$(CC) $(LDFLAGS) -o $@  wm_xia_d$(O) $(WMLIB) $(WAVELIB) $(LIBS) $(PGMLIBS)

gen_xia_sig$(EXE): gen_xia_sig$(O)
	$(CC) $(LDFLAGS) -o $@ gen_xia_sig$(O) $(LIBS)

cmp_xia_sig$(EXE): cmp_xia_sig$(O)
	$(CC) $(LDFLAGS) -o $@ cmp_xia_sig$(O) $(LIBS)

xiaclean:
	$(RM) gen_xia_sig$(EXE) wm_xia_e$(EXE) wm_xia_d$(EXE) cmp_xia_sig$(EXE)

# Wang's algorithm (DWT, non-blind, spread-spectrum, detail subbands)

wang: gen_wang_sig$(EXE) wm_wang_e$(EXE) wm_wang_d$(EXE) cmp_wang_sig$(EXE)

wangtest: wang
	gen_wang_sig$(EXE) -n 1000 > ../sigs/wang.sig
	wm_wang_e$(EXE) -s ../sigs/wang.sig -o ../watermarked/wang_lena.pgm ../images/lena.pgm
	wm_wang_d$(EXE) -s ../sigs/wang.sig -o ../wms/wang.wm -i ../images/lena.pgm ../watermarked/wang_lena.pgm 
	cmp_wang_sig$(EXE) -s ../sigs/wang.sig ../wms/wang.wm

wanginstall: wang
	$(CP) gen_wang_sig$(EXE) wm_wang_e$(EXE) wm_wang_d$(EXE) cmp_wang_sig$(EXE) $(INSTALLDIR)

wangman: gen_wang_sig.ps wm_wang_e.ps wm_wang_d.ps

wm_wang_e$(EXE): wm_wang_e$(O) $(LIBPREFIX)wm$(LIB) $(LIBPREFIX)wavelet$(LIB) wang_common$(O)
	$(CC) $(LDFLAGS) -o $@ wm_wang_e$(O) wang_common$(O) $(WMLIB) $(WAVELIB) $(LIBS) $(PGMLIBS)

wm_wang_d$(EXE): wm_wang_d$(O) $(LIBPREFIX)wm$(LIB) $(LIBPREFIX)wavelet$(LIB) wang_common$(O)
	$(CC) $(LDFLAGS) -o $@  wm_wang_d$(O) wang_common$(O) $(WMLIB) $(WAVELIB) $(LIBS) $(PGMLIBS)

gen_wang_sig$(EXE): gen_wang_sig$(O)
	$(CC) $(LDFLAGS) -o $@ gen_wang_sig$(O) $(LIBS)

cmp_wang_sig$(EXE): cmp_wang_sig$(O)
	$(CC) $(LDFLAGS) -o $@ cmp_wang_sig$(O) $(LIBS)

wangclean:
	$(RM) gen_wang_sig$(EXE) wm_wang_e$(EXE) wm_wang_d$(EXE) cmp_wang_sig$(EXE)

# Kim's algorithm (DWT, non-blind, spread-spectrum, approx. & detail subbands)

kim: gen_kim_sig$(EXE) wm_kim_e$(EXE) wm_kim_d$(EXE) cmp_kim_sig$(EXE)

kimtest: kim
	gen_kim_sig$(EXE) -n 1000 > ../sigs/kim.sig
	wm_kim_e$(EXE) -s ../sigs/kim.sig -o ../watermarked/kim_lena.pgm ../images/lena.pgm
	wm_kim_d$(EXE) -s ../sigs/kim.sig -o ../wms/kim.wm -i ../images/lena.pgm ../watermarked/kim_lena.pgm 
	cmp_kim_sig$(EXE) -s ../sigs/kim.sig ../wms/kim.wm

kiminstall: kim
	$(CP) gen_kim_sig$(EXE) wm_kim_e$(EXE) wm_kim_d$(EXE) cmp_kim_sig$(EXE) $(INSTALLDIR)

kimman: gen_kim_sig.ps wm_kim_e.ps wm_kim_d.ps

wm_kim_e$(EXE): wm_kim_e$(O) $(LIBPREFIX)wm$(LIB) $(LIBPREFIX)wavelet$(LIB) kim_common$(O)
	$(CC) $(LDFLAGS) -o $@ wm_kim_e$(O) kim_common$(O) $(WMLIB) $(WAVELIB) $(LIBS) $(PGMLIBS)

wm_kim_d$(EXE): wm_kim_d$(O) $(LIBPREFIX)wm$(LIB) $(LIBPREFIX)wavelet$(LIB) kim_common$(O)
	$(CC) $(LDFLAGS) -o $@  wm_kim_d$(O) kim_common$(O) $(WMLIB) $(WAVELIB) $(LIBS) $(PGMLIBS)

gen_kim_sig$(EXE): gen_kim_sig$(O)
	$(CC) $(LDFLAGS) -o $@ gen_kim_sig$(O) $(LIBS)

cmp_kim_sig$(EXE): cmp_kim_sig$(O)
	$(CC) $(LDFLAGS) -o $@ cmp_kim_sig$(O) $(LIBS)

kimclean:
	$(RM) gen_kim_sig$(EXE) wm_kim_e$(EXE) wm_kim_d$(EXE) cmp_kim_sig$(EXE)

# Zhu's algorithm (DWT, non-blind, spread-spectrum, detail subbands)

zhu: gen_zhu_sig$(EXE) wm_zhu_e$(EXE) wm_zhu_d$(EXE) cmp_zhu_sig$(EXE)

zhutest: zhu
	gen_zhu_sig$(EXE) > ../sigs/zhu.sig
	wm_zhu_e$(EXE) -s ../sigs/zhu.sig -o ../watermarked/zhu_lena.pgm ../images/lena.pgm
	wm_zhu_d$(EXE) -s ../sigs/zhu.sig -o ../wms/zhu.wm -i ../images/lena.pgm ../watermarked/zhu_lena.pgm 
	cmp_zhu_sig$(EXE) -s ../sigs/zhu.sig ../wms/zhu.wm

zhuinstall: zhu
	$(CP) gen_zhu_sig$(EXE) wm_zhu_e$(EXE) wm_zhu_d$(EXE) cmp_zhu_sig$(EXE) $(INSTALLDIR)

zhuman: gen_zhu_sig.ps wm_zhu_e.ps wm_zhu_d.ps

wm_zhu_e$(EXE): wm_zhu_e$(O) $(LIBPREFIX)wm$(LIB) $(LIBPREFIX)wavelet$(LIB)
	$(CC) $(LDFLAGS) -o $@ wm_zhu_e$(O) $(WMLIB) $(WAVELIB) $(LIBS) $(PGMLIBS)

wm_zhu_d$(EXE): wm_zhu_d$(O) $(LIBPREFIX)wm$(LIB) $(LIBPREFIX)wavelet$(LIB)
	$(CC) $(LDFLAGS) -o $@  wm_zhu_d$(O) $(WMLIB) $(WAVELIB) $(LIBS) $(PGMLIBS)

gen_zhu_sig$(EXE): gen_zhu_sig$(O)
	$(CC) $(LDFLAGS) -o $@ gen_zhu_sig$(O) $(LIBS)

cmp_zhu_sig$(EXE): cmp_zhu_sig$(O)
	$(CC) $(LDFLAGS) -o $@ cmp_zhu_sig$(O) $(LIBS)

zhuclean:
	$(RM) gen_zhu_sig$(EXE) wm_zhu_e$(EXE) wm_zhu_d$(EXE) cmp_zhu_sig$(EXE)

# Xie's algorithm (DWT, blind, binary, quantization, approximation image)

xie: gen_xie_sig$(EXE) wm_xie_e$(EXE) wm_xie_d$(EXE) cmp_xie_sig$(EXE)

xietest: xie
	gen_xie_sig$(EXE) -n 800 gen_xie_sig.c > ../sigs/xie.sig
	wm_xie_e$(EXE) -s ../sigs/xie.sig -o ../watermarked/xie_lena.pgm ../images/lena.pgm
	wm_xie_d$(EXE) -s ../sigs/xie.sig -o ../wms/xie.wm ../watermarked/xie_lena.pgm 
	cmp_xie_sig$(EXE) -s ../sigs/xie.sig ../wms/xie.wm

xieinstall: xie
	$(CP) gen_xie_sig$(EXE) wm_xie_e$(EXE) wm_xie_d$(EXE) cmp_xie_sig$(EXE) $(INSTALLDIR)

xieman: gen_xie_sig.ps wm_xie_e.ps wm_xie_d.ps

wm_xie_e$(EXE): wm_xie_e$(O) $(LIBPREFIX)wm$(LIB) $(LIBPREFIX)wavelet$(LIB)
	$(CC) $(LDFLAGS) -o $@ wm_xie_e$(O) $(WMLIB) $(WAVELIB) $(LIBS) $(PGMLIBS)

wm_xie_d$(EXE): wm_xie_d$(O) $(LIBPREFIX)wm$(LIB) $(LIBPREFIX)wavelet$(LIB)
	$(CC) $(LDFLAGS) -o $@  wm_xie_d$(O) $(WMLIB) $(WAVELIB) $(LIBS) $(PGMLIBS)

gen_xie_sig$(EXE): gen_xie_sig$(O) wm$(O)
	$(CC) $(LDFLAGS) -o $@ gen_xie_sig$(O) wm$(O) $(LIBS)

cmp_xie_sig$(EXE): cmp_xie_sig$(O) $(LIBPREFIX)wm$(LIB)
	$(CC) $(LDFLAGS) -o $@ cmp_xie_sig$(O) $(WMLIB) $(LIBS)

xieclean:
	$(RM) gen_xie_sig$(EXE) wm_xie_e$(EXE) wm_xie_d$(EXE) cmp_xie_sig$(EXE)

# Xie2's algorithm (DWT, blind, binary, quantization, approximation image)

xie2: gen_xie2_sig$(EXE) wm_xie2_e$(EXE) wm_xie2_d$(EXE) cmp_xie2_sig$(EXE)

xie2test: xie2
	gen_xie2_sig$(EXE) -n 800 gen_xie2_sig.c > ../sigs/xie2.sig
	wm_xie2_e$(EXE) -s ../sigs/xie2.sig -o ../watermarked/xie2_lena.pgm ../images/lena.pgm
	wm_xie2_d$(EXE) -s ../sigs/xie2.sig -o ../wms/xie2.wm ../watermarked/xie2_lena.pgm 
	cmp_xie2_sig$(EXE) -s ../sigs/xie2.sig ../wms/xie2.wm

xie2install: xie2
	$(CP) gen_xie2_sig$(EXE) wm_xie2_e$(EXE) wm_xie2_d$(EXE) cmp_xie2_sig$(EXE) $(INSTALLDIR)

xie2man: gen_xie2_sig.ps wm_xie2_e.ps wm_xie2_d.ps

wm_xie2_e$(EXE): wm_xie2_e$(O) $(LIBPREFIX)wm$(LIB) $(LIBPREFIX)wavelet$(LIB)
	$(CC) $(LDFLAGS) -o $@ wm_xie2_e$(O) $(WMLIB) $(WAVELIB) $(LIBS) $(PGMLIBS)

wm_xie2_d$(EXE): wm_xie2_d$(O) $(LIBPREFIX)wm$(LIB) $(LIBPREFIX)wavelet$(LIB)
	$(CC) $(LDFLAGS) -o $@  wm_xie2_d$(O) $(WMLIB) $(WAVELIB) $(LIBS) $(PGMLIBS)

gen_xie2_sig$(EXE): gen_xie2_sig$(O) wm$(O)
	$(CC) $(LDFLAGS) -o $@ gen_xie2_sig$(O) wm$(O) $(LIBS)

cmp_xie2_sig$(EXE): cmp_xie2_sig$(O) $(LIBPREFIX)wm$(LIB)
	$(CC) $(LDFLAGS) -o $@ cmp_xie2_sig$(O) $(WMLIB) $(LIBS)

xie2clean:
	$(RM) gen_xie2_sig$(EXE) wm_xie2_e$(EXE) wm_xie2_d$(EXE) cmp_xie2_sig$(EXE)

# Kundur's algorithm 2 (DWT, blind, binary, quantization, detail subbands, reference watermark)

kund2: gen_kund2_sig$(EXE) wm_kund2_e$(EXE) wm_kund2_d$(EXE) cmp_kund2_sig$(EXE)

kund2test: kund2
	gen_kund2_sig$(EXE) -n 1000 -l 3 -q 2 gen_kund2_sig.c > ../sigs/kund2.sig
	wm_kund2_e$(EXE) -s ../sigs/kund2.sig -o ../watermarked/kund2_lena.pgm ../images/lena.pgm
	cjpeg -quality 70 ../watermarked/kund2_lena.pgm | djpeg | wm_kund2_d$(EXE) -s ../sigs/kund2.sig -o ../wms/kund2.wm
	cmp_kund2_sig$(EXE) -s ../sigs/kund2.sig ../wms/kund2.wm

kund2install: kund2
	$(CP) gen_kund2_sig$(EXE) wm_kund2_e$(EXE) wm_kund2_d$(EXE) cmp_kund2_sig$(EXE) $(INSTALLDIR)

kund2man: gen_kund2_sig.ps wm_kund2_e.ps wm_kund2_d.ps

wm_kund2_e$(EXE): wm_kund2_e$(O) $(LIBPREFIX)wm$(LIB) $(LIBPREFIX)wavelet$(LIB)
	$(CC) $(LDFLAGS) -o $@ wm_kund2_e$(O) $(WMLIB) $(WAVELIB) $(LIBS) $(PGMLIBS)

wm_kund2_d$(EXE): wm_kund2_d$(O) $(LIBPREFIX)wm$(LIB) $(LIBPREFIX)wavelet$(LIB)
	$(CC) $(LDFLAGS) -o $@  wm_kund2_d$(O) $(WMLIB) $(WAVELIB) $(LIBS) $(PGMLIBS)

gen_kund2_sig$(EXE): gen_kund2_sig$(O) wm$(O) signature$(O)
	$(CC) $(LDFLAGS) -o $@ gen_kund2_sig$(O) wm$(O) signature$(O) $(LIBS)

cmp_kund2_sig$(EXE): cmp_kund2_sig$(O) $(LIBPREFIX)wm$(LIB)
	$(CC) $(LDFLAGS) -o $@ cmp_kund2_sig$(O) $(WMLIB) $(LIBS)

kund2clean:
	$(RM) gen_kund2_sig$(EXE) wm_kund2_e$(EXE) wm_kund2_d$(EXE) cmp_kund2_sig$(EXE)

# Kundur's algorithm 3 (DWT, blind, binary, quantization, detail subbands)

kund3: gen_kund3_sig$(EXE) wm_kund3_e$(EXE) wm_kund3_d$(EXE) cmp_kund3_sig$(EXE)

kund3test: kund3
	gen_kund3_sig$(EXE) -n 1000 -l 2 -q 2 gen_kund3_sig.c > ../sigs/kund3.sig
	wm_kund3_e$(EXE) -s ../sigs/kund3.sig -o ../watermarked/kund3_lena.pgm ../images/lena.pgm
	wm_kund3_d$(EXE) -s ../sigs/kund3.sig -o ../wms/kund3.wm ../watermarked/kund3_lena.pgm 
	cmp_kund3_sig$(EXE) -s ../sigs/kund3.sig ../wms/kund3.wm

kund3install: kund3
	$(CP) gen_kund3_sig$(EXE) wm_kund3_e$(EXE) wm_kund3_d$(EXE) cmp_kund3_sig$(EXE) $(INSTALLDIR)

kund3man: gen_kund3_sig.ps wm_kund3_e.ps wm_kund3_d.ps

wm_kund3_e$(EXE): wm_kund3_e$(O) $(LIBPREFIX)wm$(LIB) $(LIBPREFIX)wavelet$(LIB)
	$(CC) $(LDFLAGS) -o $@ wm_kund3_e$(O) $(WMLIB) $(WAVELIB) $(LIBS) $(PGMLIBS)

wm_kund3_d$(EXE): wm_kund3_d$(O) $(LIBPREFIX)wm$(LIB) $(LIBPREFIX)wavelet$(LIB)
	$(CC) $(LDFLAGS) -o $@  wm_kund3_d$(O) $(WMLIB) $(WAVELIB) $(LIBS) $(PGMLIBS)

gen_kund3_sig$(EXE): gen_kund3_sig$(O) wm$(O) signature$(O)
	$(CC) $(LDFLAGS) -o $@ gen_kund3_sig$(O) wm$(O) signature$(O) $(LIBS)

cmp_kund3_sig$(EXE): cmp_kund3_sig$(O) $(LIBPREFIX)wm$(LIB)
	$(CC) $(LDFLAGS) -o $@ cmp_kund3_sig$(O) $(WMLIB) $(LIBS)

kund3clean:
	$(RM) gen_kund3_sig$(EXE) wm_kund3_e$(EXE) wm_kund3_d$(EXE) cmp_kund3_sig$(EXE)

# Dugad's algorithm (DWT, blind)

dugad: gen_dugad_sig$(EXE) wm_dugad_e$(EXE) wm_dugad_d$(EXE) cmp_dugad_sig$(EXE)

dugadtest: dugad
	gen_dugad_sig$(EXE) -o ../sigs/dugad.sig
	wm_dugad_e$(EXE) -s ../sigs/dugad.sig -o ../watermarked/dugad_lena.pgm ../images/lena.pgm
	wm_dugad_d$(EXE) -s ../sigs/dugad.sig -o ../wms/dugad.wm ../watermarked/dugad_lena.pgm 
	cmp_dugad_sig$(EXE) -s ../sigs/dugad.sig ../wms/dugad.wm

dugadinstall: dugad
	$(CP) gen_dugad_sig$(EXE) wm_dugad_e$(EXE) wm_dugad_d$(EXE) cmp_dugad_sig$(EXE) $(INSTALLDIR)

dugadman: gen_dugad_sig.ps wm_dugad_e.ps wm_dugad_d.ps

wm_dugad_e$(EXE): wm_dugad_e$(O) $(LIBPREFIX)wm$(LIB) $(LIBPREFIX)wavelet$(LIB)
	$(CC) $(LDFLAGS) -o $@ wm_dugad_e$(O) $(WMLIB) $(WAVELIB) $(LIBS) $(PGMLIBS)

wm_dugad_d$(EXE): wm_dugad_d$(O) $(LIBPREFIX)wm$(LIB) $(LIBPREFIX)wavelet$(LIB)
	$(CC) $(LDFLAGS) -o $@  wm_dugad_d$(O) $(WMLIB) $(WAVELIB) $(LIBS) $(PGMLIBS)

gen_dugad_sig$(EXE): gen_dugad_sig$(O)
	$(CC) $(LDFLAGS) -o $@ gen_dugad_sig$(O) $(LIBS)

cmp_dugad_sig$(EXE): cmp_dugad_sig$(O) $(LIBPREFIX)wm$(LIB)
	$(CC) $(LDFLAGS) -o $@ cmp_dugad_sig$(O) $(WMLIB) $(LIBS)

dugadclean:
	$(RM) gen_dugad_sig$(EXE) wm_dugad_e$(EXE) wm_dugad_d$(EXE) cmp_dugad_sig$(EXE)



clean:	coxclean bruynclean kochclean corviclean xiaclean zhuclean xieclean kund3clean kund2clean \
	dugadclean kimclean wangclean frid2clean toolsclean libraryclean waveletclean xie2clean
	$(RM) *$(O) *.ps ../sigs/* ../wms/* ../watermarked/* 
	

man:	coxman bruynman kochman corviman xiaman xieman toolsman

test:	coxtest bruyntest kochtest corvitest xiatest xietest dugadtest zhutest \
	wangtest frid2test kimtest toolstest kund3test kund2test

install: coxinstall bruyninstall kochinstall corviinstall xiainstall xieinstall \
	dugadinstall zhuinstall wanginstall frid2install kiminstall toolsinstall kund3install kund2install

depend:
	$(MAKEDEP) *.h *.c

