#include "../test.h"

go_bandit([]() {
  describe("adiar/exec_policy.h", []() {
    describe("exec_policy", []() {
      it("uses expected number of bytes",
         []() { AssertThat(sizeof(exec_policy), Is().EqualTo(12u)); });

      const float default__transposition_growth = exec_policy::quantify::transposition_growth();
      const unsigned char default__transposition_max = exec_policy::quantify::transposition_max();

      describe("exec_policy::_::number wrapper", [&]() {
        it("'quantify::transposition growth' defaults to a non-negative value",
           [&]() { AssertThat(default__transposition_growth, Is().GreaterThanOrEqualTo(0.0f)); });

        it("'quantify::transposition growth' truncates negative values to '0.0f'", [&]() {
          const exec_policy::quantify::transposition_growth res(-0.5f);
          AssertThat(static_cast<float>(res), Is().GreaterThanOrEqualTo(0.0f));
        });
      });

      describe("exec_policy(const __ &)", [&]() {
        it("is default constructed with default settings", [&]() {
          exec_policy ep;

          AssertThat(ep.template get<exec_policy::access>(),
                     Is().EqualTo(exec_policy::access::Auto));

          AssertThat(ep.template get<exec_policy::memory>(),
                     Is().EqualTo(exec_policy::memory::Auto));

          AssertThat(ep.template get<exec_policy::quantify::algorithm>(),
                     Is().EqualTo(exec_policy::quantify::Nested));
          AssertThat(
            static_cast<float>(ep.template get<exec_policy::quantify::transposition_growth>()),
            Is().EqualTo(default__transposition_growth));
          AssertThat(
            static_cast<unsigned char>(ep.template get<exec_policy::quantify::transposition_max>()),
            Is().EqualTo(default__transposition_max));
        });

        it("can be conversion constructed from 'access mode'", [&]() {
          exec_policy ep = exec_policy::access::Priority_Queue;

          AssertThat(ep.template get<exec_policy::access>(),
                     Is().EqualTo(exec_policy::access::Priority_Queue));
          AssertThat(ep.template get<exec_policy::memory>(),
                     Is().EqualTo(exec_policy::memory::Auto));

          AssertThat(ep.template get<exec_policy::quantify::algorithm>(),
                     Is().EqualTo(exec_policy::quantify::Nested));
          AssertThat(
            static_cast<float>(ep.template get<exec_policy::quantify::transposition_growth>()),
            Is().EqualTo(default__transposition_growth));
          AssertThat(
            static_cast<unsigned char>(ep.template get<exec_policy::quantify::transposition_max>()),
            Is().EqualTo(default__transposition_max));
        });

        it("can be conversion constructed from 'memory mode'", [&]() {
          exec_policy ep = exec_policy::memory::Internal;

          AssertThat(ep.template get<exec_policy::access>(),
                     Is().EqualTo(exec_policy::access::Auto));
          AssertThat(ep.template get<exec_policy::memory>(),
                     Is().EqualTo(exec_policy::memory::Internal));

          AssertThat(ep.template get<exec_policy::quantify::algorithm>(),
                     Is().EqualTo(exec_policy::quantify::Nested));
          AssertThat(
            static_cast<float>(ep.template get<exec_policy::quantify::transposition_growth>()),
            Is().EqualTo(default__transposition_growth));
          AssertThat(
            static_cast<unsigned char>(ep.template get<exec_policy::quantify::transposition_max>()),
            Is().EqualTo(default__transposition_max));
        });

        it("can be conversion constructed from 'quantify::algorithm'", [&]() {
          exec_policy ep = exec_policy::quantify::Singleton;

          AssertThat(ep.template get<exec_policy::access>(),
                     Is().EqualTo(exec_policy::access::Auto));
          AssertThat(ep.template get<exec_policy::memory>(),
                     Is().EqualTo(exec_policy::memory::Auto));

          AssertThat(ep.template get<exec_policy::quantify::algorithm>(),
                     Is().EqualTo(exec_policy::quantify::Singleton));
          AssertThat(
            static_cast<float>(ep.template get<exec_policy::quantify::transposition_growth>()),
            Is().EqualTo(default__transposition_growth));
          AssertThat(
            static_cast<unsigned char>(ep.template get<exec_policy::quantify::transposition_max>()),
            Is().EqualTo(default__transposition_max));
        });

        it("can be conversion constructed from 'quantify::transposition growth'", [&]() {
          exec_policy ep = exec_policy::quantify::transposition_growth(4.2f);

          AssertThat(ep.template get<exec_policy::access>(),
                     Is().EqualTo(exec_policy::access::Auto));
          AssertThat(ep.template get<exec_policy::memory>(),
                     Is().EqualTo(exec_policy::memory::Auto));

          AssertThat(ep.template get<exec_policy::quantify::algorithm>(),
                     Is().EqualTo(exec_policy::quantify::Nested));
          AssertThat(
            static_cast<float>(ep.template get<exec_policy::quantify::transposition_growth>()),
            Is().EqualTo(4.2f));
          AssertThat(
            static_cast<unsigned char>(ep.template get<exec_policy::quantify::transposition_max>()),
            Is().EqualTo(default__transposition_max));
        });

        it("can be conversion constructed from 'quantify::transposition max'", [&]() {
          exec_policy ep = exec_policy::quantify::transposition_max(42);

          AssertThat(ep.template get<exec_policy::access>(),
                     Is().EqualTo(exec_policy::access::Auto));
          AssertThat(ep.template get<exec_policy::memory>(),
                     Is().EqualTo(exec_policy::memory::Auto));

          AssertThat(ep.template get<exec_policy::quantify::algorithm>(),
                     Is().EqualTo(exec_policy::quantify::Nested));
          AssertThat(
            static_cast<float>(ep.template get<exec_policy::quantify::transposition_growth>()),
            Is().EqualTo(default__transposition_growth));
          AssertThat(
            static_cast<unsigned char>(ep.template get<exec_policy::quantify::transposition_max>()),
            Is().EqualTo(42));
        });
      });

      describe("set(const __ &)", [&]() {
        it("can set 'access mode'", [&]() {
          exec_policy ep;
          AssertThat(ep.template get<exec_policy::access>(),
                     Is().EqualTo(exec_policy::access::Auto));

          ep.set(exec_policy::access::Random_Access);
          AssertThat(ep.template get<exec_policy::access>(),
                     Is().EqualTo(exec_policy::access::Random_Access));

          ep.set(exec_policy::access::Priority_Queue);
          AssertThat(ep.template get<exec_policy::access>(),
                     Is().EqualTo(exec_policy::access::Priority_Queue));

          ep.set(exec_policy::access::Auto);
          AssertThat(ep.template get<exec_policy::access>(),
                     Is().EqualTo(exec_policy::access::Auto));
        });

        it("can set 'memory mode'", [&]() {
          exec_policy ep;
          AssertThat(ep.template get<exec_policy::memory>(),
                     Is().EqualTo(exec_policy::memory::Auto));

          ep.set(exec_policy::memory::Internal);
          AssertThat(ep.template get<exec_policy::memory>(),
                     Is().EqualTo(exec_policy::memory::Internal));

          ep.set(exec_policy::memory::External);
          AssertThat(ep.template get<exec_policy::memory>(),
                     Is().EqualTo(exec_policy::memory::External));

          ep.set(exec_policy::memory::Auto);
          AssertThat(ep.template get<exec_policy::memory>(),
                     Is().EqualTo(exec_policy::memory::Auto));
        });

        it("can set 'quantify algorithm'", [&]() {
          exec_policy ep;
          AssertThat(ep.template get<exec_policy::quantify::algorithm>(),
                     Is().EqualTo(exec_policy::quantify::Nested));

          ep.set(exec_policy::quantify::Singleton);
          AssertThat(ep.template get<exec_policy::quantify::algorithm>(),
                     Is().EqualTo(exec_policy::quantify::Singleton));

          ep.set(exec_policy::quantify::Nested);
          AssertThat(ep.template get<exec_policy::quantify::algorithm>(),
                     Is().EqualTo(exec_policy::quantify::Nested));
        });

        it("can set 'quantify::transposition growth epsilon'", [&]() {
          exec_policy ep;
          AssertThat(
            static_cast<float>(ep.template get<exec_policy::quantify::transposition_growth>()),
            Is().EqualTo(default__transposition_growth));

          ep.set(exec_policy::quantify::transposition_growth(4.2));
          AssertThat(
            static_cast<float>(ep.template get<exec_policy::quantify::transposition_growth>()),
            Is().EqualTo(4.2f));
        });

        /* TODO
        it("throws when setting 'quantify::transposition growth epsilon' to a negative value", []()
        { exec_policy ep; AssertThrows(invalid_argument,
        ep.set(exec_policy::quantify::transposition_growth(-42.0)));
        });
        */

        it("can set 'quantify::transposition max delta'", [&]() {
          exec_policy ep;
          AssertThat(
            static_cast<unsigned char>(ep.template get<exec_policy::quantify::transposition_max>()),
            Is().EqualTo(default__transposition_max));

          ep.set(exec_policy::quantify::transposition_max(42));
          AssertThat(
            static_cast<unsigned char>(ep.template get<exec_policy::quantify::transposition_max>()),
            Is().EqualTo(42));
        });

        it("can set settigs with a builder pattern syntax", [&]() {
          exec_policy ep;

          ep.set(exec_policy::access::Priority_Queue)
            .set(exec_policy::memory::External)
            .set(exec_policy::quantify::Singleton)
            .set(exec_policy::quantify::transposition_growth(4.2f))
            .set(exec_policy::quantify::transposition_max(42));

          AssertThat(ep.template get<exec_policy::access>(),
                     Is().EqualTo(exec_policy::access::Priority_Queue));

          AssertThat(ep.template get<exec_policy::memory>(),
                     Is().EqualTo(exec_policy::memory::External));

          AssertThat(ep.template get<exec_policy::quantify::algorithm>(),
                     Is().EqualTo(exec_policy::quantify::Singleton));
          AssertThat(
            static_cast<float>(ep.template get<exec_policy::quantify::transposition_growth>()),
            Is().EqualTo(4.2f));
          AssertThat(
            static_cast<unsigned char>(ep.template get<exec_policy::quantify::transposition_max>()),
            Is().EqualTo(42));
        });
      });

      describe("operator ==(const exec_policy&)", [&]() {
        it("matches for default settings", [&]() {
          exec_policy ep1;
          exec_policy ep2;

          AssertThat(ep1, Is().EqualTo(ep2));
        });

        it("matches for (non-default) Adiar v1.0 settings", [&]() {
          exec_policy ep1;
          ep1.set(exec_policy::quantify::Singleton)
            .set(exec_policy::memory::External)
            .set(exec_policy::access::Priority_Queue);

          exec_policy ep2;
          ep2.set(exec_policy::access::Priority_Queue)
            .set(exec_policy::memory::External)
            .set(exec_policy::quantify::Singleton);

          AssertThat(ep1, Is().EqualTo(ep2));
        });

        it("mismatches on 'access mode'", [&]() {
          exec_policy ep1 = exec_policy::access::Priority_Queue;
          exec_policy ep2 = exec_policy::access::Random_Access;

          AssertThat(ep1, Is().Not().EqualTo(ep2));
        });

        it("mismatches on 'memory mode'", [&]() {
          exec_policy ep1 = exec_policy::memory::Internal;
          exec_policy ep2 = exec_policy::memory::Auto;

          AssertThat(ep1, Is().Not().EqualTo(ep2));
        });

        it("mismatches on 'quantify::algorithm'", [&]() {
          exec_policy ep1 = exec_policy::quantify::Nested;
          exec_policy ep2 = exec_policy::quantify::Singleton;

          AssertThat(ep1, Is().Not().EqualTo(ep2));
        });

        it("mismatches on 'quantify::transposition growth'", [&]() {
          exec_policy ep1 = exec_policy::quantify::transposition_growth(4.2f);
          exec_policy ep2 = exec_policy::quantify::transposition_growth(2.1f);

          AssertThat(ep1, Is().Not().EqualTo(ep2));
        });

        it("mismatches on 'quantify::transposition max'", [&]() {
          exec_policy ep1 = exec_policy::quantify::transposition_max(32);
          exec_policy ep2 = exec_policy::quantify::transposition_max(12);

          AssertThat(ep1, Is().Not().EqualTo(ep2));
        });
      });

      describe("operator &(const exec_policy&)", [&]() {
        it("can create a copy with another 'access mode'", [&]() {
          const exec_policy in  = exec_policy::memory::Internal;
          const exec_policy out = in & exec_policy::access::Random_Access;

          AssertThat(in.template get<exec_policy::access>(),
                     Is().EqualTo(exec_policy::access::Auto));

          AssertThat(out.template get<exec_policy::access>(),
                     Is().EqualTo(exec_policy::access::Random_Access));
        });

        it("can create a copy with another 'memory mode'", [&]() {
          const exec_policy in  = exec_policy::access::Priority_Queue;
          const exec_policy out = in & exec_policy::memory::Internal;

          AssertThat(in.template get<exec_policy::memory>(),
                     Is().EqualTo(exec_policy::memory::Auto));

          AssertThat(out.template get<exec_policy::memory>(),
                     Is().EqualTo(exec_policy::memory::Internal));
        });

        it("can create a copy with another 'quantify::*'", [&]() {
          const exec_policy in  = exec_policy::memory::Internal;
          const exec_policy out = in & exec_policy::quantify::Singleton
            & exec_policy::quantify::transposition_growth(4.2f)
            & exec_policy::quantify::transposition_max(42);

          AssertThat(in.template get<exec_policy::quantify::algorithm>(),
                     Is().EqualTo(exec_policy::quantify::Nested));
          AssertThat(
            static_cast<float>(in.template get<exec_policy::quantify::transposition_growth>()),
            Is().EqualTo(default__transposition_growth));
          AssertThat(
            static_cast<unsigned char>(in.template get<exec_policy::quantify::transposition_max>()),
            Is().EqualTo(default__transposition_max));

          AssertThat(out.template get<exec_policy::quantify::algorithm>(),
                     Is().EqualTo(exec_policy::quantify::Singleton));
          AssertThat(
            static_cast<float>(out.template get<exec_policy::quantify::transposition_growth>()),
            Is().EqualTo(4.2f));
          AssertThat(static_cast<unsigned char>(
                       out.template get<exec_policy::quantify::transposition_max>()),
                     Is().EqualTo(42));
        });

        it("can lift enum values [access]", [&]() {
          const exec_policy ep = exec_policy::access::Random_Access & exec_policy::memory::Internal;

          AssertThat(ep.template get<exec_policy::access>(),
                     Is().EqualTo(exec_policy::access::Random_Access));
          AssertThat(ep.template get<exec_policy::memory>(),
                     Is().EqualTo(exec_policy::memory::Internal));
          AssertThat(ep.template get<exec_policy::quantify::algorithm>(),
                     Is().EqualTo(exec_policy::quantify::Nested));
        });

        it("can lift enum values [memory]", [&]() {
          const exec_policy ep = exec_policy::memory::Internal & exec_policy::access::Random_Access;

          AssertThat(ep.template get<exec_policy::access>(),
                     Is().EqualTo(exec_policy::access::Random_Access));
          AssertThat(ep.template get<exec_policy::memory>(),
                     Is().EqualTo(exec_policy::memory::Internal));
          AssertThat(ep.template get<exec_policy::quantify::algorithm>(),
                     Is().EqualTo(exec_policy::quantify::Nested));
        });

        it("can lift enum values [quantify::*]", [&]() {
          const exec_policy ep = exec_policy::quantify::Nested
            & exec_policy::quantify::transposition_growth(4.2)
            & exec_policy::quantify::transposition_max(42);

          AssertThat(ep.template get<exec_policy::access>(),
                     Is().EqualTo(exec_policy::access::Auto));

          AssertThat(ep.template get<exec_policy::memory>(),
                     Is().EqualTo(exec_policy::memory::Auto));

          AssertThat(ep.template get<exec_policy::quantify::algorithm>(),
                     Is().EqualTo(exec_policy::quantify::Nested));
          AssertThat(
            static_cast<float>(ep.template get<exec_policy::quantify::transposition_growth>()),
            Is().EqualTo(4.2f));
          AssertThat(
            static_cast<unsigned char>(ep.template get<exec_policy::quantify::transposition_max>()),
            Is().EqualTo(42));
        });
      });
    });
  });
});
