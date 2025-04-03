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
} *Scope;

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

Expr *eval(Expr *expr, Scope s) {
  switch (expr->type) {
  case EXPR_VAR: {
    Variable var = expr->u.var;
    ptrdiff_t index = hmgeti(s, var);
    return index != -1 ? &s[index].value : NULL;
  }
  case EXPR_APP: {
    Expr *func = eval(expr->u.app.func, s);
    Expr *arg = eval(expr->u.app.arg, s);
    hmput(s, func->u.abs.bound_var, *arg);
    return eval(func->u.abs.body, s);
  }
  default:
    return expr;
  }
}

int main(int argc, char *argv[]) {
  Scope s = NULL;

  Variable x = "x";
  Variable y = "y";
  Expr x_expr = (Expr){.type = EXPR_VAR, .u.var = x};
  Expr y_expr = (Expr){.type = EXPR_VAR, .u.var = y};
  hmput(s, x, x_expr);
  hmput(s, y, y_expr);

  Expr *inner = new_abs(y, &x_expr);
  Expr *outer = new_abs(x, inner);
  print_expr(outer);

  Expr *app1 = new_app(outer, &y_expr);
  Expr *app2 = new_app(app1, &x_expr);
  print_expr(app2);
  Expr *expr = eval(app2, s);
  print_expr(expr);

  return EXIT_SUCCESS;
}
