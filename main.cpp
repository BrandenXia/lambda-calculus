#include <format>
#include <iostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>

using Variable = std::string_view;
struct Abstraction;
struct Application;

using Expr = std::variant<Abstraction, Application, Variable>;

struct Abstraction {
  Variable bound_var;
  Expr &body;
};

struct Application {
  Expr &func;
  Expr &arg;
};

using Scope = std::unordered_map<Variable, Expr>;

std::string _to_string(const Expr &expr) {
  return std::visit(
      [](const auto &e) -> std::string {
        using T = std::decay_t<decltype(e)>;
        if constexpr (std::is_same_v<T, Variable>) {
          return std::string(e);
        } else if constexpr (std::is_same_v<T, Abstraction>) {
          return std::format("(λ{} . {})", e.bound_var, _to_string(e.body));
        } else if constexpr (std::is_same_v<T, Application>) {
          return std::format("({} {})", _to_string(e.func), _to_string(e.arg));
        } else {
          return "Unknown Expr type";
        }
      },
      expr);
}

template <> struct std::formatter<Expr> : std::formatter<std::string> {
  // This is a custom formatter for Expr to use std::format
  template <typename FormatContext>
  auto format(const Expr &expr, FormatContext &ctx) const {
    return std::formatter<std::string>::format(_to_string(expr), ctx);
  }
};

Expr eval(const Expr &expr, Scope scope) {
  return std::visit(
      [&scope](auto &&e) {
        using T = std::decay_t<decltype(e)>;
        using std::is_same_v;

        if constexpr (is_same_v<T, Application>) {
          const auto &func = eval(e.func, scope);
          const auto &arg = eval(e.arg, scope);
          const auto &abstraction = std::get<Abstraction>(func);
          scope.insert({abstraction.bound_var, arg});
          return eval(abstraction.body, scope);
        }

        return Expr{e};
      },
      expr);
}

int main(int argc, char *argv[]) {
  Expr x = Variable{"x"};
  Expr y = Variable{"y"};
  Expr _inner = Abstraction{"y", x};
  Expr true_var = Abstraction{"x", _inner};

  std::cout << std::format("true_var: {}\n", true_var);

  Expr tmp = Application{true_var, x};
  Expr test = Application{tmp, y};
  Expr res = eval(test, Scope{{"x", x}, {"y", y}});
  std::cout << std::format("test: {}\n", res);

  return 0;
}
