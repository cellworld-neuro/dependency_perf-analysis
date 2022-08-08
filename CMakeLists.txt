cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)

project(perf-analysis
        VERSION 2022.1.0
        DESCRIPTION "c++ Performance analysis"
        LANGUAGES CXX)

add_library(perf-analysis src/performance.cpp)

set_target_properties(perf-analysis
        PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED On
        CXX_EXTENSIONS Off
        VERSION ${PROJECT_VERSION})

if(MSVC)
    target_compile_options(perf-analysis PRIVATE /W4)
else(MSVC)
    target_compile_options(perf-analysis PRIVATE -Wall -Wextra -pedantic)
endif(MSVC)

target_include_directories(perf-analysis
        SYSTEM INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>)

target_include_directories(perf-analysis
        PRIVATE
        include)

###
### LIBRARY INSTALLATION
###

export(TARGETS perf-analysis FILE PerfAnalysisConfig.cmake)

include(GNUInstallDirs)

install(TARGETS perf-analysis EXPORT PerfAnalysisConfig
        ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR})

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT PerfAnalysisConfig
        DESTINATION ${CMAKE_INSTALL_DATADIR}/cmake/PerfAnalysis
        EXPORT_LINK_INTERFACE_LIBRARIES)

export(TARGETS perf-analysis FILE PerfAnalysisConfig.cmake)