
####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was InfluxDBConfig.cmake.in                            ########

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)

macro(set_and_check _var _file)
  set(${_var} "${_file}")
  if(NOT EXISTS "${_file}")
    message(FATAL_ERROR "File or directory ${_file} referenced by variable ${_var} does not exist !")
  endif()
endmacro()

macro(check_required_components _NAME)
  foreach(comp ${${_NAME}_FIND_COMPONENTS})
    if(NOT ${_NAME}_${comp}_FOUND)
      if(${_NAME}_FIND_REQUIRED_${comp})
        set(${_NAME}_FOUND FALSE)
      endif()
    endif()
  endforeach()
endmacro()

####################################################################################

set(InfluxDB_VERSION 0.8.0)
set(InfluxDB_WITH_BOOST ON)

get_filename_component(InfluxDB_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
include(CMakeFindDependencyMacro)

if(InfluxDB_WITH_BOOST)
  find_dependency(Boost COMPONENTS system REQUIRED)
endif()
find_dependency(cpr REQUIRED)
find_dependency(Threads REQUIRED)

if(NOT TARGET InfluxData::InfluxDB)
  include("${InfluxDB_CMAKE_DIR}/InfluxDBTargets.cmake")
endif()

message(STATUS "InfluxDB ${InfluxDB_VERSION} found")
