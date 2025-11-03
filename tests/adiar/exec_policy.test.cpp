#include "../test.h"

go_bandit([]() {
  describe("adiar/exec_policy.h", []() {
    describe("exec_policy", []() {
      it("uses expected number of bytes",
         []() { AssertThat(sizeof(exec_policy), Is().EqualTo(3u)); });

      describe("exec_policy(const __ &)", [&]() {
        it("is default constructed with default settings", [&]() {
          exec_policy ep;

          AssertThat(ep.template get<exec_policy::access>(),
                     Is().EqualTo(exec_policy::access::Auto));

          AssertThat(ep.template get<exec_policy::memory>(),
                     Is().EqualTo(exec_policy::memory::Auto));

          AssertThat(ep.template get<exec_policy::quantify::algorithm>(),
                     Is().EqualTo(exec_policy::quantify::Nested));
        });

        it("can be conversion constructed from 'access mode'", [&]() {
          exec_policy ep = exec_policy::access::Priority_Queue;

          AssertThat(ep.template get<exec_policy::access>(),
                     Is().EqualTo(exec_policy::access::Priority_Queue));
          AssertThat(ep.template get<exec_policy::memory>(),
                     Is().EqualTo(exec_policy::memory::Auto));

          AssertThat(ep.template get<exec_policy::quantify::algorithm>(),
                     Is().EqualTo(exec_policy::quantify::Nested));
        });

        it("can be conversion constructed from 'memory mode'", [&]() {
          exec_policy ep = exec_policy::memory::Internal;

          AssertThat(ep.template get<exec_policy::access>(),
                     Is().EqualTo(exec_policy::access::Auto));
          AssertThat(ep.template get<exec_policy::memory>(),
                     Is().EqualTo(exec_policy::memory::Internal));

          AssertThat(ep.template get<exec_policy::quantify::algorithm>(),
                     Is().EqualTo(exec_policy::quantify::Nested));
        });

        it("can be conversion constructed from 'quantify::algorithm'", [&]() {
          exec_policy ep = exec_policy::quantify::Singleton;

          AssertThat(ep.template get<exec_policy::access>(),
                     Is().EqualTo(exec_policy::access::Auto));
          AssertThat(ep.template get<exec_policy::memory>(),
                     Is().EqualTo(exec_policy::memory::Auto));

          AssertThat(ep.template get<exec_policy::quantify::algorithm>(),
                     Is().EqualTo(exec_policy::quantify::Singleton));
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

        it("can set settigs with a builder pattern syntax", [&]() {
          exec_policy ep;

          ep.set(exec_policy::access::Priority_Queue)
            .set(exec_policy::memory::External)
            .set(exec_policy::quantify::Singleton);

          AssertThat(ep.template get<exec_policy::access>(),
                     Is().EqualTo(exec_policy::access::Priority_Queue));

          AssertThat(ep.template get<exec_policy::memory>(),
                     Is().EqualTo(exec_policy::memory::External));

          AssertThat(ep.template get<exec_policy::quantify::algorithm>(),
                     Is().EqualTo(exec_policy::quantify::Singleton));
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
          const exec_policy out = in & exec_policy::quantify::Singleton;

          AssertThat(in.template get<exec_policy::quantify::algorithm>(),
                     Is().EqualTo(exec_policy::quantify::Nested));
          AssertThat(out.template get<exec_policy::quantify::algorithm>(),
                     Is().EqualTo(exec_policy::quantify::Singleton));
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

        it("can lift enum values [quantify*]", [&]() {
          const exec_policy ep = exec_policy::quantify::Nested;

          AssertThat(ep.template get<exec_policy::access>(),
                     Is().EqualTo(exec_policy::access::Auto));

          AssertThat(ep.template get<exec_policy::memory>(),
                     Is().EqualTo(exec_policy::memory::Auto));

          AssertThat(ep.template get<exec_policy::quantify::algorithm>(),
                     Is().EqualTo(exec_policy::quantify::Nested));
        });
      });
    });
  });
});
