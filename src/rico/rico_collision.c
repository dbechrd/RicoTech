static bool collide_ray_plane(struct vec3 *_contact, const struct ray *ray,
                              const struct plane *plane)
{
    float t = 0.0f;

    // w = plane.p - ray.p
    // t = (plane.n * w) / (plane.n * ray.d)

    float denom = v3_dot(&plane->n, &ray->d);
    if (fabs(denom) > EPSILON)
    {
        struct vec3 w = plane->p;
        v3_sub(&w, &ray->origin);

        t = v3_dot(&plane->n, &w) / denom;
        if (t >= 0)
        {
            if (_contact)
            {
                *_contact = ray->d;
                v3_scalef(_contact, t);
                v3_add(_contact, &ray->origin);
            }
            return true;
        }
    }

    return false;
}
// Based on Christer Ericson's book:
//   "Real-Time Collision Detection" (p. 178)
//   http://realtimecollisiondetection.net/books/rtcd/
static bool collide_ray_sphere(float *_t, const struct ray *ray,
                               const struct sphere *sphere)
{
    struct vec3 m = ray->origin;
    v3_sub(&m, &sphere->center);

    // Note: dot(m, m) == len(m) * len(m)
    float b = v3_dot(&m, &ray->d);
    float c = v3_dot(&m, &m) - (sphere->r * sphere->r);
    if (c > 0.0f && b > 0.0f)
    {
        return false;
    }

    // Discriminant
    float disc = (b * b) - c;
    if (disc < 0.0f)
    {
        return false;
    }

    float t = -b - sqrtf(disc);
    if (t > 0.0f)
    {
        if (_t)
        {
            *_t = t;
        }
        return true;
    }

    return false;
}
static bool collide_ray_aabb(float *_t, const struct ray *ray,
                             const struct ric_aabb *aabb)
{
    struct vec3 p[2] = {
        VEC3(
            aabb->c.x - aabb->e.x,
            aabb->c.y - aabb->e.y,
            aabb->c.z - aabb->e.z
        ),
        VEC3(
            aabb->c.x + aabb->e.x,
            aabb->c.y + aabb->e.y,
            aabb->c.z + aabb->e.z
        ),
    };

    struct vec3 invdir;
    invdir.x = 1.0f / ray->d.x;
    invdir.y = 1.0f / ray->d.y;
    invdir.z = 1.0f / ray->d.z;

    int sign[3];
    sign[0] = invdir.x < 0;
    sign[1] = invdir.y < 0;
    sign[2] = invdir.z < 0;

    float t_min, t_max, y_min, y_max, z_min, z_max;

    // x-axis
    t_min = (p[    sign[0]].x - ray->origin.x) * invdir.x;
    t_max = (p[1 - sign[0]].x - ray->origin.x) * invdir.x;

    // y-axis
    y_min = (p[    sign[1]].y - ray->origin.y) * invdir.y;
    y_max = (p[1 - sign[1]].y - ray->origin.y) * invdir.y;

    if (t_min > y_max || y_min > t_max)
        return false;
    if (y_min > t_min)
        t_min = y_min;
    if (y_max < t_max)
        t_max = y_max;

    // z-axis
    z_min = (p[    sign[2]].z - ray->origin.z) * invdir.z;
    z_max = (p[1 - sign[2]].z - ray->origin.z) * invdir.z;

    if ((t_min > z_max) || (z_min > t_max))
        return false;
    if (z_min > t_min)
        t_min = z_min;
    if (z_max < t_max)
        t_max = z_max;

    // Object behind ray
    if (t_min < 0)
        return false;

    *_t = t_min;
    return true;
}
static bool collide_ray_obb(float *_dist, const struct ray *r,
                            const struct ric_aabb *aabb,
                            const struct mat4 *model_matrix)
{
    struct vec3 p0 = aabb->c;
    v3_sub(&p0, &aabb->e);
    struct vec3 p1 = aabb->c;
    v3_add(&p1, &aabb->e);

	// Intersection method from Real-Time Rendering and Essential Mathematics
    // for Games

	float t_min = 0.0f;
	float t_max = 100000.0f;

    struct ray ray = *r;

    //DEBUG: Draw BBox without transform
    // bbox_render_color(bbox, &MAT4_IDENT, COLOR_GRAY_HIGHLIGHT);

    struct vec3 model_orig;
    model_orig.x = model_matrix->m[0][3];
    model_orig.y = model_matrix->m[1][3];
    model_orig.z = model_matrix->m[2][3];

    ray.origin = model_orig;
    v3_sub(&ray.origin, &r->origin);

    //DEBUG: Draw sphere / ray
    // struct sphere sphere_ray;
    // sphere_ray.orig = ray.orig;
    // sphere_ray.radius = 0.05f;
    // prim_draw_sphere(&sphere_ray, &COLOR_MAGENTA_HIGHLIGHT);

    // struct ray debug_ray = ray;
    // v3_scalef(&debug_ray.d, 10.0f);
    // prim_draw_ray(&debug_ray, &MAT4_IDENT, COLOR_YELLOW_HIGHLIGHT);

	// Test intersection with the 2 planes perpendicular to the OBB's X axis
	{
        struct vec3 x_axis;
        x_axis.x = model_matrix->m[0][0];
        x_axis.y = model_matrix->m[1][0];
        x_axis.z = model_matrix->m[2][0];

        float x_scale = v3_length(&x_axis);
        v3_normalize(&x_axis);
        v3_scalef(&x_axis, 1.0f / x_scale);

        // struct ray x_ray;
        // x_ray.orig = model_orig;
        // x_ray.d = x_axis;
        // prim_draw_ray(&x_ray, &MAT4_IDENT, COLOR_RED_HIGHLIGHT);

        // struct sphere x_sphere;
        // x_sphere.orig = x_ray.orig;
        // v3_add(&x_sphere.orig, &x_ray.d);
        // x_sphere.radius = 0.05f;
        // prim_draw_sphere(&x_sphere, &COLOR_RED_HIGHLIGHT);

		float e = v3_dot(&x_axis, &ray.origin);
		float f = v3_dot(&ray.d, &x_axis);

		if (fabsf(f) > 0.00001f) // Standard case
        {
            // Intersection with the "left" plane
            float t1 = (e + p0.x) / f;
            // Intersection with the "right" plane
			float t2 = (e + p1.x) / f;
			// t1 and t2 now contain distances betwen ray origin and ray-plane
            // intersections

			// We want t1 to represent the nearest intersection,
			// so if it's not the case, invert t1 and t2
			if (t1>t2){
				float w=t1;t1=t2;t2=w; // swap t1 and t2
			}

			// t_max is the nearest "far" intersection (amongst the X,Y and Z
            // planes pairs)
			if ( t2 < t_max )
				t_max = t2;
			// t_min is the farthest "near" intersection (amongst the X,Y and Z
            // planes pairs)
			if ( t1 > t_min )
				t_min = t1;

			// And here's the trick :
			// If "far" is closer than "near", then there is NO intersection.
			// See the images in the tutorials for the visual explanation.
			if (t_max < t_min )
				return false;

		}
        else
        {
            // Rare case : the ray is almost parallel to the planes, so they
            // don't have any "intersection"
			if(-e + p0.x > 0.0f || -e + p1.x < 0.0f)
				return false;
		}
	}

    // Test y-axis
	{
        struct vec3 y_axis;
        y_axis.x = model_matrix->m[0][1];
        y_axis.y = model_matrix->m[1][1];
        y_axis.z = model_matrix->m[2][1];

        float scale = v3_length(&y_axis);
        v3_normalize(&y_axis);
        v3_scalef(&y_axis, 1.0f / scale);

        // struct ray y_ray;
        // y_ray.orig = model_orig;
        // y_ray.d = y_axis;
        // prim_draw_ray(&y_ray, &MAT4_IDENT, COLOR_GREEN_HIGHLIGHT);

        // struct sphere y_sphere;
        // y_sphere.orig = y_ray.orig;
        // v3_add(&y_sphere.orig, &y_ray.d);
        // y_sphere.radius = 0.05f;
        // prim_draw_sphere(&y_sphere, &COLOR_GREEN_HIGHLIGHT);

        float e = v3_dot(&y_axis, &ray.origin);
		float f = v3_dot(&ray.d, &y_axis);

		if (fabsf(f) > 0.00001f)
        {
            float t1 = (e + p0.y) / f;
			float t2 = (e + p1.y) / f;

			if (t1>t2){float w=t1;t1=t2;t2=w;}

			if ( t2 < t_max )
				t_max = t2;
			if ( t1 > t_min )
				t_min = t1;
			if (t_min > t_max)
				return false;
		}
        else
        {
			if(-e + p0.y > 0.0f || -e + p1.y < 0.0f)
				return false;
		}
	}

    // Test z-axis
	{
        struct vec3 z_axis;
        z_axis.x = model_matrix->m[0][2];
        z_axis.y = model_matrix->m[1][2];
        z_axis.z = model_matrix->m[2][2];

        float scale = v3_length(&z_axis);
        v3_normalize(&z_axis);
        v3_scalef(&z_axis, 1.0f / scale);

        // struct ray z_ray;
        // z_ray.orig = model_orig;
        // z_ray.d = z_axis;
        // prim_draw_ray(&z_ray, &MAT4_IDENT, COLOR_BLUE_HIGHLIGHT);

        // struct sphere z_sphere;
        // z_sphere.orig = z_ray.orig;
        // v3_add(&z_sphere.orig, &z_ray.d);
        // z_sphere.radius = 0.05f;
        // prim_draw_sphere(&z_sphere, &COLOR_BLUE_HIGHLIGHT);

        float e = v3_dot(&z_axis, &ray.origin);
		float f = v3_dot(&ray.d, &z_axis);

		if (fabsf(f) > 0.00001f)
        {
			float t1 = (e + p0.z) / f;
			float t2 = (e + p1.z) / f;

			if (t1>t2){float w=t1;t1=t2;t2=w;}

			if ( t2 < t_max )
				t_max = t2;
			if ( t1 > t_min )
				t_min = t1;
			if (t_min > t_max)
				return false;
		}
        else
        {
			if(-e + p0.z > 0.0f || -e + p1.z < 0.0f)
				return false;
		}
	}

	*_dist = t_min;
	return true;
}
// 3D OBB v. OBB collision test using separating-axis theorem
// Based on Christer Ericson's book:
//   "Real-Time Collision Detection" (p. 103)
//   http://realtimecollisiondetection.net/books/rtcd/
// and David Eberly's paper:
//   "Dynamic Collision Detection using Oriented Bounding Boxes" (p. 7)
//   https://www.geometrictools.com/Documentation/DynamicCollisionDetection.pdf
bool collide_obb_obb(struct RICO_obb *a, struct RICO_obb *b, int *axis)
{
    bool a_has_obb = v3_length(&a->e) > 0.0f;
    bool b_has_obb = v3_length(&b->e) > 0.0f;
#if RICO_DEBUG
    // TODO: Make sure everything has OBB and remove this line
    if (!a_has_obb || !b_has_obb) return false;
#endif
    DLB_ASSERT(a_has_obb);
    DLB_ASSERT(b_has_obb);

    // TODO: Find and return collision manifold

    float ra, rb;
    struct mat4 C = { 0 };
    struct mat4 CAbs = { 0 };

    struct vec3 t = b->c;
    v3_sub(&t, &a->c);
    t = VEC3(v3_dot(&t, &a->u[0]), v3_dot(&t, &a->u[1]), v3_dot(&t, &a->u[2]));

    // Test axes A0, A1, A2
    // (and build B -> A transform matrix for AABB v. OBB optimization)
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            C.m[i][j] = v3_dot(&a->u[i], &b->u[j]);
            CAbs.m[i][j] = fabsf(C.m[i][j]) + MATH_EPSILON;
        }

        ra = a->e.a[i];
        rb = b->e.a[0] * CAbs.m[i][0] +
             b->e.a[1] * CAbs.m[i][1] +
             b->e.a[2] * CAbs.m[i][2];
        if (fabsf(t.a[i]) > ra + rb)
        {
            if (axis) *axis = i + 1;
            return false;
        }
    }

    // Test axes B0, B1, B2
    for (int i = 0; i < 3; ++i)
    {
        ra = a->e.a[0] * CAbs.m[0][i] +
             a->e.a[1] * CAbs.m[1][i] +
             a->e.a[2] * CAbs.m[2][i];
        rb = b->e.a[i];
        if (fabsf(t.a[0] * C.m[0][i] +
                  t.a[1] * C.m[1][i] +
                  t.a[2] * C.m[2][i]) > ra + rb)
        {
            if (axis) *axis = i + 4;
            return false;
        }
    }

    // Test axis L = A0 x B0
    ra = a->e.a[1] * CAbs.m[2][0] + a->e.a[2] * CAbs.m[1][0];
    rb = b->e.a[1] * CAbs.m[0][2] + b->e.a[2] * CAbs.m[0][1];
    if (fabsf(t.a[2] * C.m[1][0] - t.a[1] * C.m[2][0]) > ra + rb)
    {
        if (axis) *axis = 7;
        return false;
    }
    // Test axis L = A0 x B1
    ra = a->e.a[1] * CAbs.m[2][1] + a->e.a[2] * CAbs.m[1][1];
    rb = b->e.a[0] * CAbs.m[0][2] + b->e.a[2] * CAbs.m[0][0];
    if (fabsf(t.a[2] * C.m[1][1] - t.a[1] * C.m[2][1]) > ra + rb)
    {
        if (axis) *axis = 8;
        return false;
    }
    // Test axis L = A0 x B2
    ra = a->e.a[1] * CAbs.m[2][2] + a->e.a[2] * CAbs.m[1][2];
    rb = b->e.a[0] * CAbs.m[0][1] + b->e.a[1] * CAbs.m[0][0];
    if (fabsf(t.a[2] * C.m[1][2] - t.a[1] * C.m[2][2]) > ra + rb)
    {
        if (axis) *axis = 9;
        return false;
    }
    // Test axis L = A1 x B0
    ra = a->e.a[0] * CAbs.m[2][0] + a->e.a[2] * CAbs.m[0][0];
    rb = b->e.a[1] * CAbs.m[1][2] + b->e.a[2] * CAbs.m[1][1];
    if (fabsf(t.a[0] * C.m[2][0] - t.a[2] * C.m[0][0]) > ra + rb)
    {
        if (axis) *axis = 10;
        return false;
    }
    // Test axis L = A1 x B1
    ra = a->e.a[0] * CAbs.m[2][1] + a->e.a[2] * CAbs.m[0][1];
    rb = b->e.a[0] * CAbs.m[1][2] + b->e.a[2] * CAbs.m[1][0];
    if (fabsf(t.a[0] * C.m[2][1] - t.a[2] * C.m[0][1]) > ra + rb)
    {
        if (axis) *axis = 11;
        return false;
    }
    // Test axis L = A1 x B2
    ra = a->e.a[0] * CAbs.m[2][2] + a->e.a[2] * CAbs.m[0][2];
    rb = b->e.a[0] * CAbs.m[1][1] + b->e.a[1] * CAbs.m[1][0];
    if (fabsf(t.a[0] * C.m[2][2] - t.a[2] * C.m[0][2]) > ra + rb)
    {
        if (axis) *axis = 12;
        return false;
    }
    // Test axis L = A2 x B0
    ra = a->e.a[0] * CAbs.m[1][0] + a->e.a[1] * CAbs.m[0][0];
    rb = b->e.a[1] * CAbs.m[2][2] + b->e.a[2] * CAbs.m[2][1];
    if (fabsf(t.a[1] * C.m[0][0] - t.a[0] * C.m[1][0]) > ra + rb)
    {
        if (axis) *axis = 13;
        return false;
    }
    // Test axis L = A2 x B1
    ra = a->e.a[0] * CAbs.m[1][1] + a->e.a[1] * CAbs.m[0][1];
    rb = b->e.a[0] * CAbs.m[2][2] + b->e.a[2] * CAbs.m[2][0];
    if (fabsf(t.a[1] * C.m[0][1] - t.a[0] * C.m[1][1]) > ra + rb)
    {
        if (axis) *axis = 14;
        return false;
    }
    // Test axis L = A2 x B2
    ra = a->e.a[0] * CAbs.m[1][2] + a->e.a[1] * CAbs.m[0][2];
    rb = b->e.a[0] * CAbs.m[2][1] + b->e.a[1] * CAbs.m[2][0];
    if (fabsf(t.a[1] * C.m[0][2] - t.a[0] * C.m[1][2]) > ra + rb)
    {
        if (axis) *axis = 15;
        return false;
    }

    return true;
}

