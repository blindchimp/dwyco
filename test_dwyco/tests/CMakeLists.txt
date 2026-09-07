add_executable(list_test_suite_part1 list_test_suite_part1.cpp)
target_include_directories(list_test_suite_part1 PRIVATE ../bld/cdc32 ../bld/vc ../bld/dwcls)
target_link_libraries(list_test_suite_part1 PRIVATE ${BASELIBS})

add_executable(test_append_operations test_append_operations.cpp)
target_include_directories(test_append_operations PRIVATE ../bld/cdc32 ../bld/vc ../bld/dwcls ../bld/kazlib ../bld/jenkins ../bld/zlib ../bld/crypto8)
target_link_libraries(test_append_operations PRIVATE ${BASELIBS})

add_executable(test_creation_destruction test_creation_destruction.cpp)
target_include_directories(test_creation_destruction PRIVATE ../bld/cdc32 ../bld/vc ../bld/dwcls ../bld/kazlib ../bld/jenkins ../bld/zlib ../bld/crypto8)
target_link_libraries(test_creation_destruction PRIVATE ${BASELIBS})

add_executable(test_lists_12 test_lists_12.cpp)
target_include_directories(test_lists_12 PRIVATE ../bld/cdc32 ../bld/vc ../bld/dwcls ../bld/kazlib ../bld/jenkins ../bld/zlib ../bld/crypto8)
target_link_libraries(test_lists_12 PRIVATE ${BASELIBS})
