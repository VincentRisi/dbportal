project (dbportal_examples_large)

include(${CMAKE_SOURCE_DIR}/cmake/dbportal.cmake)

pathed(iiDir     ${CMAKE_CURRENT_BINARY_DIR}/ii) 
pathed(shDir     ${CMAKE_CURRENT_BINARY_DIR}/sh)
pathed(soDir     ${CMAKE_CURRENT_BINARY_DIR}/so)
pathed(sqlDir    ${CMAKE_CURRENT_BINARY_DIR}/sql)
pathed(pyDir     ${CMAKE_CURRENT_BINARY_DIR}/py)
pathed(csDir     ${CMAKE_CURRENT_BINARY_DIR}/cs)
pathed(csadoDir  ${CMAKE_CURRENT_BINARY_DIR}/cs/ado)
pathed(csnet7Dir ${CMAKE_CURRENT_BINARY_DIR}/cs/7)
pathed(csrwDir   ${CMAKE_CURRENT_BINARY_DIR}/cs/rw)
pathed(piDir     ${CMAKE_CURRENT_BINARY_DIR}/pi)
pathed(binDir    ${CMAKE_CURRENT_BINARY_DIR}/bin)
pathed(logDir    ${CMAKE_CURRENT_BINARY_DIR}/log/)

set (switches 
  "OneSQLScript=1"
  "Internal=1"
  "IDLUbi=1"
  "ControlDB=lexadm"
  "UConnect=lex"
  "IDL|${iiDir}|.ii|IDL Files|.*[.]ii$"
  "C|${shDir}|.sh|SH Files|.*[.]sh$"  
  "SO|${soDir}|.so"
  "SQL|${sqlDir}|.sql|SQL Files|.*[.]sql$"
  "LogDir=${logDir}"
  "PythonDir=${pyDir}"
  "CSIDL2Dir=${csDir}"
  "CSAdoNet|${csadoDir}|.cs|CS Files|.*[.]cs$"
  "CSNet7Dir=${csnet7Dir}"
  "CSRWDir=${csrwDir}"
  "PARMSDir=${piDir}"
)

file (GLOB siFiles ${CMAKE_CURRENT_SOURCE_DIR}/si/*.si)

dbportal(dbportal_examples_large examples "${siFiles}" ${switches})

add_dependencies(dbportal_examples_large pocioci pocilink)
