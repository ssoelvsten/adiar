# ============================================================================ #
# N-Queens example
add_executable (queens queens.cpp)

target_link_libraries(queens adiar)

# ============================================================================ #
# Knight's Tour example
add_executable (knights_tour knights_tour.cpp)

target_link_libraries(knights_tour adiar)
