#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "../../src/lexer/lexer.h"

static int passed = 0;
static int failed = 0;

#define CHECK(cond, msg) do { \
    if (cond) { printf("PASS  %s\n", msg); passed++; } \
    else       { printf("FAIL  %s\n", msg); failed++; } \
} while (0)

static void test_basic_split(void)
{
    char line[] = "ls -la /tmp";
    char *argv[16];
    int argc = tokenize(line, argv, 15);

    CHECK(argc == 3,              "basic: argc == 3");
    CHECK(strcmp(argv[0], "ls") == 0,    "basic: argv[0] == ls");
    CHECK(strcmp(argv[1], "-la") == 0,   "basic: argv[1] == -la");
    CHECK(strcmp(argv[2], "/tmp") == 0,  "basic: argv[2] == /tmp");
    CHECK(argv[3] == NULL,               "basic: argv[argc] == NULL");
}

static void test_empty_line(void)
{
    char line[] = "\n";
    char *argv[16];
    int argc = tokenize(line, argv, 15);

    CHECK(argc == 0,       "empty: argc == 0");
    CHECK(argv[0] == NULL, "empty: argv[0] == NULL");
}

static void test_extra_spaces(void)
{
    char line[] = "  echo   hello   world  ";
    char *argv[16];
    int argc = tokenize(line, argv, 15);

    CHECK(argc == 3,                      "spaces: argc == 3");
    CHECK(strcmp(argv[0], "echo") == 0,   "spaces: argv[0] == echo");
    CHECK(strcmp(argv[2], "world") == 0,  "spaces: argv[2] == world");
}

static void test_single_token(void)
{
    char line[] = "pwd\n";
    char *argv[16];
    int argc = tokenize(line, argv, 15);

    CHECK(argc == 1,                    "single: argc == 1");
    CHECK(strcmp(argv[0], "pwd") == 0,  "single: argv[0] == pwd");
}

static void test_max_args_limit(void)
{
    char line[] = "a b c d e";
    char *argv[4];
    int argc = tokenize(line, argv, 3);

    CHECK(argc == 3,       "limit: truncated at max_args");
    CHECK(argv[3] == NULL, "limit: sentinel still written");
}

int main(void)
{
    printf("=== lexer unit tests ===\n");
    test_basic_split();
    test_empty_line();
    test_extra_spaces();
    test_single_token();
    test_max_args_limit();

    printf("\nResults: %d passed, %d failed\n", passed, failed);
    return failed > 0 ? 1 : 0;
}
