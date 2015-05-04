project (dbportal_pcexamples)

include(${CMAKE_SOURCE_DIR}/cmake/dbportal.cmake)

pathed(soDir ${CMAKE_CURRENT_BINARY_DIR}/so)
pathed(pcDir ${CMAKE_CURRENT_BINARY_DIR}/pc)

set (switches 
  "SO|${soDir}|.so"
)

file (GLOB siFiles ${CMAKE_CURRENT_SOURCE_DIR}/si/*.si)

dbportal_genproc(dbportal_pcexamples "${siFiles}" ${switches})

add_dependencies(dbportal_pcexamples pocioci genproc)
