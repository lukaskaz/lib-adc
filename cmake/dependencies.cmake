cmake_minimum_required(VERSION 3.10)

include(ExternalProject)

include(ExternalProject)

set(source_dir "${CMAKE_BINARY_DIR}/libshellcmd-src")
set(build_dir "${CMAKE_BINARY_DIR}/libshellcmd-build")

EXTERNALPROJECT_ADD(
  libshellcmd
  GIT_REPOSITORY    https://github.com/lukaskaz/lib-shellcmd.git
  GIT_TAG           main
  PATCH_COMMAND     ${patching_cmd}
  PREFIX            libshellcmd-workspace
  SOURCE_DIR        ${source_dir}
  BINARY_DIR        ${build_dir}
  CONFIGURE_COMMAND mkdir /${build_dir}/build &> /dev/null
  BUILD_COMMAND     cd ${build_dir}/build && cmake -D BUILD_SHARED_LIBS=ON
                    ${source_dir} && make -j 4
  UPDATE_COMMAND    ""
  INSTALL_COMMAND   "" 
  TEST_COMMAND      ""
)

include_directories(${source_dir}/inc)
link_directories(${build_dir}/build)

set(source_dir "${CMAKE_BINARY_DIR}/libsysfs-src")
set(build_dir "${CMAKE_BINARY_DIR}/libsysfs-build")

EXTERNALPROJECT_ADD(
libsysfs
    GIT_REPOSITORY    https://github.com/lukaskaz/lib-sysfs.git
    GIT_TAG           main
    PATCH_COMMAND     ""
    PREFIX            libsysfs-workspace
    SOURCE_DIR        ${source_dir}
    BINARY_DIR        ${build_dir}
    CONFIGURE_COMMAND mkdir /${build_dir}/build &> /dev/null
    BUILD_COMMAND     cd ${build_dir}/build && cmake -D BUILD_SHARED_LIBS=ON
                        ${source_dir} && make -j 4
    UPDATE_COMMAND    ""
    INSTALL_COMMAND   ""
    TEST_COMMAND      ""
)
include_directories(${source_dir}/inc)
link_directories(${build_dir}/build)

if(NOT (EXISTS ${CMAKE_BINARY_DIR}/liblogger-src
    OR EXISTS ${CMAKE_BINARY_DIR}/liblogger-build))
    execute_process(
        COMMAND ln -s ${build_dir}/build/liblogger-src ${CMAKE_BINARY_DIR}
        COMMAND ln -s ${build_dir}/build/liblogger-build ${CMAKE_BINARY_DIR}
    )
endif()

set(source_dir "${CMAKE_BINARY_DIR}/liblogger-src")
set(build_dir "${CMAKE_BINARY_DIR}/liblogger-build")
include_directories(${source_dir}/inc)
link_directories(${build_dir}/build)
