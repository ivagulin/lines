VPATH += ..
CC=gcc
LIBS=gtk+-2.0
CFLAGS = $(shell pkg-config --cflags $(LIBS)) -Ic:/mingw/include/SDL
LDFLAGS= -Wl,--subsystem,windows -Lc:/mingw/bin -Lc:/gtk/bin -lsdl -lsdl_mixer -lsmpeg -lgtk-win32-2.0-0 -lgdk-win32-2.0-0 -latk-1.0-0 -lgdk_pixbuf-2.0-0 -lpangowin32-1.0-0 -lpangocairo-1.0-0 -lpango-1.0-0 -lcairo-2 -lgobject-2.0-0 -lgmodule-2.0-0 -lglib-2.0-0 -lintl -liconv
SRC = main.c table.c algorithms.c sound.c scoremgr.c
OBJS = $(SRC:.c=.o)
DEPENDS = $(SRC:.c=.d)
TARGET = lines

$(TARGET): $(OBJS)
	$(CC) --enable-auto-import $(LDFLAGS) -o $@ $^

%.o: %.c %.d Makefile
	$(CC) -c $(CFLAGS) -mno-cygwin -mms-bitfields -O0 --std=c99 -Wall -o $@ $<

%.d: %.c
	@echo Dependence for $<
	@$(CC) -M $(CFLAGS) $< > $@

clean:                                                                                                                                                             
	@del $(DEPENDS)
	@del $(OBJS)
	@del $(TARGET)

-include $(DEPENDS)

