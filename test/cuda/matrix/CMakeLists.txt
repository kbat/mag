    project(matrix LANGUAGES CXX CUDA)

    add_definitions(-D_WITH_CUDA)
    find_package(CUDA REQUIRED)
    # add_library(matmul STATIC
    #   kernel.cu
    #   kernel.h
    # ) 
    # target_compile_features(matmul PUBLIC cxx_std_11)
    # set_target_properties(matmul
    #                       PROPERTIES CUDA_SEPARABLE_COMPILATION ON
    #                      )

    add_executable(matrixmul matrixmul.cu)
    set_target_properties(matrixmul PROPERTIES CUDA_SEPARABLE_COMPILATION ON)
#    target_link_libraries(matrixmul PRIVATE matmul)

    add_test(NAME MatrixMul COMMAND matrixmul )
