cmake_minimum_required (VERSION 3.16)
project(RollbackGame)

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)


option(Gpr_Assert "Activate Assertion" ON)
option(Gpr_Abort "Activate Assertion with std::abort" OFF)
option(Gpr_Exit_On_Warning "Exit on Warning Assertion" ON)
option(ENABLE_PROFILING "Enable Tracy Profiling" OFF)
option(ENABLE_SQLITE_STORE "Enable info storing in sqlite" OFF)

include(cmake/data.cmake)

if (MSVC)
    # warning level 4 
    add_compile_options(/W4 /w14640 /permissive-)
    add_compile_definitions(_USE_MATH_DEFINES)
else()
    # lots of warnings
    add_compile_options(-Wall -Wextra -Wshadow -Wnon-virtual-dtor -pedantic)
endif()
if(ENABLE_PROFILING)
    add_subdirectory(externals/tracy)
endif()
add_subdirectory(core)
add_subdirectory(game)
