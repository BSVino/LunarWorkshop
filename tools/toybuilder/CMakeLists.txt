include_directories (
	${PROJECT_SOURCE_DIR}/ext-deps/EASTL/include
	${CMAKE_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}/common
	${CMAKE_SOURCE_DIR}/common/math
	${CMAKE_SOURCE_DIR}/tengine
	${CMAKE_SOURCE_DIR}/models
)

set (TOYBUILDER_SOURCES
	main.cpp
	toy_physics.cpp
)

add_executable (toybuilder ${TOYBUILDER_SOURCES})
target_link_libraries (toybuilder lwbase modelconverter toys)
