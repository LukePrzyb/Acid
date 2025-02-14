include(FetchContent)

FetchContent_Declare(
		googletest
		URL https://github.com/google/googletest/archive/release-1.8.1.tar.gz
		URL_MD5 2e6fbeb6a91310a16efe181886c59596
		)
FetchContent_GetProperties(googletest)
if(NOT googletest_POPULATED)
	FetchContent_Populate(googletest)
	add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})
	set_target_properties(gmock PROPERTIES FOLDER googletest)
	set_target_properties(gmock_main PROPERTIES FOLDER googletest)
	set_target_properties(gtest PROPERTIES FOLDER googletest)
	set_target_properties(gtest_main PROPERTIES FOLDER googletest)
endif()

enable_testing()

file(GLOB_RECURSE TESTUNITS_HEADER_FILES
		"*.h"
		"*.hpp"
		)
file(GLOB_RECURSE TESTUNITS_SOURCE_FILES
		"*.c"
		"*.cpp"
		)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "Header Files" FILES ${TESTUNITS_HEADER_FILES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "Source Files" FILES ${TESTUNITS_SOURCE_FILES})

add_executable(UnitTests ${TESTUNITS_HEADER_FILES} ${TESTUNITS_SOURCE_FILES})

target_compile_features(UnitTests PUBLIC cxx_std_17)
target_include_directories(UnitTests PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_link_libraries(UnitTests PRIVATE Acid::Acid gtest_main)

set_target_properties(UnitTests PROPERTIES
		FOLDER "Acid"
		)

add_test(NAME unit
		COMMAND ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}/vat_test
		)
		