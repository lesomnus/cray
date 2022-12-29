include(FetchContent)

FetchContent_Declare(
	Catch2
	GIT_REPOSITORY https://github.com/catchorg/Catch2.git
	GIT_TAG        v3.2.1
)
FetchContent_MakeAvailable(
	Catch2
)



enable_testing()
include(Catch)



macro (CRay_SIMPLE_TEST NAME)
	add_executable(
		${NAME}-test
			src/${NAME}.cpp
	)
	target_include_directories(
		${NAME}-test PRIVATE
			include
	)
	target_link_libraries(
		${NAME}-test PRIVATE
			Catch2::Catch2WithMain
			CRay::CRay
	)
	catch_discover_tests(${NAME}-test)
endmacro (CRay_SIMPLE_TEST)

CRay_SIMPLE_TEST(interval)
CRay_SIMPLE_TEST(node)
CRay_SIMPLE_TEST(ordered-map)
CRay_SIMPLE_TEST(ordered-set)
CRay_SIMPLE_TEST(prop)
CRay_SIMPLE_TEST(report-yaml)
CRay_SIMPLE_TEST(source)
CRay_SIMPLE_TEST(types)

target_link_libraries(
	source-test PRIVATE
		CRay::yaml
)
