#ifndef TINKER_RENDER_COMMON_H
#define TINKER_RENDER_COMMON_H

typedef enum
{
	BLEND_NONE = 0,
	BLEND_ALPHA,		// Source image scaled by its alpha channel
	BLEND_ADDITIVE,		// Source image values added to destination, scaled by its alpha channel
	BLEND_BOTH,			// Source and destination images added, no scaling
} blendtype_t;

typedef enum
{
	DF_NEVER,
	DF_LESS,
	DF_EQUAL,
	DF_LEQUAL,
	DF_GREATER,
	DF_NOTEQUAL,
	DF_GEQUAL,
	DF_ALWAYS,
} depth_function_t;

#endif
