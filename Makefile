#! /usr/bin/make -f
SHELL=/bin/sh

DESTDIR?=/usr/local
prefix?=${DESTDIR}

# files that need mode 755
EXEC_FILES=fobiad

all:
	@echo "usage: make install     -> installs only"
	@echo "       make install-all -> installs git-ftp and man pages"	
	@echo "       make clean"

install:
	mkdir -p build
	gcc main.c -o build/$(EXEC_FILES)

brun: 
	@echo "---------------"
	@echo ""
	@build/$(EXEC_FILES)

install-all: install brun


clean:
	rm -r build

