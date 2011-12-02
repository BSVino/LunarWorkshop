include_directories (
	${PROJECT_SOURCE_DIR}/ext-deps/EASTL/include
	${CMAKE_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}/common
	${CMAKE_SOURCE_DIR}/common/math
	${CMAKE_SOURCE_DIR}/tengine
	${CMAKE_SOURCE_DIR}/models
)

set (LICENSE_SOURCES
	main.cpp
	license.cpp
)

add_executable (license ${LICENSE_SOURCES})
target_link_libraries (license lwbase)
