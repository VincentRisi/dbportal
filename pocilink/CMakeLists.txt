project (dbportal_pocilib)

set (SRC_DIR ${PORTAL_SOURCE_DIR})

include_directories (
  ${SRC_DIR} 
)

set (source
  ${CMAKE_BINARY_DIR}/pocioci/yytok.cpp
  )

set (cpp_list
  linkmain slink
  )

foreach (name ${cpp_list})
  list (APPEND source ${SRC_DIR}/${name}.cpp)
endforeach ()

add_executable(pocilink ${source})

target_link_libraries (pocilink poci)
