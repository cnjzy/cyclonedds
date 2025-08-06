cmake_minimum_required(VERSION 3.8)
project(ddsos VERSION 0.0.1)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
add_library(ddsos STATIC)

message(STATUS "CMAKE_SYSTEM_NAME == ${CMAKE_SYSTEM_NAME}")

aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/common SOURCE_COMMON)
target_include_directories(ddsos PUBLIC ${CMAKE_CURRENT_LIST_DIR})
message(STATUS "COMMON SOURCE == ${SOURCE_COMMON}")

if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/windows SOURCE_PLATFORM)
elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/linux SOURCE_PLATFORM)
	target_link_libraries(ddsos pthread)
else ()
	aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/acore3 SOURCE_PLATFORM)
endif ()

message(STATUS "PLATFORM SOURCE == ${SOURCE_PLATFORM}")
target_sources(ddsos PRIVATE ${SOURCE_COMMON} ${SOURCE_PLATFORM})
target_include_directories(ddsos PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)

