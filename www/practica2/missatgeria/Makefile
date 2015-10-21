
INCDIR=src/include
LIB_SRCDIR=src/lib
LIB_DIR=lib
LIB_NAME=web-pract2

BIN_SRCDIR=src/utils-c
BIN_SHDIR=src/utils-sh
BIN_DIR=bin
BIN_APPNAMES=db_create_table db_drop_table db_insert db_delete db_select db_create_db
BIN_SHNAMES=db_drop_db


INCLUDES=$(INCDIR)/urlcoding.h $(INCDIR)/dbc.h $(INCDIR)/strlist.h $(INCDIR)/cgilib.h
LIB_OBJS=urlcoding.o read_input.o dbc.o strlist.o cgilib.o
BIN_OBJS=$(BIN_APPNAMES:%=%.o)

CFLAGS=-g -Wall -I$(INCDIR)

#VPATH=$(INCDIR) $(SRCDIR)

.PHONY: lib bins all clean

help:
	@echo "Targets:"
	@echo "    all      Construcción de la biblioteca y utilidades"
	@echo "    clean    Borrado de la biblioteca y utilidades"

all: lib bins


lib: $(LIB_DIR)/$(LIB_NAME).a
	rm -f $(LIB_OBJS)

$(LIB_DIR)/$(LIB_NAME).a: $(LIB_OBJS) Makefile
	test -d $(LIB_DIR) || mkdir $(LIB_DIR)
	ar rc $@ $(LIB_OBJS)
	ranlib $@

%.o: $(LIB_SRCDIR)/%.c
	$(COMPILE.c) -o $@ $<

$(LIB_OBJS): $(INCLUDES) Makefile


bins: $(BIN_APPNAMES:%=$(BIN_DIR)/%) $(BIN_SHNAMES:%=$(BIN_DIR)/%)
	rm -f $(BIN_OBJS)

$(BIN_DIR)/%: %.o
	test -d $(BIN_DIR) || mkdir $(BIN_DIR)
	$(LINK.c) -o $@ $< $(LIB_DIR)/$(LIB_NAME).a

%.o: $(BIN_SRCDIR)/%.c
	$(COMPILE.c) -o $@ $<

$(BIN_APPNAMES:%=$(BIN_DIR)/%): $(INCLUDES) $(LIB_DIR)/$(LIB_NAME).a Makefile

$(BIN_DIR)/%: $(BIN_SHDIR)/%
	cp -pu $< $@
	chmod +x $@


clean:
	rm -fR $(LIB_OBJS) $(BIN_OBJS) $(LIB_DIR) $(BIN_DIR)

