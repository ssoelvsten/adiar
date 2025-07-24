.PHONY: build clean coverage docs test test/*

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

TEST_NAME = test-adiar

test: TEST_FOLDER = test
test: TEST_NAME = adiar
test:
	@mkdir -p build/
	@cd build/ && cmake -D CMAKE_BUILD_TYPE=Debug \
                      -D CMAKE_C_FLAGS=$(O2_FLAGS) \
                      -D CMAKE_CXX_FLAGS=$(O2_FLAGS) \
                      -D ADIAR_STATS=ON \
                ..

	@cd build/ && $(MAKE) $(MAKE_FLAGS) $(subst /,-,$(TEST_FOLDER))-$(TEST_NAME)

	$(MAKE) clean/files
	@./build/$(TEST_FOLDER)/$(subst /,-,$(TEST_FOLDER))-$(TEST_NAME) \
      --reporter=info --colorizer=light

	$(MAKE) clean/files

test/all:
	$(MAKE) $(MAKE_FLAGS) test

test/adiar:
	$(MAKE) $(MAKE_FLAGS) test

test/adiar/bool_op:
	$(MAKE) $(MAKE_FLAGS) test TEST_FOLDER=test/adiar TEST_NAME=bool_op

test/adiar/builder:
	$(MAKE) $(MAKE_FLAGS) test TEST_FOLDER=test/adiar TEST_NAME=builder

test/adiar/domain:
	$(MAKE) $(MAKE_FLAGS) test TEST_FOLDER=test/adiar TEST_NAME=domain

test/adiar/exec_policy:
	$(MAKE) $(MAKE_FLAGS) test TEST_FOLDER=test/adiar TEST_NAME=exec_policy

test/adiar/functional:
	$(MAKE) $(MAKE_FLAGS) test TEST_FOLDER=test/adiar TEST_NAME=functional

test/adiar/bdd/apply:
	$(MAKE) $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/bdd TEST_NAME=apply

test/adiar/bdd/bdd:
	$(MAKE) $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/bdd TEST_NAME=bdd

test/adiar/bdd/build:
	$(MAKE) $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/bdd TEST_NAME=build

test/adiar/bdd/count:
	$(MAKE) $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/bdd TEST_NAME=count

test/adiar/bdd/evaluate:
	$(MAKE) $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/bdd TEST_NAME=evaluate

test/adiar/bdd/if_then_else:
	$(MAKE) $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/bdd TEST_NAME=if_then_else

test/adiar/bdd/optmin:
	$(MAKE) $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/bdd TEST_NAME=optmin

test/adiar/bdd/pred:
	$(MAKE) $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/bdd TEST_NAME=pred

test/adiar/bdd/negate:
	$(MAKE) $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/bdd TEST_NAME=negate

test/adiar/bdd/quantify:
	$(MAKE) $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/bdd TEST_NAME=quantify

test/adiar/bdd/relprod:
	$(MAKE) $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/bdd TEST_NAME=relprod

test/adiar/bdd/replace:
	$(MAKE) $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/bdd TEST_NAME=replace

test/adiar/bdd/restrict:
	$(MAKE) $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/bdd TEST_NAME=restrict

test/adiar/internal/dd_func:
	$(MAKE) $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/internal TEST_NAME=dd_func

test/adiar/internal/util:
	$(MAKE) $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/internal TEST_NAME=util

test/adiar/internal/algorithms/convert:
	$(MAKE) $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/internal/algorithms TEST_NAME=convert

test/adiar/internal/algorithms/dot:
	$(MAKE) $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/internal/algorithms TEST_NAME=dot

test/adiar/internal/algorithms/isomorphism:
	$(MAKE) $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/internal/algorithms TEST_NAME=isomorphism

test/adiar/internal/algorithms/nested_sweeping:
	$(MAKE) $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/internal/algorithms TEST_NAME=nested_sweeping

test/adiar/internal/algorithms/reduce:
	$(MAKE) $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/internal/algorithms TEST_NAME=reduce

test/adiar/internal/bool_op:
	$(MAKE) $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/internal TEST_NAME=bool_op

test/adiar/internal/data_structures/level_merger:
	$(MAKE) $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/internal/data_structures TEST_NAME=level_merger

test/adiar/internal/data_structures/levelized_priority_queue:
	$(MAKE) $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/internal/data_structures TEST_NAME=levelized_priority_queue

test/adiar/internal/data_structures/priority_queue:
	$(MAKE) $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/internal/data_structures TEST_NAME=priority_queue

test/adiar/internal/data_structures/sorter:
	$(MAKE) $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/internal/data_structures TEST_NAME=sorter

test/adiar/internal/data_structures/stack:
	$(MAKE) $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/internal/data_structures TEST_NAME=stack

test/adiar/internal/data_types/arc:
	$(MAKE) $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/internal/data_types TEST_NAME=arc

test/adiar/internal/data_types/convert:
	$(MAKE) $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/internal/data_types TEST_NAME=convert

test/adiar/internal/data_types/level_info:
	$(MAKE) $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/internal/data_types TEST_NAME=level_info

test/adiar/internal/data_types/node:
	$(MAKE) $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/internal/data_types TEST_NAME=node

test/adiar/internal/data_types/ptr:
	$(MAKE) $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/internal/data_types TEST_NAME=ptr

test/adiar/internal/data_types/request:
	$(MAKE) $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/internal/data_types TEST_NAME=request

test/adiar/internal/data_types/tuple:
	$(MAKE) $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/internal/data_types TEST_NAME=tuple

test/adiar/internal/data_types/uid:
	$(MAKE) $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/internal/data_types TEST_NAME=uid

test/adiar/internal/io/arc_file:
	$(MAKE) $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/internal/io TEST_NAME=arc_file

test/adiar/internal/io/file:
	$(MAKE) $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/internal/io TEST_NAME=file

test/adiar/internal/io/levelized_file:
	$(MAKE) $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/internal/io TEST_NAME=levelized_file

test/adiar/internal/io/node_file:
	$(MAKE) $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/internal/io TEST_NAME=node_file

test/adiar/internal/io/shared_file_ptr:
	$(MAKE) $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/internal/io TEST_NAME=shared_file_ptr

test/adiar/zdd/binop:
	$(MAKE) $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/zdd TEST_NAME=binop

test/adiar/zdd/build:
	$(MAKE) $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/zdd TEST_NAME=build

test/adiar/zdd/change:
	$(MAKE) $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/zdd TEST_NAME=change

test/adiar/zdd/complement:
	$(MAKE) $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/zdd TEST_NAME=complement

test/adiar/zdd/contains:
	$(MAKE) $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/zdd TEST_NAME=contains

test/adiar/zdd/count:
	$(MAKE) $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/zdd TEST_NAME=count

test/adiar/zdd/elem:
	$(MAKE) $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/zdd TEST_NAME=elem

test/adiar/zdd/expand:
	$(MAKE) $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/zdd TEST_NAME=expand

test/adiar/zdd/pred:
	$(MAKE) $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/zdd TEST_NAME=pred

test/adiar/zdd/project:
	$(MAKE) $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/zdd TEST_NAME=project

test/adiar/zdd/subset:
	$(MAKE) $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/zdd TEST_NAME=subset

test/adiar/zdd/zdd:
	$(MAKE) $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/zdd TEST_NAME=zdd

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
	@cd build/ && $(MAKE) $(MAKE_FLAGS) test-adiar

	@lcov --directory build/src/adiar/ --zerocounters
	$(MAKE) clean/files

	@./build/test/test-adiar || echo ""
	$(MAKE) clean/files

  # create report
	@rm -rf test/report/
	@lcov --capture --directory build/ --output-file ./coverage.info
	@lcov --remove coverage.info --output-file coverage.info "/usr/*" "*/external/*" "./test/*"
  # print report to console
	@lcov --list coverage.info
  # print report to html file
	@genhtml coverage.info -o test/report/

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
M = 1024

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
	@$(MAKE) playground

# ============================================================================ #
#  EXAMPLES
# ============================================================================ #

example/queens: N := 8
example/queens:
  # Build
	@mkdir -p build/
	@cd build/ && cmake -D CMAKE_BUILD_TYPE=Release ..

	@cd build/ && $(MAKE) $(MAKE_FLAGS) queens

  # Run
	@echo ""
	./build/example/queens -N ${N} -M ${M}
	@echo ""


example/knights_tour/all: N := 5
example/knights_tour/all:
  # Build
	@mkdir -p build/
	@cd build/ && cmake -D CMAKE_BUILD_TYPE=Release ..

	@cd build/ && $(MAKE) $(MAKE_FLAGS) knights_tour

  # Run
	@echo ""
	@./build/example/knights_tour -N ${N} -M ${M}
	@echo ""

example/knights_tour/closed: N := 6
example/knights_tour/closed:
  # Build
	@mkdir -p build/
	@cd build/ && cmake -D CMAKE_BUILD_TYPE=Release ..

	@cd build/ && $(MAKE) $(MAKE_FLAGS) knights_tour

  # Run
	@echo ""
	@./build/example/knights_tour -N ${N} -M ${M} -c
	@echo ""
