project (dbportal_genproc)

set (SRC_DIR ${PORTAL_SOURCE_DIR})

include_directories (
  ${SRC_DIR} 
)

set (source
  ${SRC_DIR}/genproc.cpp
  )

add_executable(genproc ${source})

target_link_libraries (genproc poci)
