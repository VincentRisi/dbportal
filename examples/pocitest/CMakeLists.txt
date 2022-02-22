set (DBPORTAL_OCI_DIR ${CMAKE_SOURCE_DIR}/tools/pocitools)
include(${CMAKE_SOURCE_DIR}/cmake/dbportal.cmake)

pathed(binDir    ${CMAKE_CURRENT_BINARY_DIR}/bin)
pathed(csDir     ${CMAKE_CURRENT_BINARY_DIR}/cs)
pathed(csadoDir  ${CMAKE_CURRENT_BINARY_DIR}/csado)
pathed(csnet7Dir ${CMAKE_CURRENT_BINARY_DIR}/csnet7)
pathed(csrwDir   ${CMAKE_CURRENT_BINARY_DIR}/csrw)
pathed(iiDir     ${CMAKE_CURRENT_BINARY_DIR}/ii)
pathed(logDir    ${CMAKE_CURRENT_BINARY_DIR}/log)
pathed(piDir     ${CMAKE_CURRENT_BINARY_DIR}/pi)
pathed(pyDir     ${CMAKE_CURRENT_BINARY_DIR}/py)
pathed(shDir     ${CMAKE_CURRENT_BINARY_DIR}/sh)
pathed(soDir     ${CMAKE_CURRENT_BINARY_DIR}/so)
pathed(sqlDir    ${CMAKE_CURRENT_BINARY_DIR}/sql)

set (switches
  "OneSQLScript=1"
  "Internal=1"
  "UseCSFields=1"
  "ControlDB=fsrvd00"
  "UConnect=fsrv"
  "LogDir=${logDir}"
  "C|${shDir}|.sh|SH Files|.*[.]sh$"
  "IDL|${iiDir}|.ii|IDL Files|.*[.]ii$"
  "SO|${soDir}|.so"
  "SQL|${sqlDir}|.sql|SQL Files|.*[.]sql$"
  "Python|${pyDir}|.py"
  "CSIDL2|${csDir}|.cs|CS Files|.*[.]cs$"
  #"CSAdoNet|${csadoDir}|.cs"
  #"CSNet7|${csnet7Dir}|.cs"
  #"CSRW|${csrwDir}|.cs"
  "PARMS|${piDir}|.pi|PI Files|.*[.]pi$"
)

set (list
  nednotes
)

set (sifiles)
foreach (name ${list})
  list (APPEND sifiles ${CMAKE_CURRENT_SOURCE_DIR}/${name}.si)
endforeach ()

dbportal(pocitest.dbportal agile "${sifiles}" ${switches})
