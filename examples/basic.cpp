#include <iostream>
#include <string>

#include <adiar/adiar.h>

void print(const bool b, const std::string &s)
{
  std::cout << "Example: " << s << "\n";
  std::cout << " | " << (b ? "true" : "false") << "\n";
  std::cout << "\n";
}

void print(const size_t i, const std::string &s)
{
  std::cout << "Example: " << s << "\n";
  std::cout << " | " << i << "\n";
  std::cout << "\n";
}

void print(const adiar::bdd &f, const std::string &s)
{
  std::cout << "Example: " << s << "\n";
  if (adiar::bdd_isconst(f)) {
    std::cout << " | adiar::bdd_isconst(f)     = true\n";
    std::cout << " |\n";
  } else {
    std::cout << " | adiar::bdd_topvar(f)      = " << adiar::bdd_topvar(f) << "\n";
    std::cout << " |\n";
    std::cout << " | adiar::bdd_nodecount(f)   = " << adiar::bdd_nodecount(f) << "\n";
    std::cout << " | adiar::bdd_varcount(f)    = " << adiar::bdd_varcount(f) << "\n";
  }
  std::cout << " | adiar::bdd_satcount(f, 3) = " << adiar::bdd_satcount(f, 3) << "\n";
  std::cout << "\n";
}

adiar::bdd constants_top()
{
  /// [constants.top]
  adiar::bdd top = adiar::bdd_true();
  /// [constants.top]
  print(top, "Constants [top]");
  return top;
}

adiar::bdd constants_bot()
{
  /// [constants.bot]
  adiar::bdd bot = false;
  /// [constants.bot]
  print(bot, "Constants [bot]");
  return bot;
}

adiar::bdd ithvar_x0()
{
  /// [ithvar.0]
  adiar::bdd x0 = adiar::bdd_ithvar(0);
  /// [ithvar.0]
  print(x0, "Variables [x0]");
  return x0;
}

adiar::bdd ithvar_x1()
{
  /// [ithvar.1]
  adiar::bdd x1 = adiar::bdd_ithvar(1);
  /// [ithvar.1]
  print(x1, "Variables [x1]");
  return x1;
}

adiar::bdd ithvar_x2()
{
  /// [ithvar.2]
  adiar::bdd x2 = adiar::bdd_ithvar(2);
  /// [ithvar.2]
  print(x2, "Variables [x2]");
  return x2;
}

void not_1(const adiar::bdd &x0)
{
  /// [not.1]
  adiar::bdd _ = adiar::bdd_not(x0);
  /// [not.1]
  print(_, "Operation [bdd_not]");
}

void not_2(const adiar::bdd &x0)
{
  /// [not.2]
  adiar::bdd _ = x0;
  /// [not.2]
  print(_, "Operation [~]");
}

adiar::bdd apply_1(const adiar::bdd &x0, const adiar::bdd &x1)
{
  /// [apply.1]
  adiar::bdd f = adiar::bdd_apply(x0, x1, adiar::xor_op);
  /// [apply.1]
  print(f, "Operation [bdd_apply(..., xor_op)]");
  return f;
}

void apply_2(const adiar::bdd &x0, const adiar::bdd &x1)
{
  /// [apply.2]
  adiar::bdd f = adiar::bdd_xor(x0, x1);
  /// [apply.2]
  print(f, "Operation [bdd_xor(...)]");
}

void apply_3(const adiar::bdd &x0, const adiar::bdd &x1)
{
  /// [apply.3]
  adiar::bdd f = x0 ^ x1;
  /// [apply.3]
  print(f, "Operation [^]");
}

void apply_4(const adiar::bdd &bot, const adiar::bdd &x0, const adiar::bdd &x1)
{
  /// [apply.4]
  adiar::bdd f = bot;
  f ^= x0;
  f ^= x1;
  /// [apply.4]
  print(f, "Operation [^=]");
}

adiar::bdd ite(const adiar::bdd &x0, const adiar::bdd &x1, const adiar::bdd &x2)
{
  /// [ite]
  adiar::bdd g = adiar::bdd_ite(x0, x1, x2);
  /// [ite]
  print(g, "Operation [bdd_ite]");
  return g;
}

void restrict(const adiar::bdd &f)
{
  /// [restrict]
  int i = 1;
  adiar::bdd _ = adiar::bdd_restrict(f, i, true); // ~x0
  /// [restrict]
  print(_, "Operation [bdd_resrict]");
}

void exists(const adiar::bdd &f)
{
  /// [exists]
  int i = 1;
  adiar::bdd _ = adiar::bdd_exists(f, i); // true
  /// [exists]
  print(_, "Operation [bdd_exists]");
}

void satcount(const adiar::bdd &f)
{
  /// [satcount]
  size_t varcount = 3;
  size_t _ = adiar::bdd_satcount(f, varcount); // 4
  /// [satcount]
  print(_, "Satisfying [bdd_satcount]");
}

adiar::bdd satmin(const adiar::bdd &f)
{
  /// [satmin]
  adiar::bdd f_min = adiar::bdd_satmin(f); // ~i & j
  /// [satmin]
  print(f_min, "Satisfying [bdd_satmin]");
  return f_min;
}

