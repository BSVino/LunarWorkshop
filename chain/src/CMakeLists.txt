include_directories (
	${PROJECT_SOURCE_DIR}/ext-deps/EASTL/include
	${CMAKE_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}/common
	${CMAKE_SOURCE_DIR}/common/math
	${CMAKE_SOURCE_DIR}/tengine
	${CMAKE_SOURCE_DIR}/chain/src/
)

if (WIN32)
	include_directories(${CMAKE_SOURCE_DIR}/ext-deps/glew/include)
	include_directories(${CMAKE_SOURCE_DIR}/ext-deps/devil-1.7.8/include)
endif (WIN32)

set (CHAIN_SOURCES
	main.cpp

	chain_player.cpp

	chain_camera.cpp
	chain_game.cpp
	chain_renderer.cpp
	chain_systems.cpp
	chain_window.cpp
	chain_playercharacter.cpp

	ui/hud.cpp
)

add_executable (chain ${CHAIN_SOURCES})
target_link_libraries (chain tinker tengine)
