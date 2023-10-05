#add_library(kernel kernel.cu)
add_library(kernel kernel.cxx)

#find_package(CUDA REQUIRED)

target_compile_features(kernel PUBLIC cxx_std_11)

#set_target_properties(kernel PROPERTIES CUDA_SEPARABLE_COMPILATION ON)

#anybody linking to kernel needs to include the current source dir, while kernel itself doesn't
target_include_directories(kernel
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
  )

#target_link_libraries(Mag kernel)
