option(USE_CUDA "Build with cuda support" OFF)

if(USE_CUDA)
    add_definitions(-D_WITH_CUDA)
    find_package(CUDA REQUIRED)

    add_subdirectory(particle)
    add_subdirectory(cusparse)
endif(USE_CUDA)
