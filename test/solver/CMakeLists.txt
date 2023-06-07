add_test(ctest_build_mag_solve
         "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} --target mag-solve)
add_test(NAME ctest_test_mag_solve COMMAND root -b -q  ${CMAKE_SOURCE_DIR}/test/solver/tests.C)
set_tests_properties(ctest_test_mag_solve
                     PROPERTIES DEPENDS ctest_build_mag_solve)
