#! /usr/bin/make -f
SHELL=/bin/sh

DESTDIR?=/usr/local
prefix?=${DESTDIR}

# files that need mode 755
EXEC_FILES=fobiad

all:
	@echo "usage: make install     -> installs git-ftp only"
	@echo "       make install-man -> installs man pages only"
	@echo "       make install-all -> installs git-ftp and man pages"	
	@echo "       make uninstall"
	@echo "       make uninstall-man"
	@echo "       make uninstall-all"
	@echo "       make clean"

install:
	mkdir build

install-build:
	@echo mkdir build

install-all: install install-build


clean:
	rm -r build

