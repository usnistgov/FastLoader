# NIST-developed software is provided by NIST as a public service.
# You may use, copy and distribute copies of the  software in any  medium,
# provided that you keep intact this entire notice. You may improve,
# modify and create derivative works of the software or any portion of the
# software, and you may copy and distribute such modifications or works.
# Modified works should carry a notice stating that you changed the software
# and should note the date and nature of any such change. Please explicitly
# acknowledge the National Institute of Standards and Technology as the
# source of the software.

# - Find HEdgehog includes and required compiler flags and library dependencies
# Dependencies: C++20
#
# This module defines
#  FastLoader_INCLUDE_DIR
#  FastLoader_FOUND
#

SET(FastLoader_FOUND ON)

include(CheckCXXCompilerFlag)
if (CMAKE_CXX_STANDARD LESS 20)
    message(FATAL_ERROR
            "Hedgehog requires at least C++20 and the current version of C++ is set to '${CMAKE_CXX_STANDARD}'.\n"
            "Consider setting CMAKE_CXX_STANDARD to 20 or higher.")
endif()

#Check FastLoader dependencies
if (FastLoader_FIND_QUIETLY)
    find_package(Hedgehog QUIET)
    if (NOT Hedgehog_FOUND)
        message(STATUS "Hedgehog library has not been found, please install it to use FastLoader.")
        SET(FastLoader_FOUND OFF)
    endif (NOT Hedgehog_FOUND)
else ()
    find_package(Hedgehog REQUIRED)
    if (NOT Hedgehog_FOUND)
        message(FATAL_ERROR "Hedgehog library has not been found, please install it to use FastLoader.")
        SET(FastLoader_FOUND OFF)
    endif (NOT Hedgehog_FOUND)
endif (FastLoader_FIND_QUIETLY)

#    Check include files
FIND_PATH(FastLoader_INCLUDE_DIR fast_loader.h
        /usr/include
        /usr/local/include
        )

IF (NOT FastLoader_INCLUDE_DIR)
    SET(FastLoader_FOUND OFF)
    MESSAGE(STATUS "Could not find FastLoader includes. FastLoader_FOUND now off")
ENDIF ()

IF (FastLoader_FOUND)
    include_directories(${FastLoader_INCLUDE_DIR})
    message(STATUS "Found FastLoader include: ${FastLoader_INCLUDE_DIR}")
ELSE (FastLoader_FOUND)
    message(FATAL_ERROR "Could not find FastLoader header files")
ENDIF (FastLoader_FOUND)


MARK_AS_ADVANCED(FastLoader_INCLUDE_DIR)