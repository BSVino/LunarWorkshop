include_directories (
	${PROJECT_SOURCE_DIR}/../ext-deps/EASTL/include
	${CMAKE_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}/common
	${CMAKE_SOURCE_DIR}/common/math
	${CMAKE_SOURCE_DIR}/tengine
	${CMAKE_SOURCE_DIR}/models
)

set (GEPPETTO_SOURCES
	geppetto.cpp
	inputscript.cpp
	toy_physics.cpp
)

set (TOYBUILDER_SOURCES
	main.cpp
)

add_library(geppetto STATIC ${GEPPETTO_SOURCES})
add_executable (toybuilder ${TOYBUILDER_SOURCES})
target_link_libraries (toybuilder lwbase tinker modelconverter toys geppetto)
