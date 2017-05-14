bool collide_ray_bbox(const struct ray *ray, const struct bbox *bbox,
                      const struct mat4 *transform)
{
    //TODO: Transform ray and bbox
    struct vec3 bbox_p[2];
    bbox_p[0] = bbox->p[0];
    bbox_p[1] = bbox->p[1];
    vec3_mul_mat4(&bbox_p[0], transform);
    vec3_mul_mat4(&bbox_p[1], transform);

    struct vec3 invdir;
    invdir.x = 1.0f / ray->dir.x;
    invdir.y = 1.0f / ray->dir.y;
    invdir.z = 1.0f / ray->dir.z;

    int sign[3];
    sign[0] = invdir.x < 0;
    sign[1] = invdir.y < 0;
    sign[2] = invdir.z < 0;

    float t_min, t_max, y_min, y_max, z_min, z_max;

    t_min = (bbox_p[    sign[0]].x - ray->orig.x) * invdir.x;
    t_max = (bbox_p[1 - sign[0]].x - ray->orig.x) * invdir.x;
    y_min = (bbox_p[    sign[1]].y - ray->orig.y) * invdir.y;
    y_max = (bbox_p[1 - sign[1]].y - ray->orig.y) * invdir.y;

    if (t_min > y_max || y_min > t_max)
        return false;
    if (y_min > t_min)
        t_min = y_min;
    if (y_max < t_max)
        t_max = y_max;

    z_min = (bbox_p[    sign[2]].z - ray->orig.z) * invdir.z;
    z_max = (bbox_p[1 - sign[2]].z - ray->orig.z) * invdir.z;

    if ((t_min > z_max) || (z_min > t_max))
        return false;
    if (z_min > t_min)
        t_min = z_min;
    if (z_max < t_max)
        t_max = z_max;

    // Object behind ray
    if (t_min < 0)
        return false;

    return true;
}

bool collide_ray_obb(const struct ray *r, const struct bbox *bbox,
                     const struct mat4 *model_matrix,
                     const struct mat4 *model_matrix_inv, float *_dist)
{
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

    ray.orig = model_orig;
    vec3_sub(&ray.orig, &r->orig);

    //DEBUG: Draw sphere / ray
    // struct sphere sphere_ray;
    // sphere_ray.orig = ray.orig;
    // sphere_ray.radius = 0.05f;
    // prim_draw_sphere(&sphere_ray, &COLOR_MAGENTA_HIGHLIGHT);

    // struct ray debug_ray = ray;
    // vec3_scalef(&debug_ray.dir, 10.0f);
    // prim_draw_ray(&debug_ray, &MAT4_IDENT, COLOR_YELLOW_HIGHLIGHT);

    UNUSED(model_matrix);
    UNUSED(model_matrix_inv);

	// Test intersection with the 2 planes perpendicular to the OBB's X axis
	{
        struct vec3 x_axis;
        x_axis.x = model_matrix->m[0][0];
        x_axis.y = model_matrix->m[1][0];
        x_axis.z = model_matrix->m[2][0];

        float x_scale = vec3_length(&x_axis);
        vec3_normalize(&x_axis);
        vec3_scalef(&x_axis, 1.0f / x_scale);

        // struct ray x_ray;
        // x_ray.orig = model_orig;
        // x_ray.dir = x_axis;
        // prim_draw_ray(&x_ray, &MAT4_IDENT, COLOR_RED_HIGHLIGHT);

        // struct sphere x_sphere;
        // x_sphere.orig = x_ray.orig;
        // vec3_add(&x_sphere.orig, &x_ray.dir);
        // x_sphere.radius = 0.05f;
        // prim_draw_sphere(&x_sphere, &COLOR_RED_HIGHLIGHT);

		float e = vec3_dot(&x_axis, &ray.orig);
		float f = vec3_dot(&ray.dir, &x_axis);

		if (fabs(f) > 0.00001f) // Standard case
        {
            // Intersection with the "left" plane
            float t1 = (e + bbox->p[0].x) / f;
            // Intersection with the "right" plane
			float t2 = (e + bbox->p[1].x) / f;
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
			if(-e + bbox->p[0].x > 0.0f || -e + bbox->p[1].x < 0.0f)
				return false;
		}
	}

    // Test y-axis
	{
        struct vec3 y_axis;
        y_axis.x = model_matrix->m[0][1];
        y_axis.y = model_matrix->m[1][1];
        y_axis.z = model_matrix->m[2][1];

        float scale = vec3_length(&y_axis);
        vec3_normalize(&y_axis);
        vec3_scalef(&y_axis, 1.0f / scale);

        // struct ray y_ray;
        // y_ray.orig = model_orig;
        // y_ray.dir = y_axis;
        // prim_draw_ray(&y_ray, &MAT4_IDENT, COLOR_GREEN_HIGHLIGHT);

        // struct sphere y_sphere;
        // y_sphere.orig = y_ray.orig;
        // vec3_add(&y_sphere.orig, &y_ray.dir);
        // y_sphere.radius = 0.05f;
        // prim_draw_sphere(&y_sphere, &COLOR_GREEN_HIGHLIGHT);

        float e = vec3_dot(&y_axis, &ray.orig);
		float f = vec3_dot(&ray.dir, &y_axis);

		if (fabs(f) > 0.00001f)
        {
            float t1 = (e + bbox->p[0].y) / f;
			float t2 = (e + bbox->p[1].y) / f;

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
			if(-e + bbox->p[0].y > 0.0f || -e + bbox->p[1].y < 0.0f)
				return false;
		}
	}

    // Test z-axis
	{
        struct vec3 z_axis;
        z_axis.x = model_matrix->m[0][2];
        z_axis.y = model_matrix->m[1][2];
        z_axis.z = model_matrix->m[2][2];

        float scale = vec3_length(&z_axis);
        vec3_normalize(&z_axis);
        vec3_scalef(&z_axis, 1.0f / scale);

        // struct ray z_ray;
        // z_ray.orig = model_orig;
        // z_ray.dir = z_axis;
        // prim_draw_ray(&z_ray, &MAT4_IDENT, COLOR_BLUE_HIGHLIGHT);

        // struct sphere z_sphere;
        // z_sphere.orig = z_ray.orig;
        // vec3_add(&z_sphere.orig, &z_ray.dir);
        // z_sphere.radius = 0.05f;
        // prim_draw_sphere(&z_sphere, &COLOR_BLUE_HIGHLIGHT);

        float e = vec3_dot(&z_axis, &ray.orig);
		float f = vec3_dot(&ray.dir, &z_axis);

		if (fabs(f) > 0.00001f)
        {
			float t1 = (e + bbox->p[0].z) / f;
			float t2 = (e + bbox->p[1].z) / f;

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
			if(-e + bbox->p[0].z > 0.0f || -e + bbox->p[1].z < 0.0f)
				return false;
		}
	}

	*_dist = t_min;
	return true;
}