#ifndef RICO_INTERNAL_CEREAL_H
#define RICO_INTERNAL_CEREAL_H

//static void cereal_(struct cereal_config config);

static void cereal_data(struct ric_config *config, struct ric_data *data);
static void cereal_s32(struct ric_config *config, s32 *data);
static void cereal_u32(struct ric_config *config, u32 *data);
static void cereal_float(struct ric_config *config, float *data);
static void cereal_bool(struct ric_config *config, bool *data);
static void cereal_string(struct ric_config *config, char *data);
static void cereal_ric_test_entity(struct ric_config *config,
                                   struct ric_test_entity *data);

#endif