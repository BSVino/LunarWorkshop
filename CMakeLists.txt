include_directories (
	${PROJECT_SOURCE_DIR}/ext-deps/EASTL/include
	.
	../
	../common
	../common/math
)

set (SMAK_SOURCES
	main.cpp
	crunch/ao.cpp
	crunch/ao_raytrace.cpp
	crunch/crunch.cpp
	crunch/normal.cpp
	shaders/shaders.cpp
	ui/modelwindow.cpp
	ui/modelwindow_input.cpp
	ui/modelwindow_ui.cpp
	ui/picker.cpp
	ui/scenetree.cpp

	../ext-deps/EASTL/src/allocator.cpp
	../ext-deps/EASTL/src/assert.cpp
	../ext-deps/EASTL/src/fixed_pool.cpp
	../ext-deps/EASTL/src/hashtable.cpp
	../ext-deps/EASTL/src/red_black_tree.cpp
	../ext-deps/EASTL/src/string.cpp

	../glgui/glgui.cpp

	../license/license.cpp

	../common/configfile.cpp
	../common/eastl.cpp
	../common/mempool.cpp
	../common/mtrand.cpp
	../common/parallelize.cpp
	../common/platform_linux.cpp
	../common/sockets/sockets.cpp
)

add_executable(smak ${SMAK_SOURCES})
target_link_libraries (smak tinker modelconverter raytracer glfw GLEW IL ILU freetype ftgl)
