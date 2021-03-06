# Author: petter.strandmark@gmail.com (Petter Strandmark)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0)
PROJECT(RAPIDSVG C CXX)

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)

# Default locations to search for on various platforms.
LIST(APPEND SEARCH_LIBS /usr/lib)
LIST(APPEND SEARCH_LIBS /usr/local/lib)
LIST(APPEND SEARCH_LIBS /usr/local/homebrew/lib) # Mac OS X
LIST(APPEND SEARCH_LIBS /opt/local/lib)

LIST(APPEND SEARCH_HEADERS /usr/include)
LIST(APPEND SEARCH_HEADERS /usr/local/include)
LIST(APPEND SEARCH_HEADERS /usr/local/homebrew/include) # Mac OS X
LIST(APPEND SEARCH_HEADERS /opt/local/include)

# Multithreading using OpenMP
OPTION(OPENMP
       "Enable multi-threading (requires OpenMP)"
       ON)

IF (${OPENMP})
  FIND_PACKAGE(OpenMP)
  IF(${OPENMP_FOUND})
    MESSAGE("-- Found OpenMP.")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    ADD_DEFINITIONS(-DUSE_OPENMP)
  ELSE(${OPENMP_FOUND})
    MESSAGE("-- Can't find OpenMP. Continuing without it.")
  ENDIF(${OPENMP_FOUND})
ENDIF (${OPENMP})


INCLUDE_DIRECTORIES(
  thirdparty/rapidxml
  )

# OpenMP needs to be included as a library on some platforms.
IF (${OPENMP_FOUND})
  IF (NOT MSVC)
    LIST(APPEND SPII_LIBRARY_DEPENDENCIES gomp)
  ENDIF (NOT MSVC)
ENDIF (${OPENMP_FOUND})

# OpenGL and GLUT
IF (NOT MSVC)
  find_package(GLUT REQUIRED)
  if (GLUT_FOUND)
    MESSAGE("-- Found Glut.")
    include_directories(${GLUT_INCLUDE_DIR})
    link_directories(${GLUT_LIBRARY_DIRS})
    add_definitions(${GLUT_DEFINITIONS})
  endif (GLUT_FOUND)

  find_package(OpenGL REQUIRED)
  if (OPENGL_FOUND)
    MESSAGE("-- Found OpenGL.")
    include_directories(${OPENGL_INCLUDE_DIR})
    link_directories(${OpenGL_LIBRARY_DIRS})
    add_definitions(${OpenGL_DEFINITIONS})
  endif (OPENGL_FOUND)
ENDIF (NOT MSVC)



# Change the default build type from Debug to Release, while still
# supporting overriding the build type.
#
# The CACHE STRING logic here and elsewhere is needed to force CMake
# to pay attention to the value of these variables.
IF (NOT CMAKE_BUILD_TYPE)
  MESSAGE("-- No build type specified; defaulting to CMAKE_BUILD_TYPE=Release.")
  SET(CMAKE_BUILD_TYPE Release CACHE STRING
    "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
    FORCE)
ENDIF (NOT CMAKE_BUILD_TYPE)

#
# gcc settings.
#
IF (CMAKE_COMPILER_IS_GNUCXX)
  MESSAGE("-- Enabling C++11 support for gcc.")
  IF (${BUILD_TESTING})
    ADD_DEFINITIONS("-std=gnu++0x")
  ELSE (${BUILD_TESTING})
    ADD_DEFINITIONS("-std=c++0x")
  ENDIF (${BUILD_TESTING})

  # GCC is not strict enough by default, so enable most of the warnings.
  ADD_DEFINITIONS("-Wall -Wextra -Wno-sign-compare -Wno-unused-parameter")
ENDIF (CMAKE_COMPILER_IS_GNUCXX)

#
# Clang settings
#
IF ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  MESSAGE("-- Enabling C++11 support for clang.")
  IF (${BUILD_TESTING})
    ADD_DEFINITIONS("-std=gnu++11")
  ELSE (${BUILD_TESTING})
    ADD_DEFINITIONS("-std=c++11")
  ENDIF (${BUILD_TESTING})

  # Enable warnings for Clang.
  ADD_DEFINITIONS("-Wall -Wextra -Wno-sign-compare -Wno-unused-parameter")
ENDIF ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")

#
# MSVC settings
#
IF (MSVC)
  # Disable deprecation warning for standard functions.
  ADD_DEFINITIONS("/wd4996")
ENDIF (MSVC)


IF (CMAKE_BUILD_TYPE STREQUAL "Release")
  IF (CMAKE_COMPILER_IS_GNUCXX)
      MESSAGE("-- Optimizing for release mode.")
      # Linux
      IF (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
        MESSAGE("-- Linux")
        ADD_DEFINITIONS("-march=native -mtune=native")
      ENDIF (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
      # Cygwin
      IF (${CMAKE_SYSTEM_NAME} STREQUAL "CYGWIN")
        MESSAGE("-- Cygwin")
        ADD_DEFINITIONS("-march=native -mtune=native")
      ENDIF (${CMAKE_SYSTEM_NAME} STREQUAL "CYGWIN")
      # Mac OS X
      IF (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        MESSAGE("-- MacOS")
        ADD_DEFINITIONS("-fast -msse3")
      ENDIF (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  ENDIF (CMAKE_COMPILER_IS_GNUCXX)
ENDIF (CMAKE_BUILD_TYPE STREQUAL "Release")

ADD_EXECUTABLE(rapidsvg
  rapidsvg.cpp
  line.cpp
  polygon.cpp
  svg_file.cpp)

IF (NOT MSVC)
  target_link_libraries(rapidsvg ${OPENGL_LIBRARIES} ${GLUT_LIBRARY})
ENDIF (NOT MSVC)

ADD_SUBDIRECTORY(svg)
