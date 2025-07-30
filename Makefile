CC = gcc
LDFLAGS = -lm

SRCDIR   = src
OBJDIR   = obj
BINDIR   = bin

ENGINE_SOURCES  = $(SRCDIR)/engine.c $(SRCDIR)/bitboard.c $(SRCDIR)/table.c $(SRCDIR)/ordering.c
GAME_SOURCES    = $(SRCDIR)/main.c $(SRCDIR)/player.c $(SRCDIR)/interface.c $(ENGINE_SOURCES)
BUILDER_SOURCES = $(SRCDIR)/book_builder.c $(ENGINE_SOURCES)

EXEC_GAME    = $(BINDIR)/connect4
EXEC_BUILDER = $(BINDIR)/book_builder

OBJECTS_GAME_RELEASE    = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/release/%.o, $(GAME_SOURCES))
OBJECTS_GAME_DEBUG      = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/debug/%.o, $(GAME_SOURCES))
OBJECTS_BUILDER_RELEASE = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/release/%.o, $(BUILDER_SOURCES))
OBJECTS_BUILDER_DEBUG   = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/debug/%.o, $(BUILDER_SOURCES))

COMMON_FLAGS   = -Iinclude -Wall -Wextra -Wshadow
RELEASE_CFLAGS = -O3 -march=native -DNDEBUG
DEBUG_CFLAGS   = -g -Werror -Wpedantic

.PHONY: all release debug clean run build-book

all: release

release: $(EXEC_GAME) $(EXEC_BUILDER)

debug: $(EXEC_GAME)_debug $(EXEC_BUILDER)_debug

$(EXEC_GAME): $(OBJECTS_GAME_RELEASE)
	@mkdir -p $(BINDIR)
	$(CC) $^ -o $@ $(LDFLAGS)

$(EXEC_GAME)_debug: $(OBJECTS_GAME_DEBUG)
	@mkdir -p $(BINDIR)
	$(CC) $^ -o $@ $(LDFLAGS)

$(EXEC_BUILDER): $(OBJECTS_BUILDER_RELEASE)
	@mkdir -p $(BINDIR)
	$(CC) $^ -o $@ $(LDFLAGS)

$(EXEC_BUILDER)_debug: $(OBJECTS_BUILDER_DEBUG)
	@mkdir -p $(BINDIR)
	$(CC) $^ -o $@ $(LDFLAGS)

$(OBJDIR)/release/%.o: $(SRCDIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(COMMON_FLAGS) $(RELEASE_CFLAGS) -c $< -o $@

$(OBJDIR)/debug/%.o: $(SRCDIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(COMMON_FLAGS) $(DEBUG_CFLAGS) -c $< -o $@

run: release
	./$(EXEC_GAME)

build-book: release
	./$(EXEC_BUILDER) 10

clean:
	rm -rf $(OBJDIR) $(BINDIR) book.bin