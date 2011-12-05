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

set (REFLECTION_SYSTEM_SOURCES
	main.cpp

	reflection_camera.cpp
	reflection_renderer.cpp
	reflection_systems.cpp
	reflection_window.cpp
)

set (REFLECTION_ENTITY_SOURCES
	world.cpp
	mirror.cpp
	receptacle.cpp
	token.cpp

	reflection_player.cpp
	reflection_game.cpp
	reflection_character.cpp
	reflection_playercharacter.cpp
)

set (REFLECTION_UI_SOURCES
	ui/hud.cpp
	ui/levelselector.cpp
)

set (REFLECTION_SOURCES
	${REFLECTION_SYSTEM_SOURCES}
	${REFLECTION_ENTITY_SOURCES}
	${REFLECTION_UI_SOURCES}
)

source_group(System FILES ${REFLECTION_SYSTEM_SOURCES})
source_group(Entity FILES ${REFLECTION_ENTITY_SOURCES})
source_group(UI FILES ${REFLECTION_UI_SOURCES})

add_executable (reflection WIN32 ${REFLECTION_SOURCES})
target_link_libraries (reflection tinker tengine)
