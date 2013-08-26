include_directories (
	${PROJECT_SOURCE_DIR}/../ext-deps/EASTL/include
	${CMAKE_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}/common
	${CMAKE_SOURCE_DIR}/common/math
	${CMAKE_SOURCE_DIR}/tinker
	${CMAKE_SOURCE_DIR}/tengine
	${CMAKE_SOURCE_DIR}/reflection/src/
)

if (WIN32)
	include_directories(${CMAKE_SOURCE_DIR}/../ext-deps/gl3w/include)
endif (WIN32)

set (RIFTPARTY_SYSTEM_SOURCES
	main.cpp

	riftparty_camera.cpp
	riftparty_renderer.cpp
	riftparty_systems.cpp
	riftparty_window.cpp
)

set (RIFTPARTY_ENTITY_SOURCES
	riftparty_player.cpp
	riftparty_game.cpp
	riftparty_character.cpp
	riftparty_playercharacter.cpp

	robot.cpp
)

set (RIFTPARTY_UI_SOURCES
	ui/hud.cpp
)

set (RIFTPARTY_SOURCES
	${RIFTPARTY_SYSTEM_SOURCES}
	${RIFTPARTY_ENTITY_SOURCES}
	${RIFTPARTY_UI_SOURCES}
)

source_group(System FILES ${RIFTPARTY_SYSTEM_SOURCES})
source_group(Entity FILES ${RIFTPARTY_ENTITY_SOURCES})
source_group(UI FILES ${RIFTPARTY_UI_SOURCES})

add_executable (riftparty WIN32 ${RIFTPARTY_SOURCES})
target_link_libraries (riftparty tinker tengine)
