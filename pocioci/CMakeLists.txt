project (dbportal_pocioci)

set (SRC_DIR ${PORTAL_SOURCE_DIR})
set (BIN_DIR ${CMAKE_CURRENT_BINARY_DIR})

include_directories (
  ${BIN_DIR}
  ${SRC_DIR} 
)

set (sqldefOut      
  ${BIN_DIR}/sqldef.cpp 
  ${BIN_DIR}/sqldef.h
  ${BIN_DIR}/yytok.cpp
)

set_source_files_properties(${sqldefOut} PROPERTIES GENERATED TRUE)

set (sqldefIn      
  ${SRC_DIR}/sqldef.y 
)

if (WIN32)
add_custom_command (OUTPUT ${sqldefOut}
  COMMAND ${PYTHON_EXECUTABLE} ${SRC_DIR}/build.py ${SRC_DIR} ${BIN_DIR}
  DEPENDS ${sqldefIn}
  )
elseif (LINUX)
add_custom_command (OUTPUT ${sqldefOut}
  COMMAND ${SRC_DIR}/build.py ${SRC_DIR} ${BIN_DIR}
  DEPENDS ${sqldefIn} 
  )  
endif ()

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

add_executable (pocioci ${source} ${SRC_DIR}/pocioci.cpp)
set_target_properties (pocioci PROPERTIES PROJECT_LABEL dbportal_pocioci)
target_link_libraries (pocioci poci)

add_executable (pocioci_sw ${source} ${SRC_DIR}/pocioci_sw.cpp)
set_target_properties (pocioci_sw PROPERTIES PROJECT_LABEL dbportal_pocioci_sw)
target_link_libraries (pocioci_sw poci)
