/*
HEADER:
    char magic[4];              R1C0
    u32 version;                1
    u32 index_offset;           12
INDEX:
    u32 entry_count;            1
    ENTRY entries[entry_count];
        u32 type;               RIC_TEXTURE
        u32 name_hash;          0x38AF194A
        u32 name_len;           7
        u32 name_offset;        -->
        u32 blob_len;           65558
        u32 blob_offset;        -->
NAME:
    u8 data[data_len];          stones\0
BLOB: (RIC_TEXTURE)
    u16 width;                  128
    u16 height;                 128
    u8 bpp;                     32
    u32 data_len;               65536
    u8 data[data_len];          RGBA pixels
BLOB: (RIC_MESH)
    u32 vertex_size;
    u32 vertex_count;
    u32 element_count;
    u8 vertex_data[vertex_size * vertex_count];
    u32 element_data[element_count];
*/

struct chet_pack
{
    const char *filename;
    struct ric_stream *stream;

    u32 num_blobs;
    u32 index_offset;
};

struct chet_pack_index
{
    u32 entry_count;
    u32 entries_ofset;
};

struct chet_pack_entry
{
    u32 type;
    u32 name_hash;
    u32 name_len;
    u32 name_offset;
    u32 blob_len;
    u32 blob_offset;
};

struct chet_pack_blob_texture
{
    u16 width;
    u16 height;
    u8 bpp;
    u32 data_len;
    u32 data_offset;
};

struct chet_pack_blob_mesh
{
    u8 vertex_format;  // e.g. RIC_VERTEX_PBR
    u32 vertex_count;
    u32 element_count;
    u32 vertex_data_offset;
    u32 element_data_offset;
};