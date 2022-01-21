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