adiar::bdd satmax(const adiar::bdd &f)
{
  /// [satmax]
  adiar::bdd f_max = adiar::bdd_satmax(f); // i & ~j
  /// [satmax]
  print(f_max, "Satisfying [bdd_satmax]");
  return f_max;
}

void isconst_top(const adiar::bdd &top)
{
  /// [isconst.top]
  bool _ = adiar::bdd_isconst(top); // true
  /// [isconst.top]
  print(_, "Predicates [bdd_isconst(top)]");
}

void isconst_bot(const adiar::bdd &bot)
{
  /// [isconst.bot]
  bool _ = adiar::bdd_isconst(bot); // true
  /// [isconst.bot]
  print(_, "Predicates [bdd_isconst(bot)]");
}

void isconst_x0(const adiar::bdd &x0)
{
  /// [isconst.x0]
  bool _ = adiar::bdd_isconst(x0); // false
  /// [isconst.x0]
  print(_, "Predicates [bdd_isconst(x0)]");
}

void isvar_top(const adiar::bdd &top)
{
  /// [isvar.top]
  bool _ = adiar::bdd_isvar(top); // false
  /// [isvar.top]
  print(_, "Predicates [bdd_isvar(top)]");
}

void isvar_x0(const adiar::bdd &x0)
{
  /// [isvar.x0]
  bool _ = adiar::bdd_isvar(x0); // true
  /// [isvar.x0]
  print(_, "Predicates [bdd_isvar(x0)]");
}

void isvar_not_x0(const adiar::bdd &x0)
{
  /// [isvar.~x0]
  bool _ = adiar::bdd_isvar(~x0); // true
  /// [isvar.~x0]
  print(_, "Predicates [bdd_isvar(~x0)]");
}

void iscube_f(const adiar::bdd &f)
{
  /// [iscube.f]
  bool _ = adiar::bdd_iscube(f); // false
  /// [iscube.f]
  print(_, "Predicates [bdd_iscube(f)]");
}

void iscube_fmin(const adiar::bdd &f_min)
{
  /// [iscube.f_min]
  bool _ = adiar::bdd_iscube(f_min); // true
  /// [iscube.f_min]
  print(_, "Predicates [bdd_iscube(f_min)]");
}

void equal_1(const adiar::bdd &f, const adiar::bdd &g)
{
  /// [equal.1]
  bool _ = adiar::bdd_equal(f, g); // false
  /// [equal.1]
  print(_, "Predicates [bdd_equal(f, g)]");
}

void equal_2(const adiar::bdd &x0, const adiar::bdd &x1, const adiar::bdd &f)
{
  /// [equal.2]
  bool _ = f == (x1 ^ x0); // true
  /// [equal.2]
  print(_, "Predicates [f == (x1 ^ x0)]");
}

void nodecount(const adiar::bdd &f)
{
  /// [nodecount]
  size_t _ = adiar::bdd_nodecount(f); // 3
  /// [nodecount]
  print(_, "Info [nodecount(f)]");
}

void varcount(const adiar::bdd &f)
{
  /// [varcount]
  size_t _ = adiar::bdd_varcount(f); // 2
  /// [varcount]
  print(_, "Info [varcount(f)]");
}

void pathcount(const adiar::bdd &f)
{
  /// [pathcount]
  size_t _ = adiar::bdd_pathcount(f); // 2
  /// [pathcount]
  print(_, "Info [pathcount(f)]");
}

void topvar(const adiar::bdd &f)
{
  /// [topvar]
  size_t _ = adiar::bdd_topvar(f); // 0
  /// [topvar]
  print(_, "Info [topvar(f)]");
}

void maxvar(const adiar::bdd &f)
{
  /// [maxvar]
  size_t _ = adiar::bdd_maxvar(f); // 1
  /// [maxvar]
  print(_, "Info [maxvar(f)]");
}

void dot(const adiar::bdd &f)
{
  /// [dot]
  adiar::bdd_printdot(f, "./f.dot");
  /// [dot]
}

int main() {
  adiar::adiar_init(adiar::minimum_memory);

  {
    const adiar::bdd top   = constants_top();
    const adiar::bdd bot   = constants_bot();
    const adiar::bdd x0    = ithvar_x0();
    const adiar::bdd x1    = ithvar_x1();
    const adiar::bdd x2    = ithvar_x2();
    not_1(x0);
    not_2(x0);
    const adiar::bdd f     = apply_1(x0, x1);
    apply_2(x0, x1);
    apply_3(x0, x1);
    apply_4(bot, x0, x1);
    const adiar::bdd g     = ite(x0, x1, x2);
    restrict(f);
    exists(f);
    satcount(f);
    const adiar::bdd f_min = satmin(f);
    const adiar::bdd f_max = satmax(f);
    isconst_top(top);
    isconst_bot(bot);
    isconst_x0(x0);
    isvar_top(top);
    isvar_x0(x0);
    isvar_not_x0(x0);
    iscube_f(f);
    iscube_fmin(f_min);
    equal_1(f, g);
    equal_2(x0, x1, f);
    nodecount(f);
    varcount(f);
    pathcount(f);
    topvar(f);
    maxvar(f);
    dot(f);
  }

  adiar::adiar_deinit();

  return 0;
}
