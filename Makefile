CC=clang
TARGET=harryplotter
LUA_INCDIR=/usr/local/include/lua5.3
INST_LIBDIR=/usr/local/lib/lua/5.3//

local_CFLAGS=$(CFLAGS)

all:
	$(CC) $(local_CFLAGS) -o $(TARGET).so src/hp.c -I$(LUA_INCDIR) $(LIBFLAG)

install:
	mkdir -p $(INST_LIBDIR)
	cp $(TARGET).so $(INST_LIBDIR)

.PHONY: all install
