
add_custom_target(test.pstudy COMMAND rm -fr case001 && pstudy -i ${CMAKE_SOURCE_DIR}/test/bins/test.pstudy -setup -run)
add_custom_target(mctal COMMAND ${CMAKE_SOURCE_DIR}/test/bins/test.sh)
