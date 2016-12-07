project (dbportal_pcexamples)

include(${CMAKE_SOURCE_DIR}/cmake/dbportal.cmake)

pathed(soDir ${CMAKE_CURRENT_BINARY_DIR}/so)
pathed(pcDir ${CMAKE_CURRENT_BINARY_DIR}/pc)

set (switches
  "SO|${soDir}|.so"
)

set (files
  compo
  rewards
)

foreach (name ${files})
  set (siFile ${CMAKE_CURRENT_SOURCE_DIR}/si/${name}.si)
  dbportal_genproc(dbportal_${name} ${siFile} ${switches})
  add_dependencies(dbportal_${name} pocioci genproc)
  proC2c (${name} ${CMAKE_CURRENT_BINARY_DIR}/pc ${CMAKE_CURRENT_BINARY_DIR}/pc)
endforeach()
