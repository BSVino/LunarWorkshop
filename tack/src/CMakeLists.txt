include_directories (
	${PROJECT_SOURCE_DIR}/../ext-deps/EASTL/include
	${CMAKE_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}/common
	${CMAKE_SOURCE_DIR}/common/math
	${CMAKE_SOURCE_DIR}/tengine
	${CMAKE_SOURCE_DIR}/tack/src/
)

if (WIN32)
	include_directories(${CMAKE_SOURCE_DIR}/../ext-deps/glew-1.7.0/include)
	include_directories(${CMAKE_SOURCE_DIR}/../ext-deps/devil-1.7.8/include)
endif (WIN32)

set (TACK_CHARACTERS_SOURCES
	characters/corpse.cpp
	characters/tack_character.cpp
	characters/tack_playercharacter.cpp
	characters/zombie.cpp
)

set (TACK_SOURCES
	main.cpp

	world.cpp

	tack_player.cpp

	tack_camera.cpp
	tack_game.cpp
	tack_renderer.cpp
	tack_systems.cpp
	tack_window.cpp

	ui/hud.cpp

	${TACK_CHARACTERS_SOURCES}
)

source_group(Characters FILES ${TACK_CHARACTERS_SOURCES})

add_executable (tack WIN32 ${TACK_SOURCES})
target_link_libraries (tack tinker tengine)
