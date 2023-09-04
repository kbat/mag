option(USE_CUDA "Build with cuda support" OFF)

if(USE_CUDA)
    project(particle_test LANGUAGES CXX CUDA)

    add_definitions(-D_WITH_CUDA)
    find_package(CUDA REQUIRED)

    add_library(particles STATIC
        randomize.cpp
        randomize.h
        particle.cu
        particle.h
        v3.cu
        v3.h
    )

    target_compile_features(particles PUBLIC cxx_std_11)

    set_target_properties(particles
                        PROPERTIES CUDA_SEPARABLE_COMPILATION ON
                        )

    add_executable(particle_test test.cu)

    set_target_properties(particle_test PROPERTIES CUDA_SEPARABLE_COMPILATION ON)
    target_link_libraries(particle_test PRIVATE particles)

    add_test(NAME particles_10k COMMAND particle_test 10000 )
    add_test(NAME particles_256k COMMAND particle_test 256000 )

endif(USE_CUDA)
