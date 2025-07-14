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
#include "adiar/internal/algorithms/test_convert.cpp"
#include "adiar/internal/algorithms/test_dot.cpp"
#include "adiar/internal/algorithms/test_isomorphism.cpp"
#include "adiar/internal/algorithms/test_nested_sweeping.cpp"
#include "adiar/internal/algorithms/test_reduce.cpp"
#include "adiar/internal/data_structures/test_level_merger.cpp"
#include "adiar/internal/data_structures/test_levelized_priority_queue.cpp"
#include "adiar/internal/data_structures/test_priority_queue.cpp"
#include "adiar/internal/data_types/test_arc.cpp"
#include "adiar/internal/data_types/test_convert.cpp"
#include "adiar/internal/data_types/test_level_info.cpp"
#include "adiar/internal/data_types/test_node.cpp"
#include "adiar/internal/data_types/test_ptr.cpp"
#include "adiar/internal/data_types/test_request.cpp"
#include "adiar/internal/data_types/test_tuple.cpp"
#include "adiar/internal/data_types/test_uid.cpp"
#include "adiar/internal/io/test_arc_file.cpp"
#include "adiar/internal/io/test_file.cpp"
#include "adiar/internal/io/test_levelized_file.cpp"
#include "adiar/internal/io/test_node_file.cpp"
#include "adiar/internal/io/test_shared_file_ptr.cpp"
#include "adiar/internal/test_dd_func.cpp"
#include "adiar/internal/test_util.cpp"
#include "adiar/test_exec_policy.cpp"
#include "adiar/test_functional.cpp"

////////////////////////////////////////////////////////////////////////////////
// Adiar Core unit tests
#include "adiar/internal/test_bool_op.cpp"
#include "adiar/test_bool_op.cpp"
#include "adiar/test_builder.cpp"
#include "adiar/test_domain.cpp"

////////////////////////////////////////////////////////////////////////////////
// Adiar BDD unit tests
#include "adiar/bdd/test_apply.cpp"
#include "adiar/bdd/test_bdd.cpp"
#include "adiar/bdd/test_build.cpp"
#include "adiar/bdd/test_count.cpp"
#include "adiar/bdd/test_evaluate.cpp"
#include "adiar/bdd/test_if_then_else.cpp"
#include "adiar/bdd/test_negate.cpp"
#include "adiar/bdd/test_pred.cpp"
#include "adiar/bdd/test_quantify.cpp"
#include "adiar/bdd/test_relprod.cpp"
#include "adiar/bdd/test_replace.cpp"
#include "adiar/bdd/test_restrict.cpp"

////////////////////////////////////////////////////////////////////////////////
// Adiar ZDD unit tests
#include "adiar/zdd/test_binop.cpp"
#include "adiar/zdd/test_build.cpp"
#include "adiar/zdd/test_change.cpp"
#include "adiar/zdd/test_complement.cpp"
#include "adiar/zdd/test_contains.cpp"
#include "adiar/zdd/test_count.cpp"
#include "adiar/zdd/test_elem.cpp"
#include "adiar/zdd/test_expand.cpp"
#include "adiar/zdd/test_pred.cpp"
#include "adiar/zdd/test_project.cpp"
#include "adiar/zdd/test_subset.cpp"
#include "adiar/zdd/test_zdd.cpp"

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
