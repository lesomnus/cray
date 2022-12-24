cmake_minimum_required(VERSION 3.21)

project(
	"CRay"
		VERSION   0.0.1
		LANGUAGES CXX
)

option(${PROJECT_NAME}_TESTS "Enable ${PROJECT_NAME} tests targets." ${PROJECT_IS_TOP_LEVEL})

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_STANDARD          20 )
set(CMAKE_CXX_STANDARD_REQUIRED ON )



add_library(
	CRay SHARED
		include/cray/detail/props/int.hpp
		include/cray/detail/props/list.hpp
		include/cray/detail/props/map.hpp
		include/cray/detail/props/mono-list.hpp
		include/cray/detail/props/mono-map.hpp
		include/cray/detail/props/scalar.hpp
		include/cray/detail/props/str.hpp
		include/cray/detail/props/structured.hpp
		include/cray/detail/interval.hpp
		include/cray/detail/ordered_map.hpp
		include/cray/detail/prop.hpp
		include/cray/detail/props.hpp
		include/cray/detail/tester.hpp
		include/cray/node.hpp
		include/cray/report.hpp
		include/cray/source.hpp
		include/cray/types.hpp

		src/report/yaml.cpp
		src/source/entry.cpp
		src/source/null.cpp
		src/source.cpp
)
target_include_directories(
	CRay
		PUBLIC
			$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
			$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>
)
# target_link_libraries(
# 	CRay
# 		PRIVATE
# 			yaml-cpp
# )

add_library(
	CRay::CRay
		ALIAS CRay
)



if(${PROJECT_NAME}_TESTS)
	add_subdirectory(tests)
endif()
