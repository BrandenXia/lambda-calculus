#include "string.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "cpp_magic.h"
#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

typedef char *Variable;
typedef struct Abstraction Abstraction;
typedef struct Application Application;
typedef struct Expr Expr;

struct Abstraction {
  Variable bound_var;
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
  } u;
} Expr;

typedef struct {
  Variable key;
  Expr value;
} Scope;
typedef Scope *Stack;

#define CHECK_NULL_ARGS_(var) !(var)
#define OR_OP() ||
#define CHECK_NULL_ARGS(...)                                                   \
  if (EVAL(MAP(CHECK_NULL_ARGS_, OR_OP, __VA_ARGS__))) {                       \
    fprintf(stderr, "Error: NULL argument(s) passed to function\n");           \
    exit(EXIT_FAILURE);                                                        \
  }

#define NEW_EXPR                                                               \
  ({                                                                           \
    Expr *e = malloc(sizeof(Expr));                                            \
    if (!e) {                                                                  \
      fprintf(stderr, "Error: Memory allocation failed\n");                    \
      exit(EXIT_FAILURE);                                                      \
    }                                                                          \
    e;                                                                         \
  })

#define NEW_EXPR_IMPL(check, initialize)                                       \
  {                                                                            \
    CHECK_NULL_ARGS check;                                                     \
    Expr *e = NEW_EXPR;                                                        \
    initialize;                                                                \
    return e;                                                                  \
  }

Expr *new_abs(Variable bound_var, Expr *body) NEW_EXPR_IMPL((bound_var, body), {
  e->type = EXPR_ABS;
  e->u.abs.bound_var = bound_var;
  e->u.abs.body = body;
});

Expr *new_app(Expr *func, Expr *arg) NEW_EXPR_IMPL((func, arg), {
  e->type = EXPR_APP;
  e->u.app.func = func;
  e->u.app.arg = arg;
});

Expr *new_var(Variable var) NEW_EXPR_IMPL((var), {
  e->type = EXPR_VAR;
  e->u.var = var;
});

#undef NEW_EXPR_IMPL
#undef NEW_EXPR
#undef CHECK_NULL_ARGS
#undef CHECK_NULL_ARGS_

void _print_expr(const Expr *expr) {
  if (!expr) {
    printf("Error: NULL expression\n");
    return;
  }

  switch (expr->type) {
  case EXPR_VAR:
    printf("%s", expr->u.var);
    break;
  case EXPR_ABS: {
    printf("(λ%s . ", expr->u.abs.bound_var);
    _print_expr(expr->u.abs.body);
    putchar(')');
    break;
  }
  case EXPR_APP: {
    putchar('(');
    _print_expr(expr->u.app.func);
    putchar(' ');
    _print_expr(expr->u.app.arg);
    putchar(')');
    break;
  }
  }
}

void print_expr(const Expr *expr) {
  _print_expr(expr);
  putchar('\n');
}

void print_stack(Stack s) {
  puts("-------------------");
  puts("Stack contents:\n");
  for (ptrdiff_t i = arrlen(s) - 1; i >= 0; --i) {
    printf("  %s: ", s[i].key);
    print_expr(&s[i].value);
  }
  if (arrlen(s) == 0)
    puts("  (empty stack)");
  putchar('\n');
  puts("-------------------");
}

ptrdiff_t find_in_stack(Stack s, Variable var) {
  for (ptrdiff_t i = arrlen(s) - 1; i >= 0; --i)
    if (strcmp(s[i].key, var) == 0)
      return i;

  return -1;
}

Expr *eval(Expr *expr, Stack *s) {
  switch (expr->type) {
  case EXPR_VAR: {
    Variable var = expr->u.var;
    ptrdiff_t i = find_in_stack(*s, var);
    return (i >= 0) ? &(*s)[i].value : expr;
  }
  case EXPR_APP: {
    Expr *func = eval(expr->u.app.func, s);
    Expr *arg = eval(expr->u.app.arg, s);
    print_stack(*s);
    arrput(*s, ((Scope){.key = func->u.abs.bound_var, .value = *arg}));
    Expr *res = eval(func->u.abs.body, s);
    print_stack(*s);
    return res;
  }
  default:
    return expr;
  }
}

int main(int argc, char *argv[]) {
  Stack s = NULL;

  Variable x = "x";
  Variable y = "y";
  Expr x_expr = (Expr){.type = EXPR_VAR, .u.var = x};
  Expr y_expr = (Expr){.type = EXPR_VAR, .u.var = y};

  Expr *inner = new_abs(y, &x_expr);
  Expr *outer = new_abs(x, inner);
  print_expr(outer);

  Expr *app1 = new_app(outer, &y_expr);
  Expr *app2 = new_app(app1, &x_expr);
  print_expr(app2);
  Expr *expr = eval(app2, &s);
  print_expr(expr);

  return EXIT_SUCCESS;
}
