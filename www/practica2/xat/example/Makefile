
INCDIR=../src/include
LIBDIR=../lib
SRCDIR=src

LIBNAME=web-pract2

OBJS=build/example.o build/example2.o

CFLAGS=-g -Wall -I$(INCDIR)

#VPATH=$(INCDIR) $(SRCDIR)

help:
	@echo "Targets:"
	@echo "    build/example.cgi   Construcción del CGI"
	@echo "    all                 Construcción de la aplicación en el directorio build"
	@echo "    clean               Borrado del directorio build"

all: build/example.cgi build/example2.cgi
	cp -pu www/* build
	rm -f $(OBJS)

build:
	mkdir build

build/example.cgi: build build/example.o $(LIBDIR)/$(LIBNAME).a
	$(LINK.c) -o $@ build/example.o $(LIBDIR)/$(LIBNAME).a

build/example2.cgi: build build/example2.o $(LIBDIR)/$(LIBNAME).a
	$(LINK.c) -o $@ build/example2.o $(LIBDIR)/$(LIBNAME).a

build/%.o: $(SRCDIR)/%.c
	$(COMPILE.c) -o $@ $<

$(OBJS): build $(INCDIR)/urlcoding.h $(INCDIR)/dbc.h $(INCDIR)/strlist.h $(INCDIR)/cgilib.h

clean:
	rm -fR build *~
