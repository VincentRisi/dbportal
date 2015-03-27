project (dbportal_pocilib)

set (SRC_DIR ${PORTAL_SOURCE_DIR})

include_directories (
  ${SRC_DIR} 
)

set (source
  ${SRC_DIR}/dumpmain.cpp
  )

add_executable(pocidump ${source})

target_link_libraries (pocidump poci)
