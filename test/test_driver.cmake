# Custom test target to run the googletest tests
add_custom_target(check)
macro(only_run_test test_target)
  add_custom_target(${test_target}_runtest
      COMMAND ${test_target} #cmake 2.6 required
      DEPENDS ${test_target}
      WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
  add_dependencies(check ${test_target}_runtest)
endmacro()

# will compile and run ${test_target}.cpp
# and add all further arguments as dependencies
macro(run_test test_target)
    add_executable(${test_target}_testrunner
        test_driver.cpp
        ${test_target}.cpp
    )
    target_link_libraries(${test_target}_testrunner
        libgtest
        #libgtest_main
        ${ARGN}
        ${GLOG_LIBRARIES}
    )
    only_run_test(${test_target}_testrunner)
endmacro()

# We need thread support
find_package(Threads REQUIRED)

# Enable ExternalProject CMake module
include(ExternalProject)

# Download and install GoogleTest
ExternalProject_Add(
    gtest
    SVN_REPOSITORY http://googletest.googlecode.com/svn/trunk/
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/gtest
    # Disable install and update step
    UPDATE_COMMAND ""
    INSTALL_COMMAND ""
)

# Create a libgtest target to be used as a dependency by test programs
add_library(libgtest IMPORTED STATIC GLOBAL)
add_dependencies(libgtest gtest)
add_library(libgtest_main IMPORTED STATIC GLOBAL)
add_dependencies(libgtest_main gtest)

# Set gtest properties
ExternalProject_Get_Property(gtest source_dir binary_dir)
set_target_properties(libgtest PROPERTIES
    "IMPORTED_LOCATION" "${binary_dir}/libgtest.a"
    "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}"
    #"INTERFACE_INCLUDE_DIRECTORIES" "${source_dir}/include"
)
set_target_properties(libgtest_main PROPERTIES
    "IMPORTED_LOCATION" "${binary_dir}/libgtest_main.a"
    "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}"
    #"INTERFACE_INCLUDE_DIRECTORIES" "${source_dir}/include"
)
include_directories("${source_dir}/include")
