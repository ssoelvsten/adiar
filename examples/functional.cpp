#include <iostream>
#include <string>

#include <adiar/adiar.h>

void print(const adiar::bdd &f, const std::string &s)
{
  std::cout << "Example: " << s << "\n";
  std::cout << " | adiar::bdd_nodecount(f) = " << adiar::bdd_nodecount(f) << "\n";
  std::cout << " | adiar::bdd_varcount(f)  = " << adiar::bdd_varcount(f) << "\n";
  std::cout << " | adiar::bdd_satcount(f)  = " << adiar::bdd_satcount(f) << "\n";
  std::cout << "\n";
}

void predicates(const adiar::bdd &f)
{
  /// [is_odd]
  const adiar::predicate<int> is_odd = [](int x) -> bool
  {
    return x % 2;
  };
  /// [is_odd]

  /// [predicate]
  adiar::bdd _ = adiar::bdd_exists(f, is_odd);
  /// [predicate]

  print(_, "Predicates");
}

void iterators_1(const adiar::bdd &f)
{
  /// [iterators_1]
  std::vector<int> xs = { 5, 3, 1 };

  adiar::bdd _ = adiar::bdd_exists(f, xs.begin(), xs.end());
  /// [iterators_1]

  print(_, "Iterators[1]");
}

void iterators_2(const adiar::bdd &f)
{
  /// [iterators_2]
  std::vector<int> xs;

  adiar::bdd_support(f, std::back_inserter(xs));
  /// [iterators_2]

  std::cout << "Example: " << "Iterators[2]" << "\n";
  std::cout << " | ";
  for (int x : xs) { std::cout << x << " "; }
  std::cout << "\n\n";
}

void generators(const adiar::bdd &f)
{
  /// [generators]
  const auto gen = [x = 7]() mutable -> adiar::optional<int>
    {
      // If x < 0, we are done.
      if (x < 0) { return {}; }
      // Otherwise, return x-2.
      return {x -= 2};
    };

  adiar::bdd _ = adiar::bdd_exists(f, gen);
  /// [generators]

  print(_, "Generators");
}

void consumers(const adiar::bdd &f)
{
  std::cout << "Example: " << "Consumers" << "\n";
  std::cout << " | ";

  /// [consumers]
  const auto con = [](int x) -> void
  {
    std::cout << x << " ";
  };
  adiar::bdd_support(f, con);
  /// [consumers]

  std::cout << "\n";
}

int main()
{
  adiar::adiar_init(adiar::minimum_memory);

  {
    // Create an 'f' using the BDD builder (see also 'builder.cpp')
    adiar::bdd_builder b;

    const adiar::bdd_ptr p2 = b.add_node(2, false, true);
    const adiar::bdd_ptr p1 = b.add_node(1, p2, true);
    const adiar::bdd_ptr p0 = b.add_node(0, p2, p1);

    adiar::bdd f = b.build();

    predicates(f);
    iterators_1(f);
    iterators_2(f);
    generators(f);
  }

  adiar::adiar_deinit();
  return 0;
}
