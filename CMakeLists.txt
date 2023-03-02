cmake_minimum_required(VERSION 3.4)
project(pythonqtconv LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 COMPONENTS Core REQUIRED)
find_package(Python3 REQUIRED COMPONENTS Development)

add_library(${PROJECT_NAME} SHARED src/PythonQtConversion.cpp)
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "src/PythonQtConversion.h;src/PythonQtPythonInclude.h")

target_compile_definitions(${PROJECT_NAME} PRIVATE Py_LIMITED_API=0x03020000)

target_include_directories(${PROJECT_NAME} PRIVATE ${Python3_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core)

if(APPLE)
	set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-fvisibility=default")
	set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-undefined dynamic_lookup -dead_strip")
endif()

install(TARGETS ${PROJECT_NAME} PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR} EXPORT ${PROJECT_NAME}Config)
install(EXPORT ${PROJECT_NAME}Config DESTINATION ${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME})
