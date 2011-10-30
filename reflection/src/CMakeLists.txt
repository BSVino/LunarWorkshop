include_directories (
	${PROJECT_SOURCE_DIR}/ext-deps/EASTL/include
	${CMAKE_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}/common
	${CMAKE_SOURCE_DIR}/common/math
	${CMAKE_SOURCE_DIR}/tengine
	${CMAKE_SOURCE_DIR}/reflection/src/
)

if (WIN32)
	include_directories(${CMAKE_SOURCE_DIR}/ext-deps/glew/include)
	include_directories(${CMAKE_SOURCE_DIR}/ext-deps/devil-1.7.8/include)
endif (WIN32)

set (REFLECTION_SOURCES
	main.cpp

	world.cpp
	mirror.cpp

	reflection_player.cpp

	reflection_camera.cpp
	reflection_game.cpp
	reflection_renderer.cpp
	reflection_systems.cpp
	reflection_window.cpp
	reflection_character.cpp
	reflection_playercharacter.cpp

	ui/hud.cpp
)

source_group(Characters FILES ${REFLECTION_CHARACTERS_SOURCES})

add_executable (reflection ${REFLECTION_SOURCES})
target_link_libraries (reflection tinker tengine)
