include(FetchContent)

FetchContent_Declare(
	Catch2
	GIT_REPOSITORY https://github.com/catchorg/Catch2.git
	GIT_TAG        v3.2.1
)
FetchContent_MakeAvailable(
	Catch2
)



if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	FetchContent_Declare(
		CMakeModules 
		GIT_REPOSITORY https://github.com/bilke/cmake-modules
		GIT_TAG        1b68d17d3e3321c08092f994a40e0b6a13dff817
	)
	FetchContent_MakeAvailable(
		CMakeModules
	)

	list(APPEND CMAKE_MODULE_PATH "${cmakemodules_SOURCE_DIR}")
	include(CodeCoverage)

	string(REPLACE " " ";" COVERAGE_COMPILER_FLAG_LIST ${COVERAGE_COMPILER_FLAGS})

	target_compile_options(
		CRay-base INTERFACE
			${COVERAGE_COMPILER_FLAG_LIST}
	)
	target_link_libraries(
		CRay-base INTERFACE
			gcov
	)

	setup_target_for_coverage_lcov(
		NAME       coverage
		EXECUTABLE ctest --test-dir "${PROJECT_BINARY_DIR}/tests"
		EXCLUDE
			"build/*"
			"tests/*"
			"${CMAKE_INSTALL_PREFIX}/*"
			"${VCPKG_INSTALLED_DIR}/*"
	)
endif()



enable_testing()
include(Catch)



add_custom_target(all-test)
macro (CRay_SIMPLE_TEST NAME)
	add_executable(
		${NAME}-test
			include/testing.hpp

			src/${NAME}.cpp
	)
	target_include_directories(
		${NAME}-test PRIVATE
			include
	)
	target_link_libraries(
		${NAME}-test PRIVATE
			Catch2::Catch2WithMain
			CRay-base
			CRay::CRay
			CRay::yaml
	)
	catch_discover_tests(${NAME}-test)
	
	add_dependencies(
		all-test
			${NAME}-test
	)
endmacro (CRay_SIMPLE_TEST)

CRay_SIMPLE_TEST(interval)
CRay_SIMPLE_TEST(node)
CRay_SIMPLE_TEST(ordered-map)
CRay_SIMPLE_TEST(ordered-set)
CRay_SIMPLE_TEST(prop)
CRay_SIMPLE_TEST(report-json-schema)
CRay_SIMPLE_TEST(report-yaml)
CRay_SIMPLE_TEST(source)
CRay_SIMPLE_TEST(types)

CRay_SIMPLE_TEST(example-report)
