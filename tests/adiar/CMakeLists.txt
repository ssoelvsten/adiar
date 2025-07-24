add_test(adiar-bool_op     bool_op.test.cpp)
add_test(adiar-builder     builder.test.cpp)
add_test(adiar-domain      domain.test.cpp)
add_test(adiar-exec_policy exec_policy.test.cpp)
add_test(adiar-functional  functional.test.cpp)

add_subdirectory (bdd)
add_subdirectory (internal)
add_subdirectory (zdd)
