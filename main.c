#include <stdio.h>
#include <stdlib.h>

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

typedef char *Variable;
typedef struct Abstraction Abstraction;
typedef struct Application Application;

typedef enum { EXPR_VAR, EXPR_ABS, EXPR_APP } ExprType;

typedef struct Expr {
  ExprType type;
  union {
    Variable var;
    Abstraction *abs;
    Application *app;
  } u;
} Expr;

struct Abstraction {
  Variable bound_var;
  Expr *body;
};

struct Application {
  Expr *func;
  Expr *arg;
};

typedef struct {
  Variable key;
  Expr value;
} *Scope;

#define EXPR(e)                                                                \
  _Generic((e),                                                                \
      Variable: ((Expr){EXPR_VAR, .u.var = (e)}),                              \
      Abstraction *: ((Expr){EXPR_ABS, .u.abs = (e)}),                         \
      Application *: ((Expr){EXPR_APP, .u.app = (e)}),                         \
      default: ({                                                              \
             fprintf(stderr, "Error: Invalid type for EXPR\n");                \
             exit(EXIT_FAILURE);                                               \
           }))

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
    printf("(λ%s . ", expr->u.abs->bound_var);
    _print_expr(expr->u.abs->body);
    putchar(')');
    break;
  }
  case EXPR_APP: {
    putchar('(');
    _print_expr(expr->u.app->func);
    putchar(' ');
    _print_expr(expr->u.app->arg);
    putchar(')');
    break;
  }
  }
}

void print_expr(const Expr *expr) {
  _print_expr(expr);
  putchar('\n');
}

Expr eval(Expr expr, Scope s) {
  switch (expr.type) {
  case EXPR_VAR: {
    Variable var = expr.u.var;
    return hmget(s, var);
  }
  case EXPR_APP: {
    Expr func = eval(*(expr.u.app->func), s);
    Expr arg = eval(*(expr.u.app->arg), s);
    hmput(s, func.u.abs->bound_var, arg);
    return eval(*(func.u.abs->body), s);
  }
  default:
    return expr;
  }
}

int main(int argc, char *argv[]) {
  Scope s = NULL;

  Variable x = "x";
  Variable y = "y";
  Expr x_expr = EXPR(x);
  Expr y_expr = EXPR(y);
  hmput(s, x, x_expr);
  hmput(s, y, y_expr);

  Abstraction inner = {.bound_var = y, .body = &x_expr};
  Expr inner_expr = EXPR(&inner);
  Abstraction outer = {.bound_var = x, .body = &inner_expr};
  Expr outer_expr = EXPR(&outer);
  print_expr(&outer_expr);

  Application app = {.func = &outer_expr, .arg = &y_expr};
  Expr app_expr = EXPR(&app);
  Application app_eval = {.func = &app_expr, .arg = &x_expr};
  Expr test_expr = EXPR(&app_eval);
  print_expr(&test_expr);
  Expr expr = eval(test_expr, s);
  print_expr(&expr);

  return EXIT_SUCCESS;
}
