CC = gcc
LDFLAGS = -lm

COMMON_FLAGS = -Iinclude -Wall -Wextra -Wshadow -Wformat=2

RELEASE_CFLAGS = -O3 -march=native -DNDEBUG

DEBUG_CFLAGS = -g -Werror -Wpedantic -gdwarf-4

CFLAGS = $(COMMON_FLAGS) $(RELEASE_CFLAGS)

test: CFLAGS = $(COMMON_FLAGS) $(DEBUG_CFLAGS)
bench: CFLAGS = $(COMMON_FLAGS) $(RELEASE_CFLAGS)

SRCDIR = src
OBJDIR = obj
BINDIR = bin
TESTDIR = bench

SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SOURCES))
EXEC = $(BINDIR)/solver

.PHONY: all clean test bench

all: $(EXEC)

$(EXEC): $(OBJECTS)
	mkdir -p $(BINDIR)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

test: clean $(EXEC)
	python3 $(TESTDIR)/benchmark.py $(EXEC)

bench: clean $(EXEC)
	python3 $(TESTDIR)/benchmark.py $(EXEC)

clean:
	rm -rf $(OBJDIR) $(BINDIR)
