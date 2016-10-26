ExternalProject_Add(
    benchpress_external
    GIT_REPOSITORY https://github.com/blockchaindev/benchpress.git
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
    UPDATE_COMMAND ""
)
ExternalProject_Get_Property(benchpress_external source_dir install_dir)

file(MAKE_DIRECTORY "${install_dir}/include")

set(${package_found_prefix}_CMAKE_DEP benchpress_external)
set(${package_found_prefix}_LIBRARIES)
set(${package_found_prefix}_INCLUDE_DIRS "${install_dir}/include")
