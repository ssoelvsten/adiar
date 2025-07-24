add_executable (adiar_playground playground.cpp)
target_link_libraries(adiar_playground adiar)

# Dump assembly output too (if GCC)
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set_target_properties(adiar_playground PROPERTIES COMPILE_FLAGS "-save-temps")
endif ()
