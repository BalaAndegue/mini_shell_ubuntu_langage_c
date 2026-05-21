# =============================================================================
# minishell — Professional Makefile
# =============================================================================

NAME        := minishell
CC          := gcc
CFLAGS      := -Wall -Wextra -Wpedantic -Wshadow -Wformat=2 -std=c11
CFLAGS      += -D_POSIX_C_SOURCE=200809L
DEBUGFLAGS  := -g3 -O0 -DDEBUG -fsanitize=address,undefined
RELEASEFLAGS:= -O2 -DNDEBUG
COVFLAGS    := -g --coverage -O0

# --- Directories --------------------------------------------------------------
SRCDIR  := src
INCDIR  := include
OBJDIR  := build/obj
BINDIR  := build/bin
DEPDIR  := build/dep

# --- Sources ------------------------------------------------------------------
SRCS := $(shell find $(SRCDIR) -name '*.c')
OBJS := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))
DEPS := $(patsubst $(SRCDIR)/%.c,$(DEPDIR)/%.d,$(SRCS))

# --- Targets ------------------------------------------------------------------
.DEFAULT_GOAL := all

.PHONY: all debug release clean fclean re test cov lint docs help

all: CFLAGS += $(RELEASEFLAGS)
all: $(BINDIR)/$(NAME)

debug: CFLAGS += $(DEBUGFLAGS)
debug: $(BINDIR)/$(NAME)

release: fclean all

# --- Link ---------------------------------------------------------------------
LDLIBS := $(shell pkg-config --libs readline 2>/dev/null || echo "")

$(BINDIR)/$(NAME): $(OBJS)
	@mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)
	@echo "[LINK] $@"

# --- Compile ------------------------------------------------------------------
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(dir $@) $(dir $(DEPDIR)/$*.d)
	$(CC) $(CFLAGS) -I$(INCDIR) -MMD -MF $(DEPDIR)/$*.d -c $< -o $@
	@echo "[CC]   $<"

-include $(DEPS)

# --- Tests --------------------------------------------------------------------
test:
	@echo "Running unit tests..."
	@$(MAKE) -C tests/unit CC=$(CC) CFLAGS="$(CFLAGS) -I$(INCDIR)"

test-integration:
	@bash scripts/run_tests.sh

# --- Coverage -----------------------------------------------------------------
cov: CFLAGS += $(COVFLAGS)
cov: fclean $(BINDIR)/$(NAME)
	@echo "Run tests then: gcov $(SRCS)"
	@echo "For HTML: lcov --capture --directory . --output-file cov.info && genhtml cov.info --output-directory cov_html"

# --- Static analysis ----------------------------------------------------------
lint:
	@command -v cppcheck >/dev/null && cppcheck --enable=all --suppress=missingIncludeSystem \
		-I$(INCDIR) $(SRCDIR) 2>&1 || echo "cppcheck not found"
	@command -v clang-tidy >/dev/null && clang-tidy $(SRCS) -- -I$(INCDIR) $(CFLAGS) || echo "clang-tidy not found"

# --- Documentation ------------------------------------------------------------
docs:
	@command -v doxygen >/dev/null && doxygen docs/Doxyfile || echo "doxygen not found"

# --- Valgrind -----------------------------------------------------------------
memcheck: debug
	valgrind --leak-check=full --show-leak-kinds=all \
	         --track-origins=yes --error-exitcode=1 \
	         $(BINDIR)/$(NAME)

# --- Clean --------------------------------------------------------------------
clean:
	rm -rf build/

fclean: clean
	rm -f $(BINDIR)/$(NAME) vgcore.* *.gcda *.gcno *.gcov

re: fclean all

# --- Help ---------------------------------------------------------------------
help:
	@echo "Targets:"
	@echo "  all          Build release binary (default)"
	@echo "  debug        Build with -g3 + AddressSanitizer + UBSan"
	@echo "  release      Clean + release build"
	@echo "  test         Run unit tests"
	@echo "  test-integration  Run integration test suite"
	@echo "  cov          Build with coverage instrumentation"
	@echo "  lint         Run cppcheck + clang-tidy"
	@echo "  memcheck     Run under Valgrind"
	@echo "  docs         Generate Doxygen HTML"
	@echo "  clean        Remove build artifacts"
	@echo "  fclean       Remove build artifacts + binary"
	@echo "  re           Full rebuild"
