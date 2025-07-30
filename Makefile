CC = gcc
LDFLAGS = -lm

SRCDIR   = src
OBJDIR   = obj
BINDIR   = bin

SOURCES = $(wildcard $(SRCDIR)/*.c)

EXEC_RELEASE = $(BINDIR)/solver
EXEC_DEBUG   = $(BINDIR)/debug

OBJECTS_RELEASE = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/release/%.o, $(SOURCES))
OBJECTS_DEBUG   = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/debug/%.o, $(SOURCES))

COMMON_FLAGS = -Iinclude -Wall -Wextra -Wshadow -Wformat=2
RELEASE_CFLAGS = -O3 -march=native -DNDEBUG
DEBUG_CFLAGS = -g -Werror -Wpedantic -gdwarf-4 -DDEBUG

.PHONY: all release debug clean

all: release

release: $(EXEC_RELEASE)
debug: $(EXEC_DEBUG)

$(EXEC_RELEASE): $(OBJECTS_RELEASE)
	mkdir -p $(BINDIR)
	$(CC) $(OBJECTS_RELEASE) -o $@ $(LDFLAGS)

$(EXEC_DEBUG): $(OBJECTS_DEBUG)
	mkdir -p $(BINDIR)
	$(CC) $(OBJECTS_DEBUG) -o $@ $(LDFLAGS)

$(OBJDIR)/release/%.o: $(SRCDIR)/%.c
	mkdir -p $(OBJDIR)/release
	$(CC) $(CFLAGS) $(COMMON_FLAGS) $(RELEASE_CFLAGS) -c $< -o $@

$(OBJDIR)/debug/%.o: $(SRCDIR)/%.c
	mkdir -p $(OBJDIR)/debug
	$(CC) $(CFLAGS) $(COMMON_FLAGS) $(DEBUG_CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR) $(BINDIR)
