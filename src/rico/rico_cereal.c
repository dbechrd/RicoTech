static void ric_uid_sav(struct ric_stream *stream, struct ric_uid *data)
{
    ADD_FIELD(V_EPOCH, pkid, pkid);
    ADD_FIELD(V_EPOCH, enum ric_asset_type, type);
    ADD_FIELD(V_EPOCH, buf32, name);
}

static void ric_transform_sav(struct ric_stream *stream,
                              struct ric_transform *data)
{
    ADD_FIELD(V_EPOCH, struct vec3, position);
    ADD_FIELD(V_EPOCH, struct quat, orientation);
    ADD_FIELD(V_EPOCH, struct vec3, scale);
}

static void ric_object_sav(struct ric_stream *stream, struct ric_object *data)
{
    ADD_STRUCT(V_EPOCH, ric_uid, uid);
    ADD_STRUCT(V_EPOCH, ric_transform, xform);
    ADD_FIELD(V_EPOCH, pkid, mesh_id);
    ADD_FIELD(V_EPOCH, pkid, material_id);
}