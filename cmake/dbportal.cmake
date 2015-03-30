function (pathed result ext_dir)
  foreach (arg ${ARGN})
    if (${ARGN} STREQUAL REMOVE)
      file (GLOB remFiles ${ext_dir}/*.*)
      list (LENGTH remFiles count)
      if (0 LESS count) 
        message (STATUS "Removing all files in ${ext_dir}")
        file (REMOVE ${remFiles})
      endif ()
    endif ()
  endforeach ()
  file(MAKE_DIRECTORY ${ext_dir})
  set ("${result}" ${ext_dir} PARENT_SCOPE)
endfunction ()

# -- splits db portal switches based on the following rules
#  1: switch                     -  "OneSQLScript=1"
#  2: target|dir|ext             -  "SO|${soDir}|.so"
#  3: target|dir|ext|group|mask  -  "C|${shDir}|.sh|SH Files|.*[.]sh$" 
#     C|${shDir}|.sh      
#     is changed to TargetC=1 CDir=${shDir} CExt=.sh 
function (dbportal_split target_args)
  list (LENGTH target_args count)
  if (4 LESS count)
    list (GET target_args 3 arg)
    set (title ${arg})
    list (GET target_args 4 arg)
    set (mask ${arg})
  endif ()  
  if (2 LESS count)
    list (GET target_args 0 arg)
    set (target "Target${arg}=1")
    set (name ${arg})
    list (GET target_args 1 arg)
    set (targetdir "${name}Dir=${arg}")
    set (dir ${arg})
    list (GET target_args 2 arg)
    set (targetext "${name}Ext=${arg}")
    string (SUBSTRING ${arg} 1 -1 ext)
  endif ()  
  set (groups PARENT_SCOPE)
  if (1 EQUAL count)
    set (switch ${target_args}            PARENT_SCOPE)
  elseif (3 EQUAL count)
    set (switch "${target};${targetdir};${targetext}" PARENT_SCOPE)
    set (groups "${dir};${ext}"                       PARENT_SCOPE)
  elseif (5 EQUAL count)
    set (switch "${target};${targetdir};${targetext}" PARENT_SCOPE)
    set (groups "${title};${mask};${dir};${ext}"      PARENT_SCOPE)
  endif ()
endfunction ()

function (dbportal projectName binFilename siFiles)
  source_group ("SI Files"  REGULAR_EXPRESSION ".*[.]si$")
  set (direxts)
  # We will store switches to file to avoid over long command line
  set (switchesList ${CMAKE_CURRENT_BINARY_DIR}/${projectName}.switches)
  file (WRITE ${switchesList} "")
  foreach (arg ${ARGN})
    string(REPLACE "|" ";" arg2 ${arg})
    dbportal_split("${arg2}")
    list (LENGTH groups noOf)
    if (3 LESS noOf)
      # this case has VS source group plus dir ext files
      list (GET groups 0 title)
      list (GET groups 1 mask)
      source_group ("${title}" REGULAR_EXPRESSION "${mask}")
      list (GET groups 2 dir)
      list (GET groups 3 ext)
      list (APPEND direxts "${dir}|${ext}")
    elseif (1 LESS noOf)
      # this case dir ext files
      list (GET groups 0 dir)
      list (GET groups 1 ext)
      list (APPEND direxts "${dir}|${ext}")
    endif ()
    # switch may be multipart with semicolon separates
    foreach (sw ${switch})
      file (APPEND ${switchesList} "${sw}\n")
    endforeach ()  
  endforeach ()
  set (allFiles)
  set (soFiles)
  foreach (siFile ${siFiles})
    get_filename_component (temp ${siFile} NAME)
    string (TOLOWER ${temp} temp1)
    get_filename_component (filename ${temp1} NAME_WE)
    set (oFiles)
    foreach (arg ${direxts})
      string(REPLACE "|" ";" arg2 ${arg})
      list (GET arg2 0 dir)
      list (GET arg2 1 ext)
      set (oFile ${dir}/${filename}.${ext})
      if (ext STREQUAL "so")
        list (APPEND soFiles  "${oFile}")
        list (APPEND oFiles   "${oFile}")
      else ()
        list (APPEND allFiles "${oFile}")
        list (APPEND oFiles   "${oFile}")
      endif ()  
    endforeach()
    add_custom_command (
      OUTPUT  ${oFiles}
      COMMAND ${pocioci} ${siFile} "-s${switchesList}"
      DEPENDS ${siFile}
      VERBATIM
    )
  endforeach()
  list (LENGTH soFiles noOf)
  if (0 LESS noOf)
    set (soFileList ${CMAKE_CURRENT_BINARY_DIR}/${projectName}.sofiles)
    file (WRITE ${soFileList} "")
    foreach (soFile ${soFiles})
      file (APPEND ${soFileList} "${soFile}\n")
    endforeach()
    set (binFile ${binDir}/${binFilename}.bin)
    add_custom_command (
      OUTPUT  ${binFile}
      COMMAND ${pocilink} -f${soFileList} -b${binDir} -n${binFilename}
      DEPENDS ${soFiles} ${siFiles}
    )
    add_custom_target (${projectName} ALL
      DEPENDS ${binFile}
      SOURCES ${siFiles} ${allFiles}
    )
  else ()
    add_custom_target (${projectName} ALL
      DEPENDS ${allFiles}
      SOURCES ${siFiles} ${allFiles}
    )
  endif ()  
  set_source_files_properties (
    ${allFiles}
    PROPERTIES GENERATED TRUE
  )
endfunction()

function (jportal projectName siFiles)
  source_group ("SI Files"  REGULAR_EXPRESSION ".*[.]si$")
  set (switches)
  foreach (arg ${ARGN})
    list(APPEND switches "${arg}")
  endforeach ()
  set (sqlFiles)
  set (jportalJar ${TOOLS_DIR}/anydb/jportal.jar)
  foreach (siFile ${siFiles})
    get_filename_component (temp ${siFile} NAME)
    string (TOLOWER ${temp} temp1)
    get_filename_component (filename ${temp1} NAME_WE)
    set (sqlFile ${sqlDir}/${filename}.sql)
    list (APPEND sqlFiles ${sqlFile})
    add_custom_command (
      OUTPUT  ${sqlFile}
      COMMAND java -jar ${jportalJar} ${siFile} ${switches}
      DEPENDS ${siFile}
      VERBATIM
    )
  endforeach ()
  set (sqlFiles ${sqlFiles} PARENT_SCOPE)
  add_custom_target (${projectName} ALL
    DEPENDS ${sqlFiles} 
    SOURCES ${siFiles}
  )
endfunction()

function (idl2 projectName imFile iiDir ibDir)
  source_group ("IDL Source"  REGULAR_EXPRESSION ".*[.](im|ib)$")
  source_group ("IDL Built"   REGULAR_EXPRESSION ".*[.](ii|idl|idl2)$")
  set (switches)
  foreach (arg ${ARGN})
    list(APPEND switches "${arg}")
  endforeach ()
  get_filename_component(temp ${imFile} NAME)
  get_filename_component(filename ${temp} NAME_WE)
  set (idlFile ${CMAKE_CURRENT_BINARY_DIR}/idl/${filename}.idl2)
  set (crackleJar ${TOOLS_DIR}/idl2/crackle.jar)
  file (GLOB ibFiles ${ibDir}/*.ib)
  file (GLOB iiFiles ${iiDir}/*.ii)
  add_custom_command(
    OUTPUT ${idlFile}
    COMMAND java -jar ${crackleJar} -i ${iiDir} -b ${ibDir} -f ${idlFile} ${imFile} ${switches}
    DEPENDS ${imFile} ${ibFiles} ${iiFiles}
    VERBATIM
  )
  add_custom_target (${projectName} ALL
    DEPENDS ${idlFile} 
    SOURCES ${imFile} ${ibFiles} ${iiFiles} ${idlFile}
  )
endfunction()

function (idl2_simple projectName idlFile)
  source_group ("IDL Source"  REGULAR_EXPRESSION ".*[.](idl|idl2)$")
  set (crackleJar ${TOOLS_DIR}/idl2/crackle.jar)
  set (switches)
  foreach (arg ${ARGN})
    list(APPEND switches "${arg}")
  endforeach ()
  set (simpleTarget ${CMAKE_CURRENT_BINARY_DIR}/${projectName})
  if (SHOW_COMMAND)
    message (STATUS "COMMAND java -jar ${crackleJar} ${idlFile} ${switches}")
    message (STATUS "COMMAND echo \"built\" > ${simpleTarget}")
  endif ()
  add_custom_command(
    OUTPUT ${simpleTarget}
    COMMAND java -jar ${crackleJar} ${idlFile} ${switches}
    COMMAND echo "built" > ${simpleTarget}
    DEPENDS ${idlFile}
    VERBATIM
  )
  add_custom_target (${projectName} ALL
    DEPENDS ${simpleTarget}
    SOURCES ${idlFile}
  )
endfunction ()

function (pickle projectName pmFile piDir prDir)
  source_group ("PICKLE Source"  REGULAR_EXPRESSION ".*[.](pm|pr)$")
  source_group ("PICKLE Built"   REGULAR_EXPRESSION ".*[.](pi|pickle)$")
  set (switches)
  foreach (arg ${ARGN})
    list(APPEND switches "${arg}")
  endforeach ()
  get_filename_component(temp ${pmFile} NAME)
  get_filename_component(filename ${temp} NAME_WE)
  set (pickleFile ${CMAKE_CURRENT_BINARY_DIR}/pickle/${filename}.pickle)
  set (pickleJar ${TOOLS_DIR}/pickle/pickle.jar)
  file (GLOB prFiles ${prDir}/*.pr)
  file (GLOB piFiles ${piDir}/*.pi)
  add_custom_command(
    OUTPUT ${pickleFile}
    COMMAND java -jar ${pickleJar} -p ${piDir} -r ${prDir} -f ${pickleFile} ${pmFile} ${switches}
    DEPENDS ${pmFile} ${piFiles} ${prFiles}
    VERBATIM
  )
  add_custom_target (${projectName} ALL
    DEPENDS ${pickleFile}
    SOURCES ${pmFile} ${piFiles} ${prFiles} ${pickleFile}
  )
endfunction()

function (lite3sql dbFile sqlFiles)
  source_group ("SQL Files"  REGULAR_EXPRESSION ".*[.](sql)$")
  get_filename_component(dbDir  ${dbFile} PATH)
  get_filename_component(dbName ${dbFile} NAME)
  set (batchFile ${dbDir}/build.bat)
  file (WRITE ${batchFile} "rem - build of lite3\n")
  set (lite3SQL ${TOOLS_DIR}/sqllite3/lite3SQL.py)
  foreach (sqlFile ${sqlFiles})
    get_filename_component(temp ${sqlFile} NAME)
    get_filename_component(tableName ${temp} NAME_WE)
    set (logName ${dbDir}/${tableName}.log)
    set (cmd "${PYTHON_EXECUTABLE} ${lite3SQL} -d ${dbName} ${sqlFile}")
    file (APPEND ${batchFile} "${cmd}\n")
  endforeach()
endfunction ()

function (install_file files destdir)
  file (INSTALL ${files} DESTINATION ${destdir} FILE_PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ GROUP_WRITE WORLD_READ)
endfunction ()  

function (install_exec files destdir)
  file (INSTALL ${files} DESTINATION ${destdir} FILE_PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ)
endfunction ()  
