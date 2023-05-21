add_test(ctest_build_gam_solve
         "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} --target gam-solve)
add_test(NAME ctest_test_gam_solve COMMAND root -b -q  ${CMAKE_SOURCE_DIR}/test/solver/tests.C)
set_tests_properties(ctest_test_gam_solve
                     PROPERTIES DEPENDS ctest_build_gam_solve)