# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "mzip_config.h.in"
  "mzip_config.h"
)


set(bin_src 
  ${MZIP_SRC_DIR}/mzip.c
)
add_executable(mzip ${bin_src})
target_link_libraries(mzip tk zlib-minizip)
