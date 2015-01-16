CFLAGS = -std=gnu99 -fPIC -I../include/ -L../lib/x86_64-linux-gnu/ -Wall -Wextra -O2
LIBS = -llua52 -ldl -lmosquitto -lssl -lcares
DESTDIR = /usr/local

all : lua_auth_plugin.so

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

lua_auth_plugin.so : lua_auth_plugin.o
	$(CC) $(CFLAGS) -shared -o $@ $^  $(LIBS)

install: lua_auth_plugin.so
	install -s -m 755 lua_auth_plugin.so $(DESTDIR)/lib

clean :
	rm -f lua_auth_plugin.so *.o

.PHONY: all clean
