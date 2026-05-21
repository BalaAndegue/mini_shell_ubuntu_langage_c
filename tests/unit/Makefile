CC      := gcc
CFLAGS  := -Wall -Wextra -std=c11 -g3 -fsanitize=address,undefined
INCDIR  := ../../include
SRCDIR  := ../../src

LEXER_SRCS := $(SRCDIR)/lexer/lexer.c

.PHONY: all clean

all: test_lexer
	./test_lexer

test_lexer: test_lexer.c $(LEXER_SRCS)
	$(CC) $(CFLAGS) -I$(INCDIR) -o $@ $^

clean:
	rm -f test_lexer
