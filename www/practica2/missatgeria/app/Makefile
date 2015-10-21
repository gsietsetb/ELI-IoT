
INCDIR=../src/include
LIBDIR=../lib
SRCDIR=src

LIBNAME=web-pract2

OBJS=build/inici.o build/request.o

CFLAGS=-g -Wall -I$(INCDIR)

#VPATH=$(INCDIR) $(SRCDIR)

help:
	@echo "Targets:"
	@echo "    build/example.cgi   Construcción del CGI"
	@echo "    all                 Construcción de la aplicación en el directorio build"
	@echo "    clean               Borrado del directorio build"

all: 	build/index.cgi
	cp -pu www/* build
	rm -f $(OBJS)

build:
	mkdir build

build/index.cgi: build $(OBJS) $(LIBDIR)/$(LIBNAME).a
	$(LINK.c) -o $@ $(OBJS) $(LIBDIR)/$(LIBNAME).a

build/%.o: $(SRCDIR)/%.c
	$(COMPILE.c) -o $@ $<

$(OBJS): build $(INCDIR)/urlcoding.h $(INCDIR)/dbc.h $(INCDIR)/strlist.h $(INCDIR)/cgilib.h $(SRCDIR)/request.h

clean:
	rm -fR build *~
