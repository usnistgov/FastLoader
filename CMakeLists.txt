cmake_minimum_required(VERSION 3.10)
project(3dfastloader VERSION 2.1.4 LANGUAGES CXX)

include(ExternalProject)

# c++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler
### If necessary, set as an environment variable
### https://gitlab.kitware.com/cmake/community/-/wikis/FAQ#method-1-use-environment-variables

# Configuring CMAKE for verbose output
set(CMAKE_VERBOSE_MAKEFILE ON)

# CMAKE options
option(BUILD_DOXYGEN "Creates the doxygen documentation of the API" OFF)
option(RUN_GTEST "Downloads google unit test API and runs google test scripts to test FastLoader core and api" OFF)
option(BUILD_MAIN "Compiles main function for testing changes to API" ON)

set(RUN_GTEST ON)

# Detect compiler
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	# using regular Clang or AppleClang
	set(CMAKE_COMPILER_IS_CLANGCXX 1)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	set(CMAKE_COMPILER_IS_GNUCXX 1)
	# using GCC
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
	# using Intel C++
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	# using Visual Studio C++
endif()
## lists of other compiler detected
## https://cmake.org/cmake/help/v3.0/variable/CMAKE_LANG_COMPILER_ID.html

# Create version.h file in api folder
string(TIMESTAMP DATE "%m/%d/%Y")
file(WRITE  fast_loader/version.h "#include <string> \n\n")
file(APPEND fast_loader/version.h "/// @file version.h\n")
file(APPEND fast_loader/version.h "/// @author Alexandre Bardakoff\n")
file(APPEND fast_loader/version.h "/// @author Antoine Gerardin\n")
file(APPEND fast_loader/version.h "/// @date ${DATE}\n")
file(APPEND fast_loader/version.h "/// @brief FastLoader version\n\n")
file(APPEND fast_loader/version.h "namespace fl{\n")
file(APPEND fast_loader/version.h "/// \\namespace fl FastLoader namespace\n\n")
file(APPEND fast_loader/version.h "// FastLoader version number\n")
file(APPEND fast_loader/version.h "\tconst int MAJOR_VERSION = ${PROJECT_VERSION_MAJOR}; ///< FastLoader major version\n")
file(APPEND fast_loader/version.h "\tconst int MINOR_VERSION = ${PROJECT_VERSION_MINOR}; ///< FastLoader minor version\n")
file(APPEND fast_loader/version.h "\tconst int PATCH_VERSION = ${PROJECT_VERSION_PATCH}; ///< FastLoader patch version\n")
file(APPEND fast_loader/version.h "\tconst std::string FULL_VERSION = \"${PROJECT_VERSION}\"; ///< FastLoader full version\n")
file(APPEND fast_loader/version.h "}\n")

# Modules defines how to find some dependencies
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules")

find_package(Hedgehog 1.1.0 REQUIRED)
find_package(CUDAToolkit QUIET)

# Since we build a library, we are copying all headers to the target include directory
# DESTINATION is append to CMAKE_INSTALL_PREFIX (default to /usr/local/)
install(DIRECTORY fast_loader
		DESTINATION include
		COMPONENT headers)

#Targets

## Running Tests
if (RUN_GTEST)
	# Add Compile flags
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

	if (NOT MSVC)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra -Werror -pedantic")
	endif (NOT MSVC)

	# Download and unpack googletest at configure time
	configure_file(tests/CMakeLists.txt.gtest googletest-download/CMakeLists.txt)
	execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
			WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download)
	execute_process(COMMAND ${CMAKE_COMMAND} --build .
			WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download)

	# Prevent GoogleTest from overriding our compiler/linker options
	# when building with Visual Studio
	set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

	# Add googletest directly to our build. This adds
	# the following targets: gtest, gtest_main, gmock
	# and gmock_main
	add_subdirectory(${CMAKE_BINARY_DIR}/googletest-src
			${CMAKE_BINARY_DIR}/googletest-build)

	# The gtest/gmock targets carry header search path
	# dependencies automatically when using CMake 2.8.11 or
	# later. Otherwise we have to add them here ourselves.
	if (CMAKE_VERSION VERSION_LESS 2.8.11)
		include_directories("${gtest_SOURCE_DIR}/include")
	endif ()
	#	enable_testing()
	add_subdirectory(tests)
	message(STATUS "GTEST downloaded and imported")
endif (RUN_GTEST)

