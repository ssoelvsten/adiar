macro(add_test NAME SOURCE)
  add_executable (adiar_test-${NAME} ${SOURCE})

  # Parse CMake options to executable
  # TODO

  # Link up with Bandit
  set(BANDIT_FLAGS "--reporter=info --colorizer=light")
  target_include_directories (adiar_test-${NAME}
                              PRIVATE ${PROJECT_SOURCE_DIR}/external/bandit)

  # Link with Adiar
  target_link_libraries(adiar_test-${NAME} adiar)
endmacro(add_test)

# ============================================================================ #
# Add 'test-adiar' target
add_test(all all.cpp)
target_compile_definitions(adiar_test-all PRIVATE ADIAR_TEST_PRINT_STATS)

# ============================================================================ #
# Add individual test files
add_subdirectory (adiar)
