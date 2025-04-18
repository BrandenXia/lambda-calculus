#include "string.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "cpp_magic.h"
#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

#define DEBUG 1

typedef unsigned int Variable;
typedef struct Abstraction Abstraction;
typedef struct Application Application;
typedef struct Expr Expr;

struct Abstraction {
  Expr *body;
};

struct Application {
  Expr *func;
  Expr *arg;
};

typedef enum { EXPR_VAR, EXPR_ABS, EXPR_APP } ExprType;

typedef struct Expr {
  ExprType type;
  union {
    Variable var;
    Abstraction abs;
    Application app;
  };
} Expr;

typedef Expr *Stack;

#define ERROR(msg, ...)                                                        \
  {                                                                            \
    fprintf(stderr, "Error: " msg "\n", ##__VA_ARGS__);                        \
    exit(EXIT_FAILURE);                                                        \
  }                                                                            \
  while (0)

#define CHECK_NULL_ARGS_(var) !(var)
#define OR_OP() ||
#define CHECK_NULL_ARGS(...)                                                   \
  if (EVAL(MAP(CHECK_NULL_ARGS_, OR_OP, __VA_ARGS__)))                         \
    ERROR("NULL argument(s) passed to function");

#define NEW_EXPR                                                               \
  ({                                                                           \
    Expr *e = malloc(sizeof(Expr));                                            \
    if (!e)                                                                    \
      ERROR("Memory allocation failed");                                       \
    e;                                                                         \
  })

#define NEW_EXPR_IMPL(check, initialize)                                       \
  {                                                                            \
    CHECK_NULL_ARGS check;                                                     \
    Expr *e = NEW_EXPR;                                                        \
    initialize;                                                                \
    return e;                                                                  \
  }

Expr *new_abs(Expr *body) NEW_EXPR_IMPL((body), {
  e->type = EXPR_ABS;
  e->abs.body = body;
});

Expr *new_app(Expr *func, Expr *arg) NEW_EXPR_IMPL((func, arg), {
  e->type = EXPR_APP;
  e->app.func = func;
  e->app.arg = arg;
});

Expr *new_var(Variable var) NEW_EXPR_IMPL((var), {
  e->type = EXPR_VAR;
  e->var = var;
});

#undef NEW_EXPR_IMPL
#undef NEW_EXPR
#undef CHECK_NULL_ARGS
#undef CHECK_NULL_ARGS_

void _print_expr(const Expr *expr) {
  if (!expr)
    ERROR("NULL expression");

  switch (expr->type) {
  case EXPR_VAR:
    printf("%u", expr->var);
    break;
  case EXPR_ABS: {
    printf("(λ ");
    _print_expr(expr->abs.body);
    putchar(')');
    break;
  }
  case EXPR_APP: {
    putchar('(');
    _print_expr(expr->app.func);
    putchar(' ');
    _print_expr(expr->app.arg);
    putchar(')');
    break;
  }
  }
}

#define print_expr(expr)                                                       \
  {                                                                            \
    _print_expr((expr));                                                       \
    putchar('\n');                                                             \
  }                                                                            \
  while (0)

#define STRINGIFY_INNER(x) #x
#define STRINGIFY(x) STRINGIFY_INNER(x)
#if DEBUG
#define dbg_stack(s)                                                           \
  {                                                                            \
    puts("-------------------");                                               \
    puts("Stack contents at " __FILE__ ":" STRINGIFY(__LINE__) ":");           \
                                                                               \
    for (ptrdiff_t i = arrlen((s)) - 1, j = 1; i >= 0; --i, j++) {             \
      printf("  [%ld] ", j);                                                   \
      print_expr(&(s)[i]);                                                     \
    }                                                                          \
                                                                               \
    if (arrlen((s)) == 0)                                                      \
      puts("  (empty stack)");                                                 \
                                                                               \
    puts("-------------------");                                               \
  }                                                                            \
  while (0)
#else
#define print_stack(s)
#endif

Expr *eval(Expr *expr, Stack *s) {
  switch (expr->type) {
  case EXPR_VAR: {
    Variable var = expr->var;
    ptrdiff_t len = arrlen(*s);
    if (var > len)
      ERROR("Variable %u not found in stack", var);
    return s[len - var];
  }
  case EXPR_APP: {
    Expr *arg = eval(expr->app.arg, s);
    Expr *func = eval(expr->app.func, s);
    dbg_stack(*s);
    arrput(*s, *arg);
    Expr *res = eval(func->abs.body, s);
    dbg_stack(*s);
    return res;
  }
  default:
    return expr;
  }
}

int main(int argc, char *argv[]) {
  Stack s = NULL;

  Expr *one = new_var(1);
  Expr *two = new_var(2);
  Expr *id = new_abs(one);
  Expr *sb = new_abs(two);

  Expr *inner = new_abs(two);
  Expr *outer = new_abs(inner);
  print_expr(outer);

  Expr *app1 = new_app(outer, id);
  Expr *app2 = new_app(app1, sb);
  print_expr(app2);
  Expr *expr = eval(app2, &s);
  print_expr(expr);

  return EXIT_SUCCESS;
}
