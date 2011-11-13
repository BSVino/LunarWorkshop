#ifndef REFLECTION_H
#define REFLECTION_H

typedef enum
{
	REFLECTION_ANY = ~0,
	REFLECTION_NONE = 0,
	REFLECTION_LATERAL,		// Left/right is reflected, due to a vertically placed mirror
	REFLECTION_VERTICAL,	// Up/down is reflected, due to a horizontally placed mirror
} reflection_t;

#endif
