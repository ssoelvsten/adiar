add_test(adiar-bool_op     test_bool_op.cpp)
add_test(adiar-builder     test_builder.cpp)
add_test(adiar-domain      test_domain.cpp)
add_test(adiar-exec_policy test_exec_policy.cpp)
add_test(adiar-functional  test_functional.cpp)

add_subdirectory (bdd)
add_subdirectory (internal)
add_subdirectory (zdd)