# Building documentation
if (BUILD_DOXYGEN)
	find_package(Doxygen REQUIRED)
	message(STATUS "Doxygen found")
	if (DOXYGEN_FOUND)
		configure_file(${CMAKE_CURRENT_SOURCE_DIR}/doxygen/fastloader.cfg ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
		add_custom_target(doc ALL
				${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
				WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
				COMMENT "Generating API documentation with Doxygen" VERBATIM
				)

		install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html DESTINATION share/doc/fast_loader)
	endif (DOXYGEN_FOUND)
endif (BUILD_DOXYGEN)

# Building sample programs


if (BUILD_MAIN)
	message(STATUS "Build main")
	set(CMAKE_CXX_FLAGS "${Hedgehog_CXX_FLAGS}")
	# Add Compile flags
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
#	if (NOT MSVC)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra -Werror -pedantic -fsanitize=address -fno-omit-frame-pointer")
#	endif (NOT MSVC)

	set(FL_SRC
			fast_loader/api/default_view.h
			fast_loader/api/unified_view.h
			fast_loader/api/index_request.h
			fast_loader/api/fast_loader_graph.h
			fast_loader/api/abstract_tile_loader.h
			fast_loader/api/abstract_border_creator.h
			fast_loader/api/fast_loader_configuration.h
			fast_loader/api/abstract_adaptive_tile_loader.h

			fast_loader/internal/cache.h
			fast_loader/internal/traits.h
			fast_loader/internal/data_type.h

			fast_loader/internal/data/cached_tile.h
			fast_loader/internal/data/tile_request.h

			fast_loader/internal/data/volume/volume.h
			fast_loader/internal/data/volume/copy_volume.h
			fast_loader/internal/data/volume/copy_volume.h

			fast_loader/internal/data/view/abstract_view.h

			fast_loader/internal/data/view_data/default_view_data.h
			fast_loader/internal/data/view_data/unified_view_data.h
			fast_loader/internal/data/view_data/abstract_view_data.h

			fast_loader/internal/border_creator/wrap_border_creator.h
			fast_loader/internal/border_creator/reflect_border_creator.h
			fast_loader/internal/border_creator/constant_border_creator.h
			fast_loader/internal/border_creator/replicate_border_creator.h
			fast_loader/internal/border_creator/reflect_101_border_creator.h

			fast_loader/internal/graph/tasks/view_loader.h
			fast_loader/internal/graph/tasks/view_waiter.h
			fast_loader/internal/graph/tasks/view_counter.h

			fast_loader/internal/graph/fast_loader_execution_pipeline.h
			fast_loader/internal/graph/memory_manager/fast_loader_memory_manager.h

			fast_loader/internal/traversal/spiral_traversal.h
			fast_loader/internal/traversal/naive_traversal.h
			fast_loader/internal/traversal/hilbert_traversal.h
			fast_loader/internal/traversal/diagonal_traversal.h
			fast_loader/internal/traversal/block_recursive_traversal.h
			fast_loader/api/adaptive_fast_loader_graph.h fast_loader/internal/graph/tasks/mapper_logical_physical.h fast_loader/internal/data/adaptive_tile_request.h fast_loader/internal/data/view/adaptive_view.h fast_loader/internal/data/view_data/adaptive_view_data.h fast_loader/internal/graph/states/direct_to_copy_state.h fast_loader/internal/graph/tasks/copy_logical_tile_to_view.h fast_loader/internal/graph/states/to_tile_loader_state.h fast_loader/internal/graph/states/tile_loader_counter_state.h)

	if(TIFF_FOUND)
		set(FL_SRC
				${FL_SRC}
				fast_loader/specialised_tile_loader/grayscale_tiff_tile_loader.h
				fast_loader/specialised_tile_loader/grayscale_tiff_strip_loader.h)

	endif(TIFF_FOUND)

	# Detect compiler
	if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
		# using regular Clang or AppleClang
		set(CMAKE_COMPILER_IS_CLANGCXX 1)
	elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		set(CMAKE_COMPILER_IS_GNUCXX 1)
		# using GCC
	elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
		# using Intel C++
	elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
		# using Visual Studio C++
	endif()


	if (NOT CMAKE_COMPILER_IS_CLANGCXX)
		link_libraries(stdc++fs)
	endif(NOT CMAKE_COMPILER_IS_CLANGCXX)

	# sample programs require tiff
	find_package(TIFF REQUIRED)
	include_directories(${TIFF_INCLUDE_DIRS})
	link_libraries(${TIFF_LIBRARIES})

	add_executable(main main.cc ${FL_SRC})
	target_link_libraries(main ${Hedgehog_LIBRARIES})
	target_include_directories(main PUBLIC ${Hedgehog_INCLUDE_DIR})

endif (BUILD_MAIN)

