file(GLOB_RECURSE TESTNETWORK_HEADER_FILES
		"*.h"
		"*.hpp"
		)
file(GLOB_RECURSE TESTNETWORK_SOURCE_FILES
		"*.c"
		"*.cpp"
		"*.rc"
		)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "Header Files" FILES ${TESTNETWORK_HEADER_FILES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "Source Files" FILES ${TESTNETWORK_SOURCE_FILES})

add_executable(TestNetwork ${TESTNETWORK_HEADER_FILES} ${TESTNETWORK_SOURCE_FILES})

target_compile_features(TestNetwork PUBLIC cxx_std_17)
target_include_directories(TestNetwork PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_link_libraries(TestNetwork PRIVATE Acid::Acid)

set_target_properties(TestNetwork PROPERTIES
		FOLDER "Acid"
		)
if(UNIX AND APPLE)
	set_target_properties(TestNetwork PROPERTIES
			MACOSX_BUNDLE_BUNDLE_NAME "Test Network"
			MACOSX_BUNDLE_SHORT_VERSION_STRING ${ACID_VERSION}
			MACOSX_BUNDLE_LONG_VERSION_STRING ${ACID_VERSION}
			MACOSX_BUNDLE_INFO_PLIST "${PROJECT_SOURCE_DIR}/CMake/MacOSXBundleInfo.plist.in"
			)
endif()

add_test(NAME "Network" COMMAND "TestNetwork")

if(ACID_INSTALL_EXAMPLES)
	install(TARGETS TestNetwork
			RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
			ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
			)
endif()
