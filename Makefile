CC = gcc
LDFLAGS = -lm

SRCDIR = src
OBJDIR = obj
BINDIR = bin

EXEC_GAME = $(BINDIR)/game
EXEC_SOLVER = $(BINDIR)/solver

CFLAGS = -Iinclude -Wall -Wextra -Wshadow -O3 -march=native -DNDEBUG

ALL_C_SOURCES = $(wildcard $(SRCDIR)/*.c)
GAME_SOURCES = $(filter-out $(SRCDIR)/solver.c $(SRCDIR)/book_builder.c, $(ALL_C_SOURCES))
SOLVER_SOURCES = $(filter-out $(SRCDIR)/game.c $(SRCDIR)/book_builder.c, $(ALL_C_SOURCES))

GAME_OBJECTS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(GAME_SOURCES))
SOLVER_OBJECTS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SOLVER_SOURCES))

.PHONY: all clean

all: $(EXEC_GAME) $(EXEC_SOLVER)

$(EXEC_GAME): $(GAME_OBJECTS)
	@mkdir -p $(BINDIR)
	$(CC) $^ -o $@ $(LDFLAGS)

$(EXEC_SOLVER): $(SOLVER_OBJECTS)
	@mkdir -p $(BINDIR)
	$(CC) $^ -o $@ $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR) $(BINDIR) book.bin
