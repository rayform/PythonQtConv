cmake_minimum_required(VERSION 3.4)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(PythonQtConv)

find_package(Qt6 COMPONENTS Core REQUIRED)
find_package(CPython REQUIRED)

set(HEADERS src/PythonQtConversion.h src/PythonQtPythonInclude.h)
add_library(${PROJECT_NAME} SHARED src/PythonQtConversion.cpp)

get_target_property(Qt6_INCLUDES Qt6::Core INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(cpython_INCLUDES cpython::python INTERFACE_INCLUDE_DIRECTORIES)

target_include_directories(${PROJECT_NAME} PRIVATE ${Qt6_INCLUDES} ${cpython_INCLUDES})

set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${HEADERS}")

if(APPLE)
	set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-fvisibility=default")
	set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-undefined dynamic_lookup -dead_strip")
endif()

install(TARGETS ${PROJECT_NAME} PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})