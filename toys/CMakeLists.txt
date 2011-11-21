include_directories (
	${PROJECT_SOURCE_DIR}/ext-deps/EASTL/include
	../common
	../common/math
)

add_library (toys
	toy.cpp
	toy_util.cpp
)