// Based on David Eberly's paper:
// "Dynamic Collision Detection using Oriented Bounding Boxes" (p. 7)
// https://www.geometrictools.com/Documentation/DynamicCollisionDetection.pdf
bool collide_obb_obb_eberly(struct RICO_obb *a, struct RICO_obb *b)
{
    float R, R0, R1;
    struct vec3 D = b->c;
    v3_sub(&D, &a->c);

    struct mat4 C = MAT4_IDENT;
    struct mat4 AbsC = MAT4_IDENT;
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            C.m[i][j] = v3_dot(&a->u[i], &b->u[j]);
            AbsC.m[i][j] = fabsf(C.m[i][j]) + MATH_EPSILON;
        }
    }

#define A0 &a->u[0]
#define A1 &a->u[1]
#define A2 &a->u[2]
#define B0 &b->u[0]
#define B1 &b->u[1]
#define B2 &b->u[2]
#define a0 a->e.x
#define a1 a->e.y
#define a2 a->e.z
#define b0 b->e.x
#define b1 b->e.y
#define b2 b->e.z
#define c00 C.m[0][0]
#define c01 C.m[0][1]
#define c02 C.m[0][2]
#define c10 C.m[1][0]
#define c11 C.m[1][1]
#define c12 C.m[1][2]
#define c20 C.m[2][0]
#define c21 C.m[2][1]
#define c22 C.m[2][2]
#define abs_c00 AbsC.m[0][0]
#define abs_c01 AbsC.m[0][1]
#define abs_c02 AbsC.m[0][2]
#define abs_c10 AbsC.m[1][0]
#define abs_c11 AbsC.m[1][1]
#define abs_c12 AbsC.m[1][2]
#define abs_c20 AbsC.m[2][0]
#define abs_c21 AbsC.m[2][1]
#define abs_c22 AbsC.m[2][2]

    ////////

    R0 = a0;
    R1 = b0*abs_c00 + b1*abs_c01 + b2*abs_c02;
    R = fabsf(v3_dot(A0, &D));
    if (R > R0 + R1)
        return true;

    R0 = a1;
    R1 = b0*abs_c10 + b1*abs_c11 + b2*abs_c12;
    R = fabsf(v3_dot(A1, &D));
    if (R > R0 + R1)
        return true;

    R0 = a2;
    R1 = b0*abs_c20 + b1*abs_c21 + b2*abs_c22;
    R = fabsf(v3_dot(A2, &D));
    if (R > R0 + R1)
        return true;

    ////////

    R0 = b0;
    R1 = a0*abs_c00 + a1*abs_c10 + a2*abs_c20;
    R = fabsf(v3_dot(B0, &D));
    if (R > R0 + R1)
        return true;

    R0 = b1;
    R1 = a0*abs_c01 + a1*abs_c11 + a2*abs_c21;
    R = fabsf(v3_dot(B1, &D));
    if (R > R0 + R1)
        return true;

    R0 = b2;
    R1 = a0*abs_c02 + a1*abs_c12 + a2*abs_c22;
    R = fabsf(v3_dot(B2, &D));
    if (R > R0 + R1)
        return true;

    ////////////////////////////////////////////////////////////////////////////
    struct vec3 t0, t1;
    ////////////////////////////////////////////////////////////////////////////

    R0 = a1*abs_c20 + a2*abs_c10;
    R1 = b1*abs_c02 + b2*abs_c01;
    t0 = *A2;
    t1 = *A1;
    R = fabsf(v3_dot(v3_scalef(&t0, c10), &D) -
              v3_dot(v3_scalef(&t1, c20), &D));
    if (R > R0 + R1)
        return true;

    R0 = a1*abs_c21 + a2*abs_c11;
    R1 = b0*abs_c02 + b2*abs_c00;
    t0 = *A2;
    t1 = *A1;
    R = fabsf(v3_dot(v3_scalef(&t0, c11), &D) -
              v3_dot(v3_scalef(&t1, c21), &D));
    if (R > R0 + R1)
        return true;

    R0 = a1*abs_c22 + a2*abs_c12;
    R1 = b0*abs_c01 + b1*abs_c00;
    t0 = *A2;
    t1 = *A1;
    R = fabsf(v3_dot(v3_scalef(&t0, c12), &D) -
              v3_dot(v3_scalef(&t1, c22), &D));
    if (R > R0 + R1)
        return true;

    ////////////////////////////////////////////////////////////////////////////

    R0 = a0*abs_c20 + a2*abs_c00;
    R1 = b1*abs_c12 + b2*abs_c11;
    t0 = *A0;
    t1 = *A2;
    R = fabsf(v3_dot(v3_scalef(&t0, c20), &D) -
              v3_dot(v3_scalef(&t1, c00), &D));
    if (R > R0 + R1)
        return true;

    R0 = a0*abs_c21 + a2*abs_c01;
    R1 = b0*abs_c12 + b2*abs_c10;
    t0 = *A0;
    t1 = *A2;
    R = fabsf(v3_dot(v3_scalef(&t0, c21), &D) -
              v3_dot(v3_scalef(&t1, c01), &D));
    if (R > R0 + R1)
        return true;

    R0 = a0*abs_c22 + a2*abs_c02;
    R1 = b0*abs_c11 + b1*abs_c10;
    t0 = *A0;
    t1 = *A2;
    R = fabsf(v3_dot(v3_scalef(&t0, c22), &D) -
              v3_dot(v3_scalef(&t1, c02), &D));
    if (R > R0 + R1)
        return true;

    ////////////////////////////////////////////////////////////////////////////

    R0 = a0*abs_c10 + a1*abs_c00;
    R1 = b1*abs_c22 + b2*abs_c21;
    t0 = *A1;
    t1 = *A0;
    R = fabsf(v3_dot(v3_scalef(&t0, c00), &D) -
              v3_dot(v3_scalef(&t1, c10), &D));
    if (R > R0 + R1)
        return true;

    R0 = a0*abs_c11 + a1*abs_c01;
    R1 = b0*abs_c22 + b2*abs_c20;
    t0 = *A1;
    t1 = *A0;
    R = fabsf(v3_dot(v3_scalef(&t0, c01), &D) -
              v3_dot(v3_scalef(&t1, c11), &D));
    if (R > R0 + R1)
        return true;

    R0 = a0*abs_c12 + a1*abs_c02;
    R1 = b0*abs_c21 + b1*abs_c20;
    t0 = *A1;
    t1 = *A0;
    R = fabsf(v3_dot(v3_scalef(&t0, c02), &D) -
              v3_dot(v3_scalef(&t1, c12), &D));
    if (R > R0 + R1)
        return true;

#undef A0
#undef A1
#undef A2
#undef B0
#undef B1
#undef B2
#undef a0
#undef a1
#undef a2
#undef b0
#undef b1
#undef b2
#undef c00
#undef c01
#undef c02
#undef c10
#undef c11
#undef c12
#undef c20
#undef c21
#undef c22
#undef abs_c00
#undef abs_c01
#undef abs_c02
#undef abs_c10
#undef abs_c11
#undef abs_c12
#undef abs_c20
#undef abs_c21
#undef abs_c22

    return false;
}