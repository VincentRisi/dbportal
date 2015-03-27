project (dbportal_pocioci)

set (BIN_DIR ${CMAKE_CURRENT_BINARY_DIR})
set (SRC_DIR ${PORTAL_SOURCE_DIR})

include_directories (
  ${BIN_DIR}
  ${SRC_DIR} 
)

set (sqldefOut      
  ${BIN_DIR}/sqldef.cpp 
  ${BIN_DIR}/sqldef.h
  ${BIN_DIR}/yytok.cpp
)

set (sqldefIn      
  ${SRC_DIR}/sqldef.y 
)

add_custom_command (OUTPUT ${sqldefOut}
  COMMAND ${SRC_DIR}/build.py ${SRC_DIR} ${BIN_DIR}
  DEPENDS ${sqldefIn} 
  )  

add_custom_target (dbportal_sqldef_gen ALL
  DEPENDS ${sqldefOut}
  )

set (source
  ${sqldefIn}   
  ${sqldefOut}
  )

set (cpp_list
  cleanup genarchive gencommn gencsadonet gencsidl2 gencsnet7 gencsrw genidl
  genoci genparm genpython genvbcode3 genvbforador genvbforidl genvbnet7 lex
  pocimain
)

foreach (name ${cpp_list})
  list (APPEND source ${SRC_DIR}/${name}.cpp)
endforeach ()

add_executable(pocioci ${source})

target_link_libraries (pocioci poci)
