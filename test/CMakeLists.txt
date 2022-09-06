#
# Copyright (C) 2022 Nikolas Koesling <nikolas@koesling.info>.
# This program is free software. You can redistribute it and/or modify it under the terms of the MIT License.
#

add_executable(test_${Target} test.cpp)
target_link_libraries(test_${Target} ${Target})
add_test(test_${Target} test_${Target})
enable_warnings(test_${Target})
set_definitions(test_${Target})
if(CLANG_FORMAT_ENABLED)
    target_clangformat_setup(test_${Target})
endif()

add_executable(test_wait_${Target} test_wait.cpp)
target_link_libraries(test_wait_${Target} ${Target})
add_test(test_wait_${Target} test_wait_${Target})
enable_warnings(test_wait_${Target})
set_definitions(test_wait_${Target})
if(CLANG_FORMAT_ENABLED)
    target_clangformat_setup(test_wait_${Target})
endif()
