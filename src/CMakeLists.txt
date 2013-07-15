
set(bin_src 
  ${UNZIP_SRC_DIR}/mzip.c
)
add_executable(unzip ${bin_src})
target_link_libraries(unzip tk)
