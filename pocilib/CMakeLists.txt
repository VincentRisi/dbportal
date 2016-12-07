project (dbportal_pocilib)

set (SRC_DIR ${PORTAL_SOURCE_DIR})

include_directories (
  ${SRC_DIR} 
)

set (source)

set (cpp_list
  binio date fndir fnext fnmerge fnname fnsplit getargs sqlbin sqltok switches xcept xstring xtalloc yyerror
  )

foreach (name ${cpp_list})
  list (APPEND source ${SRC_DIR}/${name}.cpp)
endforeach ()

set (h_list
  addlist binio date dir getargs machine sqlbin sqlcons sqltok xcept xdir xstring xtalloc yyerror
  )

foreach (name ${cpp_list})
  list (APPEND source ${SRC_DIR}/${name}.cpp)
endforeach ()

foreach (name ${h_list})
  list (APPEND source ${SRC_DIR}/${name}.h)
endforeach ()

if (WIN32)
  add_library(poci ${source})
else ()
  add_library(poci SHARED ${source})
endif ()
