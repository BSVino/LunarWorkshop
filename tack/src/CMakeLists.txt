include_directories (
	${PROJECT_SOURCE_DIR}/ext-deps/EASTL/include
	${CMAKE_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}/common
	${CMAKE_SOURCE_DIR}/common/math
	${CMAKE_SOURCE_DIR}/tengine
	${CMAKE_SOURCE_DIR}/tack/src/
)

if (WIN32)
	include_directories(${CMAKE_SOURCE_DIR}/ext-deps/glew/include)
	include_directories(${CMAKE_SOURCE_DIR}/ext-deps/devil-1.7.8/include)
endif (WIN32)

set (TACK_SOURCES
	main.cpp

	world.cpp

	tack_character.cpp
	tack_player.cpp
	tack_playercharacter.cpp

	tack_camera.cpp
	tack_game.cpp
	tack_renderer.cpp
	tack_systems.cpp
	tack_window.cpp

	ui/hud.cpp
)
add_executable (tack ${TACK_SOURCES})
target_link_libraries (tack tinker tengine)
