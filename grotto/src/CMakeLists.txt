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

set (GROTTO_SYSTEM_SOURCES
	main.cpp

	grotto_camera.cpp
	grotto_renderer.cpp
	grotto_systems.cpp
	grotto_window.cpp
)

set (GROTTO_ENTITY_SOURCES
	asymmetric_kinematic.cpp
	kaleidobeast.cpp
	mirror.cpp
	momento.cpp
	receptacle.cpp
	reflectionproxy.cpp
	token.cpp

	grotto_player.cpp
	grotto_game.cpp
	grotto_character.cpp
	grotto_playercharacter.cpp
)

set (GROTTO_UI_SOURCES
	ui/hud.cpp
	ui/levelselector.cpp
)

set (GROTTO_SOURCES
	${GROTTO_SYSTEM_SOURCES}
	${GROTTO_ENTITY_SOURCES}
	${GROTTO_UI_SOURCES}
)

source_group(System FILES ${GROTTO_SYSTEM_SOURCES})
source_group(Entity FILES ${GROTTO_ENTITY_SOURCES})
source_group(UI FILES ${GROTTO_UI_SOURCES})

add_executable (grotto WIN32 ${GROTTO_SOURCES})
target_link_libraries (grotto tinker tengine)
