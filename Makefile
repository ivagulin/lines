.PHONY=win
LIBS=gtk+-2.0 sdl
CFLAGS = $(shell pkg-config --cflags $(LIBS))
LDFLAGS = $(shell pkg-config --libs $(LIBS)) -lSDL_mixer
SRC = main.c table.c algorithms.c sound.c scoremgr.c serializable.c
OBJS = $(SRC:.c=.o)
DEPENDS = $(SRC:.c=.d)
TARGET = lines

all:$(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

exe:
	wine mingw32-make -C win
	cp win/lines.exe /mnt/hdc/tmp/lines-stuff/

%.o: %.c %.d Makefile
	$(CC) -c $(CFLAGS) -g -O0 --std=c99 -Wall -o $@ $<

%.d: %.c
	@echo Dependence for $<
	@$(CC) -M $(CFLAGS) $< > $@

clean:                                                                                                                                                             
	@rm -vf $(DEPENDS)
	@rm -vf $(OBJS)
	@rm -vf $(TARGET)

-include $(DEPENDS)

