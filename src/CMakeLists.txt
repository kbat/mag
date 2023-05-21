cmake_minimum_required(VERSION 3.6.0)

project(gam)
add_library(Gam STATIC
        functions.cxx
        Material.cxx
        OptArguments.cxx
        Optimiser.cxx
        SDEF.cxx
        SolverArguments.cxx
        Solver.cxx
        Source.cxx
        Test.cxx
)

add_executable(gam-solve
        solve.cxx
)

target_link_libraries(gam-solve
        Gam
        TBB::tbb
        ROOT::Geom
        Boost::program_options
)

add_executable(gam
        gam.cxx
)

target_link_libraries(gam
        Gam
        TBB::tbb
        ROOT::Geom
        Boost::program_options
)



find_program(MEMORYCHECK_COMMAND "valgrind" )

if(MEMORYCHECK_COMMAND)
    set(MEMORYCHECK_COMMAND_OPTIONS --tool=memcheck --smc-check=all-non-file --error-limit=no --leak-check=yes --num-callers=2 --leak-check=full --track-origins=yes --show-leak-kinds=all --suppressions=${ROOTETC}/valgrind-root.supp )
    set(MEMORYCHECK_TARGET_OPTIONS -test 2 1)
    add_custom_target(valgrind
        COMMAND "${MEMORYCHECK_COMMAND}" ${MEMORYCHECK_COMMAND_OPTIONS} $<TARGET_FILE:gam-solve> ${MEMORYCHECK_TARGET_OPTIONS})
endif()