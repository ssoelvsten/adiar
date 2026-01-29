////////////////////////////////////////////////////////////////////////////////
// Common testing logic
#define ADIAR_TEST_NO_INIT

#include "test.h"

////////////////////////////////////////////////////////////////////////////////
// Adiar Initialization unit tests
go_bandit([]() {
  describe("adiar/adiar.h", []() {
    it("is at first not initialized", [&]() { AssertThat(adiar_initialized(), Is().False()); });

    it("throws exception when given '0' memory",
       [&]() { AssertThrows(invalid_argument, adiar_init(0)); });

    it("throws exception when given 'minimum_memory - 1' memory",
       [&]() { AssertThrows(invalid_argument, adiar_init(minimum_memory - 1)); });

    it("can run 'adiar_init()'", [&]() { adiar_init(1024 * 1024 * 1024); });

    it("is then initialized", [&]() { AssertThat(adiar_initialized(), Is().True()); });

    it("can rerun 'adiar_init()' without any new effect", [&]() {
      AssertThat(adiar_initialized(), Is().True());
      adiar_init(1024 * 1024 * 1024);
    });

    // TODO: more tests when 'https://github.com/thomasmoelhave/tpie/issues/265'
    //       is resolved.
  });

  // Kill program immediately instead of trying to run crashing unit tests.
  if (!adiar_initialized()) exit(-1);
});

go_bandit([]() {
  describe("adiar/domain.h", []() {
    it("initially as no domain", [&]() { AssertThat(domain_isset(), Is().False()); });
  });
});

////////////////////////////////////////////////////////////////////////////////
// Adiar Internal unit tests
#include "adiar/exec_policy.test.cpp"
#include "adiar/functional.test.cpp"
#include "adiar/internal/algorithms/convert.test.cpp"
#include "adiar/internal/algorithms/dot.test.cpp"
#include "adiar/internal/algorithms/isomorphism.test.cpp"
#include "adiar/internal/algorithms/nested_sweeping.test.cpp"
#include "adiar/internal/algorithms/reduce.test.cpp"
#include "adiar/internal/data_structures/level_merger.test.cpp"
#include "adiar/internal/data_structures/levelized_priority_queue.test.cpp"
#include "adiar/internal/data_structures/priority_queue.test.cpp"
#include "adiar/internal/data_types/arc.test.cpp"
#include "adiar/internal/data_types/convert.test.cpp"
#include "adiar/internal/data_types/level_info.test.cpp"
#include "adiar/internal/data_types/node.test.cpp"
#include "adiar/internal/data_types/ptr.test.cpp"
#include "adiar/internal/data_types/request.test.cpp"
#include "adiar/internal/data_types/tuple.test.cpp"
#include "adiar/internal/data_types/uid.test.cpp"
#include "adiar/internal/dd_func.test.cpp"
#include "adiar/internal/io/arc_file.test.cpp"
#include "adiar/internal/io/file.test.cpp"
#include "adiar/internal/io/levelized_file.test.cpp"
#include "adiar/internal/io/node_file.test.cpp"
#include "adiar/internal/io/shared_file_ptr.test.cpp"
#include "adiar/internal/util.test.cpp"

////////////////////////////////////////////////////////////////////////////////
// Adiar Core unit tests
#include "adiar/bool_op.test.cpp"
#include "adiar/builder.test.cpp"
#include "adiar/domain.test.cpp"
#include "adiar/internal/bool_op.test.cpp"

////////////////////////////////////////////////////////////////////////////////
// Adiar BDD unit tests
#include "adiar/bdd/apply.test.cpp"
#include "adiar/bdd/bdd.test.cpp"
#include "adiar/bdd/build.test.cpp"
#include "adiar/bdd/count.test.cpp"
#include "adiar/bdd/evaluate.test.cpp"
#include "adiar/bdd/if_then_else.test.cpp"
#include "adiar/bdd/negate.test.cpp"
#include "adiar/bdd/optmin.test.cpp"
#include "adiar/bdd/pred.test.cpp"
#include "adiar/bdd/quantify.test.cpp"
#include "adiar/bdd/relprod.test.cpp"
#include "adiar/bdd/replace.test.cpp"
#include "adiar/bdd/restrict.test.cpp"

////////////////////////////////////////////////////////////////////////////////
// Adiar ZDD unit tests
#include "adiar/zdd/binop.test.cpp"
#include "adiar/zdd/build.test.cpp"
#include "adiar/zdd/change.test.cpp"
#include "adiar/zdd/complement.test.cpp"
#include "adiar/zdd/contains.test.cpp"
#include "adiar/zdd/count.test.cpp"
#include "adiar/zdd/elem.test.cpp"
#include "adiar/zdd/expand.test.cpp"
#include "adiar/zdd/pred.test.cpp"
#include "adiar/zdd/project.test.cpp"
#include "adiar/zdd/subset.test.cpp"
#include "adiar/zdd/zdd.test.cpp"

////////////////////////////////////////////////////////////////////////////////
// Adiar Deinitialization unit tests
go_bandit([]() {
  describe("adiar/adiar.h", []() {
    it("can be deinitialized", [&]() {
      AssertThat(adiar_initialized(), Is().True());

      // TODO: enforce being true independent of above unit tests
      AssertThat(domain_isset(), Is().True());

      adiar_deinit();

      AssertThat(adiar_initialized(), Is().False());
      AssertThat(domain_isset(), Is().False());
    });

    it("throws exception when reinitialized", [&]() {
      // TODO: remove when 'github.com/thomasmoelhave/tpie/issues/265' is fixed.
      AssertThrows(runtime_error, adiar_init(minimum_memory));
    });
  });
});
