add_subdirectory(bins)
add_subdirectory(solver)
add_subdirectory(cuda)

#-------------------------------------------------------------------------------
# Google Test
#-------------------------------------------------------------------------------
if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/googletest/CMakeLists.txt)
    add_subdirectory(googletest)

    set(TESTFILES
            ut/SolverArguments.test.cpp
    )

    foreach (test_cpp ${TESTFILES})
        get_filename_component(test ${test_cpp} NAME_WE)
        set(name ${test}.test)
        add_executable(${name} ${test_cpp})
        target_link_libraries(${name}
            Mag
            TBB::tbb
            ROOT::Geom
            Boost::program_options
            GTest::gtest_main
        )
        target_include_directories(${name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
        add_test(NAME ${name} COMMAND ${name})
        list(APPEND ALL_TESTS ${name})
    endforeach()

    add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} DEPENDS ${ALL_TESTS})
else()
    message(WARNING "No googletest found in subdirectory. Git update submodules needed. Skipping GoogleTests.")
endif()
