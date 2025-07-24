.PHONY: build clean coverage docs tests tests/*

MAKE_FLAGS=-j $$(nproc)

# ============================================================================ #
#  BUILD
# ============================================================================ #
BUILD_TYPE = "Release"

build:
	$(MAKE) build/static

build/static:
	@mkdir -p build/ && cd build/ && cmake -D CMAKE_BUILD_TYPE=$(BUILD_TYPE) ..
	@cd build/ && make adiar

build/shared:
	@echo "Not supportd (See Issue #200)"

# ============================================================================ #
#  CLEAN
# ============================================================================ #
clean/files:
	@rm -rf *.tpie
	@rm -rf *.adiar*
	@rm -rf *.dot*

clean: | clean/files
	@rm -r -f build/

# ============================================================================ #
#  CLANG TOOLS
# ============================================================================ #
format:
	@mkdir -p build/ && cd build/ && cmake -D CMAKE_BUILD_TYPE=$(BUILD_TYPE) ..
	@cd build/ && make adiar_clang-format

# ============================================================================ #
#  UNIT TESTING
# ============================================================================ #
O2_FLAGS = "-g -O2"

tests: TEST_SUBFOLDER = ""
tests: TEST_NAME = "all"
tests:
	@mkdir -p build/
	@cd build/ && cmake -D CMAKE_BUILD_TYPE=Debug \
                      -D CMAKE_C_FLAGS=$(O2_FLAGS) \
                      -D CMAKE_CXX_FLAGS=$(O2_FLAGS) \
                      -D ADIAR_STATS=ON \
                ..

	@cd build/ && $(MAKE) $(MAKE_FLAGS) adiar_test-$(subst /,-,$(TEST_SUBFOLDER))$(TEST_NAME)

	$(MAKE) clean/files
	@./build/tests/$(TEST_SUBFOLDER)/adiar_test-$(subst /,-,$(TEST_SUBFOLDER))$(TEST_NAME) \
      --reporter=info --colorizer=light

	$(MAKE) clean/files

tests/all:
	$(MAKE) $(MAKE_FLAGS) tests

tests/adiar/bool_op:
	$(MAKE) $(MAKE_FLAGS) tests TEST_SUBFOLDER=adiar/ TEST_NAME=bool_op

tests/adiar/builder:
	$(MAKE) $(MAKE_FLAGS) tests TEST_SUBFOLDER=adiar/ TEST_NAME=builder

tests/adiar/domain:
	$(MAKE) $(MAKE_FLAGS) tests TEST_SUBFOLDER=adiar/ TEST_NAME=domain

tests/adiar/exec_policy:
	$(MAKE) $(MAKE_FLAGS) tests TEST_SUBFOLDER=adiar/ TEST_NAME=exec_policy

tests/adiar/functional:
	$(MAKE) $(MAKE_FLAGS) tests TEST_SUBFOLDER=adiar/ TEST_NAME=functional

tests/adiar/bdd/apply:
	$(MAKE) $(MAKE_FLAGS) tests TEST_SUBFOLDER=adiar/bdd/ TEST_NAME=apply

tests/adiar/bdd/bdd:
	$(MAKE) $(MAKE_FLAGS) tests TEST_SUBFOLDER=adiar/bdd/ TEST_NAME=bdd

tests/adiar/bdd/build:
	$(MAKE) $(MAKE_FLAGS) tests TEST_SUBFOLDER=adiar/bdd/ TEST_NAME=build

tests/adiar/bdd/count:
	$(MAKE) $(MAKE_FLAGS) tests TEST_SUBFOLDER=adiar/bdd/ TEST_NAME=count

tests/adiar/bdd/evaluate:
	$(MAKE) $(MAKE_FLAGS) tests TEST_SUBFOLDER=adiar/bdd/ TEST_NAME=evaluate

tests/adiar/bdd/if_then_else:
	$(MAKE) $(MAKE_FLAGS) tests TEST_SUBFOLDER=adiar/bdd/ TEST_NAME=if_then_else

tests/adiar/bdd/optmin:
	$(MAKE) $(MAKE_FLAGS) tests TEST_SUBFOLDER=adiar/bdd/ TEST_NAME=optmin

tests/adiar/bdd/pred:
	$(MAKE) $(MAKE_FLAGS) tests TEST_SUBFOLDER=adiar/bdd/ TEST_NAME=pred

tests/adiar/bdd/negate:
	$(MAKE) $(MAKE_FLAGS) tests TEST_SUBFOLDER=adiar/bdd/ TEST_NAME=negate

tests/adiar/bdd/quantify:
	$(MAKE) $(MAKE_FLAGS) tests TEST_SUBFOLDER=adiar/bdd/ TEST_NAME=quantify

tests/adiar/bdd/relprod:
	$(MAKE) $(MAKE_FLAGS) tests TEST_SUBFOLDER=adiar/bdd/ TEST_NAME=relprod

tests/adiar/bdd/replace:
	$(MAKE) $(MAKE_FLAGS) tests TEST_SUBFOLDER=adiar/bdd/ TEST_NAME=replace

tests/adiar/bdd/restrict:
	$(MAKE) $(MAKE_FLAGS) tests TEST_SUBFOLDER=adiar/bdd/ TEST_NAME=restrict

tests/adiar/internal/dd_func:
	$(MAKE) $(MAKE_FLAGS) tests TEST_SUBFOLDER=adiar/internal/ TEST_NAME=dd_func

tests/adiar/internal/util:
	$(MAKE) $(MAKE_FLAGS) tests TEST_SUBFOLDER=adiar/internal/ TEST_NAME=util

tests/adiar/internal/algorithms/convert:
	$(MAKE) $(MAKE_FLAGS) tests TEST_SUBFOLDER=adiar/internal/algorithms/ TEST_NAME=convert

tests/adiar/internal/algorithms/dot:
	$(MAKE) $(MAKE_FLAGS) tests TEST_SUBFOLDER=adiar/internal/algorithms/ TEST_NAME=dot

tests/adiar/internal/algorithms/isomorphism:
	$(MAKE) $(MAKE_FLAGS) tests TEST_SUBFOLDER=adiar/internal/algorithms/ TEST_NAME=isomorphism

tests/adiar/internal/algorithms/nested_sweeping:
	$(MAKE) $(MAKE_FLAGS) tests TEST_SUBFOLDER=adiar/internal/algorithms/ TEST_NAME=nested_sweeping

tests/adiar/internal/algorithms/reduce:
	$(MAKE) $(MAKE_FLAGS) tests TEST_SUBFOLDER=adiar/internal/algorithms/ TEST_NAME=reduce

tests/adiar/internal/bool_op:
	$(MAKE) $(MAKE_FLAGS) tests TEST_SUBFOLDER=adiar/internal/ TEST_NAME=bool_op

tests/adiar/internal/data_structures/level_merger:
	$(MAKE) $(MAKE_FLAGS) tests TEST_SUBFOLDER=adiar/internal/data_structures/ TEST_NAME=level_merger

tests/adiar/internal/data_structures/levelized_priority_queue:
	$(MAKE) $(MAKE_FLAGS) tests TEST_SUBFOLDER=adiar/internal/data_structures/ TEST_NAME=levelized_priority_queue

tests/adiar/internal/data_structures/priority_queue:
	$(MAKE) $(MAKE_FLAGS) tests TEST_SUBFOLDER=adiar/internal/data_structures/ TEST_NAME=priority_queue

tests/adiar/internal/data_structures/sorter:
	$(MAKE) $(MAKE_FLAGS) tests TEST_SUBFOLDER=adiar/internal/data_structures/ TEST_NAME=sorter

tests/adiar/internal/data_structures/stack:
	$(MAKE) $(MAKE_FLAGS) tests TEST_SUBFOLDER=adiar/internal/data_structures/ TEST_NAME=stack

tests/adiar/internal/data_types/arc:
	$(MAKE) $(MAKE_FLAGS) tests TEST_SUBFOLDER=adiar/internal/data_types/ TEST_NAME=arc

tests/adiar/internal/data_types/convert:
	$(MAKE) $(MAKE_FLAGS) tests TEST_SUBFOLDER=adiar/internal/data_types/ TEST_NAME=convert

tests/adiar/internal/data_types/level_info:
	$(MAKE) $(MAKE_FLAGS) tests TEST_SUBFOLDER=adiar/internal/data_types/ TEST_NAME=level_info

tests/adiar/internal/data_types/node:
	$(MAKE) $(MAKE_FLAGS) tests TEST_SUBFOLDER=adiar/internal/data_types/ TEST_NAME=node

tests/adiar/internal/data_types/ptr:
	$(MAKE) $(MAKE_FLAGS) tests TEST_SUBFOLDER=adiar/internal/data_types/ TEST_NAME=ptr

tests/adiar/internal/data_types/request:
	$(MAKE) $(MAKE_FLAGS) tests TEST_SUBFOLDER=adiar/internal/data_types/ TEST_NAME=request

tests/adiar/internal/data_types/tuple:
	$(MAKE) $(MAKE_FLAGS) tests TEST_SUBFOLDER=adiar/internal/data_types/ TEST_NAME=tuple

tests/adiar/internal/data_types/uid:
	$(MAKE) $(MAKE_FLAGS) tests TEST_SUBFOLDER=adiar/internal/data_types/ TEST_NAME=uid

tests/adiar/internal/io/arc_file:
	$(MAKE) $(MAKE_FLAGS) tests TEST_SUBFOLDER=adiar/internal/io/ TEST_NAME=arc_file

tests/adiar/internal/io/file:
	$(MAKE) $(MAKE_FLAGS) tests TEST_SUBFOLDER=adiar/internal/io/ TEST_NAME=file

tests/adiar/internal/io/levelized_file:
	$(MAKE) $(MAKE_FLAGS) tests TEST_SUBFOLDER=adiar/internal/io/ TEST_NAME=levelized_file

tests/adiar/internal/io/node_file:
	$(MAKE) $(MAKE_FLAGS) tests TEST_SUBFOLDER=adiar/internal/io/ TEST_NAME=node_file

tests/adiar/internal/io/shared_file_ptr:
	$(MAKE) $(MAKE_FLAGS) tests TEST_SUBFOLDER=adiar/internal/io/ TEST_NAME=shared_file_ptr

tests/adiar/zdd/binop:
	$(MAKE) $(MAKE_FLAGS) tests TEST_SUBFOLDER=adiar/zdd/ TEST_NAME=binop

tests/adiar/zdd/build:
	$(MAKE) $(MAKE_FLAGS) tests TEST_SUBFOLDER=adiar/zdd/ TEST_NAME=build

tests/adiar/zdd/change:
	$(MAKE) $(MAKE_FLAGS) tests TEST_SUBFOLDER=adiar/zdd/ TEST_NAME=change

tests/adiar/zdd/complement:
	$(MAKE) $(MAKE_FLAGS) tests TEST_SUBFOLDER=adiar/zdd/ TEST_NAME=complement

tests/adiar/zdd/contains:
	$(MAKE) $(MAKE_FLAGS) tests TEST_SUBFOLDER=adiar/zdd/ TEST_NAME=contains

tests/adiar/zdd/count:
	$(MAKE) $(MAKE_FLAGS) tests TEST_SUBFOLDER=adiar/zdd/ TEST_NAME=count

tests/adiar/zdd/elem:
	$(MAKE) $(MAKE_FLAGS) tests TEST_SUBFOLDER=adiar/zdd/ TEST_NAME=elem

tests/adiar/zdd/expand:
	$(MAKE) $(MAKE_FLAGS) tests TEST_SUBFOLDER=adiar/zdd/ TEST_NAME=expand

tests/adiar/zdd/pred:
	$(MAKE) $(MAKE_FLAGS) tests TEST_SUBFOLDER=adiar/zdd/ TEST_NAME=pred

tests/adiar/zdd/project:
	$(MAKE) $(MAKE_FLAGS) tests TEST_SUBFOLDER=adiar/zdd/ TEST_NAME=project

tests/adiar/zdd/subset:
	$(MAKE) $(MAKE_FLAGS) tests TEST_SUBFOLDER=adiar/zdd/ TEST_NAME=subset

tests/adiar/zdd/zdd:
	$(MAKE) $(MAKE_FLAGS) tests TEST_SUBFOLDER=adiar/zdd/ TEST_NAME=zdd

# ============================================================================ #
#  LCOV COVERAGE REPORT
# ============================================================================ #
COV_C_FLAGS = "-g -O0 -Wall -fprofile-arcs -ftest-coverage"
COV_EXE_LINKER_FLAGS = "-fprofile-arcs -ftest-coverage"

coverage:
	@mkdir -p build/
	@cd build/ && cmake -D CMAKE_BUILD_TYPE=Debug \
                      -D CMAKE_C_FLAGS=$(COV_C_FLAGS) \
                      -D CMAKE_CXX_FLAGS=$(COV_C_FLAGS) \
                      -D CMAKE_EXE_LINKER_FLAGS=$(COV_EXE_LINKER_FLAGS) \
                      -D ADIAR_STATS=ON \
                ..
	@cd build/ && $(MAKE) $(MAKE_FLAGS) adiar_test-all

	@lcov --directory ./build/src/adiar/ --zerocounters
	$(MAKE) clean/files

	@./build/tests/adiar_test-all || echo ""
	$(MAKE) clean/files

  # create report
	@rm -rf tests/report/
	@lcov --capture --directory build/ --output-file ./coverage.info
	@lcov --remove coverage.info --ignore-errors unused --output-file coverage.info "/usr/*" "external/*" "*/external/*" "tests/*"
  # print report to console
	@lcov --list coverage.info
  # print report to html file
	@genhtml coverage.info -o tests/report/

# ============================================================================ #
#  DOCUMENTATION
# ============================================================================ #
docs:
	@mkdir -p build/
	@cd build/ && cmake ..

	@cd build/ && $(MAKE) adiar_docs

# ============================================================================ #
#  PLAYGROUND
# ============================================================================ #
playground: M := 1024
playground:
	@mkdir -p build/
	@cd build/ && cmake -D CMAKE_BUILD_TYPE=Debug \
                      -D CMAKE_C_FLAGS=$(O2_FLAGS) \
                      -D CMAKE_CXX_FLAGS=$(O2_FLAGS) \
                      -D ADIAR_STATS=ON \
                ..
	@cd build/ && $(MAKE) $(MAKE_FLAGS) adiar_playground
	@echo "" && echo ""
	@./build/app/adiar_playground ${M}

play:
	@$(MAKE) $(MAKE_FLAGS) playground

# ============================================================================ #
#  EXAMPLES
# ============================================================================ #
examples/NAME:
  # Build
	@mkdir -p build/
	@cd build/ && cmake -D CMAKE_BUILD_TYPE=Release ..

	@cd build/ && $(MAKE) $(MAKE_FLAGS) adiar_example-$(NAME)

  # Run
	@echo ""
	./build/examples/adiar_example-$(NAME)
	@echo ""

examples/builder:
	$(MAKE) $(MAKE_FLAGS) examples/NAME NAME=builder

examples/functional:
	$(MAKE) $(MAKE_FLAGS) examples/NAME NAME=functional

examples/queens: M := 1024
examples/queens: N := 8
examples/queens:
  # Build
	@mkdir -p build/
	@cd build/ && cmake -D CMAKE_BUILD_TYPE=Release ..

	@cd build/ && $(MAKE) $(MAKE_FLAGS) adiar_example-queens

  # Run
	@echo ""
	./build/examples/adiar_example-queens -N ${N} -M ${M}
	@echo ""
