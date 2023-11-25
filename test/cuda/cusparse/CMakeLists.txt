if(USE_CUDA)
    project(cusparse_test LANGUAGES CXX CUDA)

    add_executable(cusparse_test cusparse_test.cu)

    set_target_properties(cusparse_test
        PROPERTIES CUDA_SEPARABLE_COMPILATION ON)
    
    target_link_libraries(cusparse_test 
        PUBLIC cusparse)

    add_test(NAME cusparse_test COMMAND cusparse_test)

endif(USE_CUDA)
