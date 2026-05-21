# =============================================================================
# minishell — Professional Makefile
# =============================================================================

NAME        := minishell
CC          := gcc
WARNINGS    := -Wall -Wextra -Wpedantic -Wshadow -Wformat=2
BASE_CFLAGS := $(WARNINGS) -std=c11 -D_POSIX_C_SOURCE=200809L

SRCDIR  := src
INCDIR  := include

# --- Source discovery ---------------------------------------------------------
SRCS := $(shell find $(SRCDIR) -name '*.c')

# --- Public targets (entry points) --------------------------------------------
.PHONY: all debug release clean fclean re test test-integration cov lint docs memcheck help

all:
	$(MAKE) _build \
	    BUILD_TYPE=release \
	    EXTRA_CFLAGS="-O2 -DNDEBUG"

debug:
	$(MAKE) _build \
	    BUILD_TYPE=debug \
	    EXTRA_CFLAGS="-g3 -O0 -DDEBUG -fsanitize=address,undefined"

cov:
	$(MAKE) _build \
	    BUILD_TYPE=cov \
	    EXTRA_CFLAGS="-g -O0 --coverage"
	@echo ""
	@echo "Run your tests, then:"
	@echo "  lcov --capture --directory build/cov --output-file cov.info"
	@echo "  genhtml cov.info --output-directory cov_html"

release: fclean all

# --- Internal build target (called recursively) -------------------------------
BUILD_TYPE  ?= release
EXTRA_CFLAGS ?=

OBJDIR := build/$(BUILD_TYPE)/obj
BINDIR := build/$(BUILD_TYPE)/bin
DEPDIR := build/$(BUILD_TYPE)/dep

CFLAGS := $(BASE_CFLAGS) $(EXTRA_CFLAGS)
OBJS   := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))
DEPS   := $(patsubst $(SRCDIR)/%.c,$(DEPDIR)/%.d,$(SRCS))
LDLIBS := $(shell pkg-config --libs readline 2>/dev/null || echo "")

.PHONY: _build
_build: $(BINDIR)/$(NAME)
	@echo "[DONE] build/$(BUILD_TYPE)/bin/$(NAME)"

$(BINDIR)/$(NAME): $(OBJS)
	@mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(dir $@) $(dir $(DEPDIR)/$*.d)
	$(CC) $(CFLAGS) -I$(INCDIR) -MMD -MF $(DEPDIR)/$*.d -c $< -o $@
	@echo "[CC]   $<"

-include $(DEPS)

# --- Tests --------------------------------------------------------------------
test:
	@$(MAKE) -C tests/unit \
	    CC=$(CC) \
	    CFLAGS="$(BASE_CFLAGS) -g3 -fsanitize=address,undefined -I../../$(INCDIR)"

test-integration:
	@bash scripts/run_tests.sh

# --- Static analysis ----------------------------------------------------------
lint:
	@command -v cppcheck >/dev/null \
	    && cppcheck --enable=all --suppress=missingIncludeSystem \
	               -I$(INCDIR) $(SRCDIR) 2>&1 \
	    || echo "cppcheck not found — install: apt install cppcheck"
	@command -v clang-tidy >/dev/null \
	    && clang-tidy $(SRCS) -- -I$(INCDIR) $(BASE_CFLAGS) \
	    || echo "clang-tidy not found — install: apt install clang-tidy"

# --- Documentation ------------------------------------------------------------
docs:
	@command -v doxygen >/dev/null \
	    && doxygen docs/Doxyfile \
	    || echo "doxygen not found — install: apt install doxygen"

# --- Valgrind (release build — incompatible with ASan) ------------------------
memcheck:
	@$(MAKE) all
	valgrind --leak-check=full --show-leak-kinds=all \
	         --track-origins=yes --error-exitcode=1 \
	         build/release/bin/$(NAME)

# --- Clean --------------------------------------------------------------------
clean:
	rm -rf build/

fclean: clean
	rm -f vgcore.* *.gcda *.gcno *.gcov

re: fclean all

# --- Help ---------------------------------------------------------------------
help:
	@echo "Targets:"
	@echo "  all               Release build  → build/release/bin/$(NAME)"
	@echo "  debug             Debug build    → build/debug/bin/$(NAME)"
	@echo "                    (AddressSanitizer + UBSan + -g3, no -O)"
	@echo "  cov               Coverage build → build/cov/bin/$(NAME)"
	@echo "  release           Clean then release build"
	@echo "  re                Clean then rebuild (same as release)"
	@echo "  test              Unit tests (ASan enabled)"
	@echo "  test-integration  Integration test suite"
	@echo "  lint              cppcheck + clang-tidy"
	@echo "  memcheck          Valgrind on release binary"
	@echo "  docs              Doxygen HTML documentation"
	@echo "  clean             Remove build/"
	@echo "  fclean            Remove build/ + coverage files"
