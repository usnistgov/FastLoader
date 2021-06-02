# NIST-developed software is provided by NIST as a public service.
# You may use, copy and distribute copies of the  software in any  medium,
# provided that you keep intact this entire notice. You may improve,
# modify and create derivative works of the software or any portion of the
# software, and you may copy and distribute such modifications or works.
# Modified works should carry a notice stating that you changed the software
# and should note the date and nature of any such change. Please explicitly
# acknowledge the National Institute of Standards and Technology as the
# source of the software.
# NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY
# OF ANY KIND, EXPRESS, IMPLIED, IN FACT  OR ARISING BY OPERATION OF LAW,
# INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST
# NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION  OF THE SOFTWARE WILL
# BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST
# DOES NOT WARRANT  OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE
# SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE
# CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
# You are solely responsible for determining the appropriateness of using
# and distributing the software and you assume  all risks associated with
# its use, including but not limited to the risks and costs of program
# errors, compliance  with applicable laws, damage to or loss of data,
# programs or equipment, and the unavailability or interruption of operation.
# This software is not intended to be used in any situation where a failure
# could cause risk of injury or damage to property. The software developed
# by NIST employees is not subject to copyright protection within
# the United States.
message(STATUS "Build tests")


include_directories(${CMAKE_SOURCE_DIR})
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
link_libraries(${CMAKE_THREAD_LIBS_INIT})

# some tests require tiff
find_package(TIFF REQUIRED)
include_directories(${TIFF_INCLUDE_DIRS})
link_libraries(${TIFF_LIBRARIES})

set(CMAKE_CXX_FLAGS ${Hedgehog_CXX_FLAGS})

# Add Compile flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
if (NOT MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra -Werror -pedantic")
endif (NOT MSVC)

set(CMAKE_EXE_LINKER_FLAGS "-v")

set(TEST_SRC
        testAll.cc
		test_cache.h
		test_filling.h
		test_requests.h
		test_adaptive_tile_loader.h
		test_channels.h
		test_radius.h
		tile_loaders/virtual_file_tile_loader.h
		tile_loaders/virtual_file_adaptive_tile_loader.h
		tile_loaders/virtual_file_tile_channel_loader.h
		tile_loaders/virtual_file_adaptive_tile_channel_loader.h)

add_executable(runAllTests ${TEST_SRC})


target_include_directories(runAllTests PUBLIC ${GTEST_INCLUDE_DIRS} ${Hedgehog_INCLUDE_DIR})
target_link_directories(runAllTests PUBLIC ${GTEST_LIBRARY_PATH})
target_link_libraries(runAllTests gtest ${Hedgehog_LIBRARIES})