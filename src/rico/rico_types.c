s32 SCREEN_WIDTH = 1600;
s32 SCREEN_HEIGHT = 900;

// 0x4f434952
static const u32 RIC_SAV_MAGIC = 'R' | 'I' << 8 | 'C' << 16 | 'O' << 24;

const char *ric_asset_type_string[]     = { RIC_ASSET_TYPES(ENUM_STRING) };
const char *ric_audio_state_string[]    = { RIC_AUDIO_STATES(ENUM_STRING) };
const char *ric_light_type_string[]     = { RIC_LIGHT_TYPES(ENUM_STRING) };
const char *ric_engine_state_string[]   = { RIC_ENGINE_STATES(ENUM_STRING) };
const char *ric_string_slot_string[]    = { RIC_STRING_SLOTS(ENUM_STRING) };
const char *ric_error_string[]          = { RIC_ERRORS(ENUM_STRING) };
const char *ric_widget_string[]         = { RIC_WIDGETS(ENUM_STRING) };

const u32 ric_asset_type_size[] = { RIC_ASSET_TYPES(ENUM_META) };
