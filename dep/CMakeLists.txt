include_directories(include include/intlib)
include_directories(include include/lua)
include_directories(include include/sol)
include_directories(include include/raf)
include_directories(include ../gamed/include)

IF(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  #SET(CMAKE_CXX_FLAGS "-O2 -std=c++11 -pedantic")
  SET(CMAKE_CXX_FLAGS "-g -std=c++11 -pedantic")
ELSEIF(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  SET(CMAKE_CXX_FLAGS "/EHsc") #So it doesn't constantly throw warnings
ELSEIF(CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
  IF(WIN32)
    SET(CMAKE_CXX_FLAGS "/Qstd=c++11")
  ELSE()
    SET(CMAKE_CXX_FLAGS "-std=c++11")
  ENDIF()
ELSEIF(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  SET(CMAKE_CXX_FLAGS "-std=c++11")
ENDIF()

file(GLOB enet_src src/enet/*.c)
file(GLOB intlib_src src/intlib/*.cpp)
file(GLOB lua_src src/lua/*.c)
file(GLOB raf_src src/raf/*.cpp)

file(GLOB enet_include include/enet/*.h)
file(GLOB intlib_include include/intlib/*.h)
file(GLOB lua_include include/lua/*.h include/lua/*.hpp)
file(GLOB raf_include include/raf/*.h)
file(GLOB sol_include include/sol/*.hpp)

add_library(enet ${enet_src} ${enet_include})
add_library(intlib ${intlib_src} ${intlib_include})
add_library(lua ${lua_src} ${lua_include} ${sol_include})
add_library(raf ${raf_src} ${raf_include})
