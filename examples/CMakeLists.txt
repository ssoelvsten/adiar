macro(add_example NAME)
  add_executable (adiar_example-${NAME} ${NAME}.cpp)
  target_link_libraries(adiar_example-${NAME} adiar)
endmacro(add_example)

add_example(basic)
add_example(builder)
add_example(functional)
add_example(queens)

