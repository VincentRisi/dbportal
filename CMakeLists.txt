project (dbportal)

cmake_minimum_required (VERSION 2.8)

if (UNIX)
  exec_program (uname OUTPUT_VARIABLE SYSTEM_NAME)
  set (SYSTEM_NAME "${SYSTEM_NAME}" CACHE INTERNAL "")
  if (SYSTEM_NAME STREQUAL "Linux")
    set (LINUX ON)
  elseif (SYSTEM_NAME STREQUAL "FreeBSD")
    set (FREEBSD ON)
  else ()
    set (AIX ON)
  endif ()
endif ()

if (LINUX)
  message (STATUS "LINUX")
  set (switches " -g -fPIC -Wno-write-strings -fpermissive")
  list (APPEND CMAKE_C_FLAGS "${switches}")
  list (APPEND CMAKE_CXX_FLAGS "${switches}")
  string (REPLACE ";" " " CMAKE_C_FLAGS   ${CMAKE_C_FLAGS})
  string (REPLACE ";" " " CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
  set (switches " -pthread")
  list (APPEND CMAKE_EXE_LINKER_FLAGS "${switches}")
  list (APPEND CMAKE_MODULE_LINKER_FLAGS "${switches}")
  list (APPEND CMAKE_SHARED_LINKER_FLAGS "${switches}")
  string (REPLACE ";" " " CMAKE_EXE_LINKER_FLAGS    ${CMAKE_EXE_LINKER_FLAGS})
  string (REPLACE ";" " " CMAKE_MODULE_LINKER_FLAGS ${CMAKE_MODULE_LINKER_FLAGS})
  string (REPLACE ";" " " CMAKE_SHARED_LINKER_FLAGS ${CMAKE_SHARED_LINKER_FLAGS})
elseif (MINGW)
elseif (WIN32)
  message (STATUS "WIN32")
  add_definitions (
    -D_CRT_SECURE_NO_WARNINGS
    -D_SCL_SECURE_NO_WARNINGS
    )
  string (REPLACE "/MD" "/MT" CMAKE_C_FLAGS_DEBUG            ${CMAKE_C_FLAGS_DEBUG})
  string (REPLACE "/MD" "/MT" CMAKE_C_FLAGS_MINSIZEREL       ${CMAKE_C_FLAGS_MINSIZEREL})
  string (REPLACE "/MD" "/MT" CMAKE_C_FLAGS_RELEASE          ${CMAKE_C_FLAGS_RELEASE})
  string (REPLACE "/MD" "/MT" CMAKE_C_FLAGS_RELWITHDEBINFO   ${CMAKE_C_FLAGS_RELWITHDEBINFO})
  string (REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_DEBUG          ${CMAKE_CXX_FLAGS_DEBUG})
  string (REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_MINSIZEREL     ${CMAKE_CXX_FLAGS_MINSIZEREL})
  string (REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_RELEASE        ${CMAKE_CXX_FLAGS_RELEASE})
  string (REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_RELWITHDEBINFO ${CMAKE_CXX_FLAGS_RELWITHDEBINFO})
endif ()

message (STATUS ${CMAKE_CXX_FLAGS})

set (PORTAL_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/code)

find_package (PythonInterp)

set (pocioci  ${CMAKE_BINARY_DIR}/pocioci/pocioci${EXEC_EXT})
set (pocilink ${CMAKE_BINARY_DIR}/pocilink/pocilink${EXEC_EXT})
set (pocidump ${CMAKE_BINARY_DIR}/pocidump/pocidump${EXEC_EXT})
set (genproc  ${CMAKE_BINARY_DIR}/genproc/genproc${EXEC_EXT})

include (${CMAKE_SOURCE_DIR}/cmake/dbportal.cmake)

set (TOOLS_DIR ${CMAKE_BINARY_DIR}/tools)

if (WIN32)
  set (EXEC_EXT ".exe")
else ()
  set (EXEC_EXT)
endif()

file (MAKE_DIRECTORY 
  ${TOOLS_DIR}
  )

add_subdirectory (tools/cocoR/cpp)
add_subdirectory (pocilib)
add_subdirectory (pocioci)
add_subdirectory (pocilink)
add_subdirectory (pocidump)
add_subdirectory (genproc)
add_subdirectory (installs)
add_subdirectory (examples)
add_subdirectory (pcexamples)
