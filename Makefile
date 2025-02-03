SHELL = /bin/bash
GIMPTOOL = gimptool-2.0
CC = gcc
LIBS = $(shell $(GIMPTOOL) --libs)
INCLUDE =

CFLAGS = $(shell $(GIMPTOOL) --cflags)
CFLAGS += -DPERLOVKA_USE_GEGL

ifeq ($(OS), Windows_NT)
	EXECUTABLE = perlovka.exe
	LIBS += -mwindows
else
	EXECUTABLE = perlovka
endif

CORE_OBJS = obj/diff.o obj/perlovka.o obj/position.o obj/solver.o obj/value.o
PLUGIN_OBJS = obj/plugin.o obj/ui.o

TESTS_OBJS = obj/test.o obj/balance_test.o obj/solver_test.o

DEST = $(APPDATA)/GIMP/2.10/plug-ins/perlovka/

.PHONY: all
all: plugin

install:
	cp $(EXECUTABLE) '$(DEST)'

plugin: $(PLUGIN_OBJS) $(CORE_OBJS)
	$(CC) -o $(EXECUTABLE) $(PLUGIN_OBJS) $(CORE_OBJS) $(LIBS)

.PHONY: clean
clean:
	-rm -f obj/*.o $(EXECUTABLE) test.exe

tests: $(TESTS_OBJS) $(CORE_OBJS)
	$(CC) -o test $(TESTS_OBJS) $(CORE_OBJS)

$(CORE_OBJS): obj/%.o: src/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $^ -o $@

$(PLUGIN_OBJS): obj/%.o: src/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $^ -o $@

$(TESTS_OBJS): obj/%.o: tests/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $^ -o $@

