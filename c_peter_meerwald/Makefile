# Makefile

WEBDIR = /home/scicomp/pmeerw/public-www/Watermarking/source

all: meerwald fotopoulos 

meerwald:
	cd Meerwald ; make

fotopoulos:
	cd Fotopoulos ; make

install:
	cd Meerwald ; make install
	cd Fotopoulos ; make install

clean:
	cd Meerwald ; make clean
	cd Fotopoulos ; make clean

dist:
	cp -v manual.pdf manual.ps $(WEBDIR)
	cp -v ANNOUNCEMENT $(WEBDIR)
	cd linux_bin ; tar -czf ../wm_linux_bin.tar.gz *
	mv -v wm_linux_bin.tar.gz $(WEBDIR)
	cd win32_bin ; zip -9 ../wm_win32_bin.zip *
	mv -v wm_win32_bin.zip $(WEBDIR)
	tar -czf wm_source_bin.tar.gz *
	mv -v wm_source_bin.tar.gz $(WEBDIR)
	zip -9r wm_source_bin.zip *
	mv -v wm_source_bin.zip $(WEBDIR)
	tar -czf wm_source.tar.gz ANNOUNCEMENT Fotopoulos/* Meerwald/* Makefile README images/* make/* manual.* sigs/.dummy watermarked/.dummy wms/.dummy win32_bin/.dummy linux_bin/.dummy
	mv -v wm_source.tar.gz $(WEBDIR)
	zip -9r wm_source.zip ANNOUNCEMENT Fotopoulos/* Meerwald/* Makefile README images/* make/* manual.* sigs/.dummy watermarked/.dummy wms/.dummy linux_bin/.dummy win32_bin/.dummy
	mv -v wm_source.zip $(WEBDIR)
	chmod a+r $(WEBDIR)/*
