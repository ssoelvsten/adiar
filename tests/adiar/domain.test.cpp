#include "../test.h"

go_bandit([]() {
  domain_unset();

  describe("adiar/domain.h", []() {
    describe("domain_isset(), domain_get()", []() {
      it("throws exception when getting missing domain", []() {
        AssertThat(domain_isset(), Is().False());
        AssertThrows(domain_error, domain_get());
      });
    });

    describe("domain_isset(), domain_set(...), domain_unset()", []() {
      it("has domain after 'domain_set(file)'", []() {
        shared_file<node::label_type> dom;

        { // Garbage collect writer to free write-lock
          ofstream<ptr_uint64::label_type> lw(dom);
          lw << 1 << 2 << 3;
        }

        AssertThat(domain_isset(), Is().False());
        domain_set(dom);
        AssertThat(domain_isset(), Is().True());
      });

      it("can remove prior domain", []() {
        AssertThat(domain_isset(), Is().True());
        domain_unset();
        AssertThat(domain_isset(), Is().False());
      });

      it("can remove with no prior domain", []() {
        AssertThat(domain_isset(), Is().False());
        domain_unset();
        AssertThat(domain_isset(), Is().False());
      });

      it("has domain after 'domain_set(varcount)'", []() {
        AssertThat(domain_isset(), Is().False());
        domain_set(42);
        AssertThat(domain_isset(), Is().True());
      });
    });

    describe("domain_isset(), domain_get(), domain_set(...)", []() {
      it("gives back domain with varcount [0]", []() {
        domain_unset();
        domain_set(0);

        AssertThat(domain_isset(), Is().True());

        ifstream<node::label_type> ls(domain_get());

        AssertThat(ls.can_pull(), Is().False());
      });

      it("gives back domain with varcount [1]", []() {
        domain_unset();
        domain_set(1);

        AssertThat(domain_isset(), Is().True());

        ifstream<node::label_type> ls(domain_get());

        AssertThat(ls.can_pull(), Is().True());
        AssertThat(ls.pull(), Is().EqualTo(0u));

        AssertThat(ls.can_pull(), Is().False());
      });

      it("gives back domain with varcount [7]", []() {
        domain_unset();
        domain_set(7);

        AssertThat(domain_isset(), Is().True());

        ifstream<node::label_type> ls(domain_get());

        AssertThat(ls.can_pull(), Is().True());
        AssertThat(ls.pull(), Is().EqualTo(0u));

        AssertThat(ls.can_pull(), Is().True());
        AssertThat(ls.pull(), Is().EqualTo(1u));

        AssertThat(ls.can_pull(), Is().True());
        AssertThat(ls.pull(), Is().EqualTo(2u));

        AssertThat(ls.can_pull(), Is().True());
        AssertThat(ls.pull(), Is().EqualTo(3u));

        AssertThat(ls.can_pull(), Is().True());
        AssertThat(ls.pull(), Is().EqualTo(4u));

        AssertThat(ls.can_pull(), Is().True());
        AssertThat(ls.pull(), Is().EqualTo(5u));

        AssertThat(ls.can_pull(), Is().True());
        AssertThat(ls.pull(), Is().EqualTo(6u));

        AssertThat(ls.can_pull(), Is().False());
      });

      it("can overwrite with new varcount [2,3]", []() {
        domain_unset();

        domain_set(3);
        {
          AssertThat(domain_isset(), Is().True());

          ifstream<node::label_type> ls(domain_get());

          AssertThat(ls.can_pull(), Is().True());
          AssertThat(ls.pull(), Is().EqualTo(0u));

          AssertThat(ls.can_pull(), Is().True());
          AssertThat(ls.pull(), Is().EqualTo(1u));

          AssertThat(ls.can_pull(), Is().True());
          AssertThat(ls.pull(), Is().EqualTo(2u));

          AssertThat(ls.can_pull(), Is().False());
        }

        domain_set(2);
        {
          AssertThat(domain_isset(), Is().True());

          ifstream<node::label_type> ls(domain_get());

          AssertThat(ls.can_pull(), Is().True());
          AssertThat(ls.pull(), Is().EqualTo(0u));

          AssertThat(ls.can_pull(), Is().True());
          AssertThat(ls.pull(), Is().EqualTo(1u));

          AssertThat(ls.can_pull(), Is().False());
        }
      });

      it("can copy the Fibonacci numbers from a generator", []() {
        domain_unset();

        domain_var x = 1;
        domain_var y = 1;

        const auto gen = [&x, &y]() {
          const domain_var z = x + y;
          x                  = y;
          y                  = z;

          return x > 13 ? make_optional<domain_var>() : make_optional<domain_var>(x);
        };

        domain_set(gen);
        AssertThat(domain_isset(), Is().True());

        ifstream<node::label_type> ls(domain_get());

        AssertThat(ls.can_pull(), Is().True());
        AssertThat(ls.pull(), Is().EqualTo(1u));

        AssertThat(ls.can_pull(), Is().True());
        AssertThat(ls.pull(), Is().EqualTo(2u));

        AssertThat(ls.can_pull(), Is().True());
        AssertThat(ls.pull(), Is().EqualTo(3u));

        AssertThat(ls.can_pull(), Is().True());
        AssertThat(ls.pull(), Is().EqualTo(5u));

        AssertThat(ls.can_pull(), Is().True());
        AssertThat(ls.pull(), Is().EqualTo(8u));

        AssertThat(ls.can_pull(), Is().True());
        AssertThat(ls.pull(), Is().EqualTo(13u));

        AssertThat(ls.can_pull(), Is().False());
      });

      it("can copy from an iterator", []() {
        domain_unset();

        std::vector<int> xs = { 0, 1, 3, 5, 42 };

        domain_set(xs.begin(), xs.end());

        AssertThat(domain_isset(), Is().True());

        ifstream<node::label_type> ls(domain_get());

        AssertThat(ls.can_pull(), Is().True());
        AssertThat(ls.pull(), Is().EqualTo(0u));

        AssertThat(ls.can_pull(), Is().True());
        AssertThat(ls.pull(), Is().EqualTo(1u));

        AssertThat(ls.can_pull(), Is().True());
        AssertThat(ls.pull(), Is().EqualTo(3u));

        AssertThat(ls.can_pull(), Is().True());
        AssertThat(ls.pull(), Is().EqualTo(5u));

        AssertThat(ls.can_pull(), Is().True());
        AssertThat(ls.pull(), Is().EqualTo(42u));

        AssertThat(ls.can_pull(), Is().False());
      });

      it("can overwrite with another iterator", []() {
        domain_unset();

        std::vector<int> xs = { 2, 4, 5 };

        domain_set(xs.begin(), xs.end());
        { // Check for xs
          AssertThat(domain_isset(), Is().True());

          ifstream<node::label_type> ls(domain_get());

          AssertThat(ls.can_pull(), Is().True());
          AssertThat(ls.pull(), Is().EqualTo(2u));

          AssertThat(ls.can_pull(), Is().True());
          AssertThat(ls.pull(), Is().EqualTo(4u));

          AssertThat(ls.can_pull(), Is().True());
          AssertThat(ls.pull(), Is().EqualTo(5u));

          AssertThat(ls.can_pull(), Is().False());
        }

        std::vector<int> ys = { 0, 3 };

        domain_set(ys.begin(), ys.end());
        { // Check for ys
          AssertThat(domain_isset(), Is().True());

          ifstream<node::label_type> ls(domain_get());

          AssertThat(ls.can_pull(), Is().True());
          AssertThat(ls.pull(), Is().EqualTo(0u));

          AssertThat(ls.can_pull(), Is().True());
          AssertThat(ls.pull(), Is().EqualTo(3u));

          AssertThat(ls.can_pull(), Is().False());
        }
      });

      it("gives back the given domain file", []() {
        domain_unset();

        shared_file<node::label_type> dom;

        { // Garbage collect writer to free write-lock
          ofstream<ptr_uint64::label_type> lw(dom);
          lw << 1 << 2 << 3;
        }

        domain_set(dom);

        AssertThat(domain_isset(), Is().True());
        AssertThat(domain_get(), Is().EqualTo(dom));
      });

      it("can overwrite with another domain file", []() {
        domain_unset();

        shared_file<node::label_type> dom1;

        { // Garbage collect writer to free write-lock
          ofstream<ptr_uint64::label_type> lw(dom1);
          lw << 1 << 2 << 3;
        }

        domain_set(dom1);

        AssertThat(domain_isset(), Is().True());
        AssertThat(domain_get(), Is().EqualTo(dom1));

        shared_file<node::label_type> dom2;

        { // Garbage collect writer to free write-lock
          ofstream<ptr_uint64::label_type> lw(dom2);
          lw << 4 << 9 << 35;
        }

        domain_set(dom2);

        AssertThat(domain_isset(), Is().True());
        AssertThat(domain_get(), Is().EqualTo(dom2));
      });
    });

    describe("domain_set(), domain_unset(), domain_size()", []() {
      it("has zero-sized domain if none is set", []() {
        domain_unset();

        AssertThat(domain_isset(), Is().False());
        AssertThat(domain_size(), Is().EqualTo(0u));
      });

      it("reports domain has no elements when set to the empty domain", []() {
        domain_unset();
        AssertThat(domain_isset(), Is().False());

        domain_set(0);
        AssertThat(domain_isset(), Is().True());

        AssertThat(domain_size(), Is().EqualTo(0u));
      });

      it("can report having a domain of 1 element", []() {
        domain_unset();
        AssertThat(domain_isset(), Is().False());

        domain_set(1);
        AssertThat(domain_isset(), Is().True());

        AssertThat(domain_size(), Is().EqualTo(1u));
      });

      it("can report having a domain of 42 element", []() {
        domain_unset();
        AssertThat(domain_isset(), Is().False());

        domain_set(42);
        AssertThat(domain_isset(), Is().True());

        AssertThat(domain_size(), Is().EqualTo(42u));
      });
    });

    // TODO: more tests when 'https://github.com/thomasmoelhave/tpie/issues/265'
    //       is resolved.
  });
});
