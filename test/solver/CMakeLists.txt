add_custom_target(solver DEPENDS gam-solve COMMAND root -b -q  ${CMAKE_SOURCE_DIR}/test/solver/tests.C)
