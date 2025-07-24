# From:
#   https://www.labri.fr/perso/fleury/posts/programming/using-clang-tidy-and-clang-format.html

# Get all src/ files
file(GLOB_RECURSE ALL_SRC_FILES
  ${PROJECT_SOURCE_DIR}/src/*.[chi]pp
  ${PROJECT_SOURCE_DIR}/src/*.[chi]xx
  ${PROJECT_SOURCE_DIR}/src/*.cc
  ${PROJECT_SOURCE_DIR}/src/*.hh
  ${PROJECT_SOURCE_DIR}/src/*.h
  ${PROJECT_SOURCE_DIR}/src/*.ii
  ${PROJECT_SOURCE_DIR}/src/*.[CHI]
)

# Get all test/ files
file(GLOB_RECURSE ALL_TEST_FILES
  ${PROJECT_SOURCE_DIR}/test/*.[chi]pp
  ${PROJECT_SOURCE_DIR}/test/*.[chi]xx
  ${PROJECT_SOURCE_DIR}/test/*.cc
  ${PROJECT_SOURCE_DIR}/test/*.hh
  ${PROJECT_SOURCE_DIR}/test/*.h
  ${PROJECT_SOURCE_DIR}/test/*.ii
  ${PROJECT_SOURCE_DIR}/test/*.[CHI]
)

# Adding clang-format target if executable is found
find_program(CLANG_FORMAT "clang-format")
if(CLANG_FORMAT)
  add_custom_target(
    adiar_clang-format
    COMMAND /usr/bin/clang-format
    ${ALL_SRC_FILES} ${ALL_TEST_FILES}
    -i
  )
endif()

# Adding clang-tidy target if executable is found
find_program(CLANG_TIDY "clang-tidy")
if(CLANG_TIDY)
  add_custom_target(
    adiar_clang-tidy
    COMMAND /usr/bin/clang-tidy
    ${ALL_SRC_SOURCE_FILES} ${ALL_TEST_SOURCE_FILES}
    -config=''
    --
    -std=c++11
    ${INCLUDE_DIRECTORIES}
  )
endif()
