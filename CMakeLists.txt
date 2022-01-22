cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(CAENReadout CXX)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")

find_package(ROOT)
find_package(CAENDigitizer)
find_package(CAENVME)
find_package(Boost COMPONENTS program_options container)
# The CAENlib need define LINUX to include stdint.h
add_definitions(-DLINUX)

add_executable(Readout src/main.cxx src/readout.cxx)
target_include_directories(Readout PUBLIC include)
# https://root.cern/manual/integrate_root_into_my_cmake_project/
target_include_directories(Readout PUBLIC ${ROOT_INCLUDE_DIRS})
target_include_directories(Readout PUBLIC ${CAENDigitizer_INCLUE_DIR} ${CAENVME_INCLUDE_DIRS})
target_include_directories(Readout PUBLIC ${BOOST_INCLUDE_DIRS})

target_link_libraries(Readout ${ROOT_LIBRARIES})
target_link_libraries(Readout ${CAENDigitizer_LIBRARY} ${CAENVME_LIBRARY})
target_link_libraries(Readout Boost::program_options Boost::container)
install(TARGETS Readout RUNTIME DESTINATION bin)

add_subdirectory(pybind11)
pybind11_add_module(ReadoutPB src/readoutBind.cpp src/readout.cxx)
target_include_directories(ReadoutPB PUBLIC include)
target_include_directories(ReadoutPB PUBLIC ${ROOT_INCLUDE_DIRS})
target_include_directories(ReadoutPB PUBLIC ${CAENDigitizer_INCLUE_DIR} ${CAENVME_INCLUDE_DIRS})
target_link_libraries(ReadoutPB PRIVATE ${ROOT_LIBRARIES})
target_link_libraries(ReadoutPB PRIVATE ${CAENDigitizer_LIBRARY} ${CAENVME_LIBRARY})
# EXAMPLE_VERSION_INFO is defined by setup.py and passed into the C++ code as a
# define (VERSION_INFO) here.
target_compile_definitions(ReadoutPB
                           PRIVATE VERSION_INFO=${EXAMPLE_VERSION_INFO})