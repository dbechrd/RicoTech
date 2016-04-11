#include "geom.h"
#include <stdlib.h>
#include <math.h>

Vec4 *Vec4_create(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	Vec4 *vec = malloc(sizeof(Vec4));
	vec->x = x;
	vec->y = y;
	vec->z = z;
	vec->w = w;

	return vec;
}

RegularPoly *RegularPoly_create(const Vec4 *p, const unsigned int count)
{
	RegularPoly *poly = malloc(sizeof(RegularPoly));
	poly->vertices = malloc(sizeof(Vec4) * count);
	poly->count = count;

	double delta_angle = PI2 / (double)count;
	double angle = 0;

	for (unsigned int i = 0; i < count; i++)
	{
		poly->vertices[i].x = (GLfloat)cos(angle);
		poly->vertices[i].y = (GLfloat)sin(angle);
		poly->vertices[i].z = p->z;
		poly->vertices[i].w = p->w;
		angle += delta_angle;
	}

	return poly;
}

void RegularPoly_destroy(RegularPoly *poly)
{
	free(poly->vertices);
	poly->vertices = NULL;

	free(poly);
	poly = NULL;
}