#include <GL/gl3w.h>
#include <stdint.h>

//Don't actually call these, it's just notes (w/ syntax highlighting)

static void notes_casey()
{
    //--------------------------------------------------------------------------
    // Handmade Hero
    //--------------------------------------------------------------------------
	//
	// NOTE(casey): Types
	//
	//#include <stdint.h>
	//#include <stddef.h>
	//#include <limits.h>
	//#include <float.h>

	typedef int8_t int8;
	typedef int16_t int16;
	typedef int32_t int32;
	typedef int64_t int64;
	typedef int32 bool32;

	typedef uint8_t uint8;
	typedef uint16_t uint16;
	typedef uint32_t uint32;
	typedef uint64_t uint64;

	typedef intptr_t intptr;
	typedef uintptr_t uintptr;

	typedef size_t memory_index;

	typedef float real32;
	typedef double real64;

	typedef int8 s8;
	typedef int8 s08;
	typedef int16 s16;
	typedef int32 s32;
	typedef int64 s64;
	typedef bool32 b32;
	typedef b32 b32x;

	typedef uint8 u8;
	typedef uint8 u08;
	typedef uint16 u16;
	typedef uint32 u32;
	typedef uint64 u64;

	typedef real32 r32;
	typedef real64 r64;
	typedef real32 f32;
	typedef real64 f64;
	//--------------------------------------------------------------------------

	// TODO: Better memory allocation (than malloc)
    // Windows:
    //   void *Block;
    //   Block = VirtualAlloc(0, Size, MEM_COMMIT, PAGE_READWRITE)
    //   VirtualFree(Block, 0, MEM_RELEASE);

    // TODO: Refactor collections of global variables into global structs

    // NOTE: Casey function pointers
    //
    //       #define GAME_UPDATE_AND_REDNER(name) void name(blah blah blah)
    //       typedef GAME_UPDATE_AND_RENDER(game_update_and_render)
    //       {
    //       }

    // TODO: Static vs. dynamic chunk data
    //       Streaming chunks

    // Note: Don't waste time writing efficient code that is only used in the
    //       internal (debug) build unless it actually offers value.

    // TODO: MSVC compiler switches
    //
    //       -MT (instead of -MD)
    //       Use static linking for MSVC (C runtime) instead of dynamic linking
    //      (i.e. get rid of "missing dll MSVCR120.dll" errors)
    //
    //       -Fm
    //       Produce map file for executable
    //
    //       /link opt:ref
    //       Only link necessary things (slightly smaller exe)

    // TODO: Don't use __rtdsc() intrinsic for game time, only for performance
    //       profiling. Use QueryPerformanceTimer() for game time on users' PCs.

    // Day 021: Dynamically loading game code, live edits
    // Day 022: PDB locked, rename it using timestamp?
    //          47:18 String for loop Path.GetPathWithoutFilename()
    char *buffer = 0;
    char *onePastLastSlash = buffer;
    for (char *scan = buffer; *scan; ++scan) {
        if (*scan == '\\') {
            onePastLastSlash = scan + 1;
            break;
        }
    }

    // Day 024: sizeof("blah") == 5; sizeof() counts null terminator!!!
    //          1:03:00 Translation Lookaside Buffer (TLB) VRAM -> RAM map

    // Day 025: 12:10 struct thread_context; info about current thread passed
    //          to any OS-specific calls. Essentially Thread.Current.
    #define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

    //          58:00 Memory-mapped file

    // Day 027: 1:04:00 round float to nearest int: (int)(f + 0.5f);

    // Day 041: Math in game programming
    //
    //  Partial differential equations
    //  Ordinary differential equations
    //  Minkowski algebra (collision detection)
    //  Control theory
    //      Feedback systems
    //      Springs
    //      Proportional derivative controllers (PDC)
    //  Operations research (optimization -> min/max)
    //      Metropolis algorithm (e.g. optimizing assembly)
    //  Prob & Stats
    //      PRNG
    //      Cryptography
	//  Lagrange Multipliers

    //  1:32:00 Use #define for global constants (e.g. VEC3_UP, VEC3_ZERO, etc.)

	// Day 043: Player motion:
	//
	//			Position = 1/2at^2 + vt + p
	//			Velocity = at + p
	//			Acceleration = 10 // From d-pad

	// Prevent double speed when moving diagonally
	// TODO: How to extend this to 3D?
	if (player.acc.x != 0.0f && player.acc.y != 0.0f)
	{
		player.acc.x *= 0.707106781187f; // cos(45 deg)
		player.acc.y *= 0.707106781187f;
	}

	// Coefficient of restitution = how much energy to absorb during collision.

	// Day 045: Collision via nearest walkable point
	//
	//			49:35 For a bounding box, clip would-be destination point to
	//			bounding volume to find the closest point in this particular
	//			space.

    // Day 055:

    // Modulus by power of two:
    // Subtracting 1 from a power of two gives us a bit mask to perform
    // a perfect modulus operation.
    // (16 - 1) & value == (0b10000 - 1) & value ==
    //       15 & value ==       0b01111 & value ==
    //       value % 16 ==       value % 0b10000
    HashValue % ArrayCount(HashTable);
    HashValue & (ArrayCount(HashTable) - 1);

    // Array indexing with pointer arithmetic:
    Item *item = HashTable[HashSlot];
    Item *item = HashTable + HashSlot;

    // In-place hash tables:
    struct tree
    {
        u32 x;
        u32 y;

        struct thing *hash_next;
    };

    struct forest
    {
        char *name;

        struct tree hash_trees[4096]; // TODO: Prime number for avalanche
    };

    // Get tree from sparse forest using hash
    struct tree *get_tree(struct forest *forest, u32 x, u32 y)
    {
        // Use proper hash function!
        u32 hash = 17*x + 13*y;
        u32 slot = hash % ArrayCount(forest->hash_trees);

        struct tree *tree = forest->hash_trees + slot;
        do
        {
            if (tree->x == x && tree->y == y)
                break;

            tree = tree->hash_next
        } while (tree);

        return tree;
    }

    //  1:22:40 Refactoring hash map out of specific struct use-case
    //  1:36:00 Run-length encoding (RLE) for tile chunks. Split chunks into
    //          "bands" of empty->filled(+data)->empty->filled(+data)->etc..

    // Day 060:
	//
    //      Alpha destination buffer:
    //          Allows rendering translucent objects front-to-back??

	// Day 134:
	//
	//	1:14:22 - 1:22:06 Switching from OOP to data-oriented /
	//					  compression-oriented

    // Day 146:
    //
    //  41:30
    struct hha_header
    {
        u32 magic;
        u32 version;

        u32 tag_count;
        u32 asset_count;
        u32 asset_type_count;

        u64 tags_offset;
        u64 assets_offset;
        u64 asset_types_offset;
    };
    struct hha_tag
    {
        u32 id;
        r32 value;
    };
    struct hha_asset_type
    {
        u32 type_id;
        u32 first_asset_idx;
        u32 asset_count;
    };

    struct hha_bitmap
    {
        u32 size[2];
    };
    struct hha_sound
    {
        u32 sample_count;
        u32 next_sound_to_play;
    };
    struct hha_asset
    {
        u64 data_offset;
        u32 first_tag_idx;
        u32 tag_count;
        union
        {
            struct hha_bitmap bitmap;
            struct hha_sound sound;
        };
    };

    struct game_assets
    {

    };

    struct game_assets assets = { 0 };
    // Populate assets

    FILE *out = fopen("casey.hha", "wb");
    if (out)
    {
        hha_header header = { 0 };
        header.magic = MAGIC;
        header.version = VERSION;
    }

	// Day 237:
	//
	//  1:32:00 glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	//  GL_MODULATE, GL_DECAL, GL_BLEND, GL_REPLACE

	// Day 242:
	//
	//  19:00
	//  char *Vendor, Renderer, Version, ShadingLanguageVersion, Extensions
	//  glGetString(xxx):
	//		GL_VENDOR
	//		GL_RENDERER
	//		GL_VERSION
	//		GL_SHADING_LANGUAGE_VERSION
	//		GL_EXTENSIONS

	// Day 243:
	//
	//  Asynchronous texture loading:
	//		Create multiple GL contexts, one of which has the sole job of
	//		uploading textures to the GPU.
	//
	//	1:26:50 Allocate/Deallocate textures (w/ tex params)





    //--------------------------------------------------------------------------
    // Blog
    //--------------------------------------------------------------------------
    // Stream 0008: "Compatible camera controls"; allow camera to moved around
    //              while in edit mode the same way the camera is moved in e.g.
    //              Blender, Maya, 3DS Max, etc. to help artists work fast.
    // https://mollyrocket.com/casey/stream_0008.html

    // Stream 0017: CaseyTown. It is a land where the birds sing in emergent
    //              counterpoint, where children laugh and play and know nothing
    //              of war, where humans live in harmony with nature, where
    //              newcomers are greeted with their own personal roasted pig
    //              and a crown of rare orchids before being ceremonially
    //              plunged into a restorative bath of chocolate milk and rock
    //              salt (it exfoliates — look it up).

    // Stream 0023: Scope braces in switch statements.
    // https://mollyrocket.com/casey/stream_0023.html
    switch (filter_mode)
    {
        case LISTER_FILTER_MODE_OFF:
        {
            // NOTE(casey): Ignored
        } break;

        case LISTER_FILTER_MODE_POSITIVE:
        {
            int handler_result = NOT_APPLICABLE;
            special.handler(e, &handler_result);
            if (handler_result != NOT_APPLICABLE)
            {
                passes_special = (handler_result != 0);
            }
        } break;

        case LISTER_FILTER_MODE_NEGATIVE:
        {
            int handler_result = NOT_APPLICABLE;
            special.handler(e, &handler_result);
            if (handler_result != NOT_APPLICABLE)
            {
                passes_special = (handler_result == 0);
            }
        } break;

        default:
        {
            assert(!"Unrecognized special filter mode");
        } break;
    }

    // Stream 0028: Reusable Components (API)
    // https://www.youtube.com/watch?v=ZQ5_u8Lgvyk
    //
    // 31:00
    // API-provided services

    // A: ChunkLoad(filename) tightly couples file I/O and interpretation
    //    of the actual raw chunk data.
    chunk = ChunkLoad(filename);

    // B: Callbacks are bad for flow control. ChunkLoad(filename) tightly
    //    couples file I/O and interpretation of the data.
    SetFileCallbacks(MyOpen, MyRead, MyClose);
    chunk = ChunkLoad(filename); // Calls MyRead for I/O

    // C: Chunk is initialized from data, but where is its memory? Unless it's
    //    being returned by value, it doesn't let us manage allocation.
    chunk = ChunkInit(filedata); // Take raw data from user

    // D: API is still allocating memory for the uncompressed data.
    filedata = DecompressData(raw_filedata);
    chunk = ChunkInit(filedata);
    FreeFileData(filedata);

    // E: More callbacks... flow control problems.
    SetMemoryCallbacks(MyAlloc, MyFree);
    filedata = DecompressData(raw_filedata); // Calls MyAlloc for memory.
    chunk = ChunkInit(filedata);

    // F: Totally decoupled, but still requires API to interpret data.
    size = GetProcessedSize(raw_filedata);
    filedata = MyAlloc(size);
    DecompressData(raw_filedata, &filedata);
    chunk = ChunkInit(filedata);

    // G: Get rid of ChunkInit and just give me a chunk.
    chunk = NewChunk();
    MyRead(file, sizeof(chunk), &chunk);

    // H: Let me define my own Chunks, just tell me what the structure is.
    chunk = MyAlloc(sizeof(chunk));
    MyRead(file, sizeof(chunk), &chunk);

    // 40:40: Retained mode vs. immediate mode API state
    //        Instead of CreateJoint() / DeleteJoint() and constantly keeping
    //        state sync'd, have DoJoint() which is called only on relevant
    //        frames to perform the check e.g. while X is down.

    // 45:00: Write the usage code for the API first. Define all of the ways
    //        the API might be used, then design or choose the API.

    // 46:25: All retained mode constructs should have immediate mode
    //        equivalents. E.g. Update(big_struct) ->
    //        Update(a), Update(b), Update(a, b, c), etc.

    // 47:00: All callback / inheritance-based constructs should have
    //        equivalents that do neither. Forfeiting flow control must be
    //        kept optional.

    // 47:24: API should never require API-specific data types for things which
    //        are commonly already present in game code (e.g. Vector, Matrix)

    // 47:50: Any API function that might not be considered atomic should be
    //        replacable with a few, more granular, calls.

    // 48:35: Any data which doesn't have a clear reason for being opaque should
    //        be fully transparent (init, update, I/O, etc.)

    // 49:32: API resource management routines (e.g. memory, file, string, etc.)
    //        must always be optional.

    // 49:42: API file formats must always be optional.

    // 49:44: Full run-time source code must be available.

    //--------------------------------------------------------------------------
    // IMGUI
    //--------------------------------------------------------------------------


    //--------------------------------------------------------------------------
    // GJK
    //--------------------------------------------------------------------------
    // Aside: "Jump tables"


    // -------------------------------------------------------------------------
}

static void notes_robert()
{
    //--------------------------------------------------------------------------
    // Double Buffer
    //
    //  Can be used for things other than graphics. E.g. physics or AI. Store
    //  previous state as a read-only replica while doing work on the current
    //  one. Swap pointers when done.
    //
    //
    //--------------------------------------------------------------------------
}

static void notes_gl()
{
    /*************************************************************************
    | Generate buffers
    |
    | Load vertex data into array
    | Load element array
    |
    **************************************************************************
    | Generate GLSL program
    |
    | Load vertex shader from file, compile, error check
    | Load fragment shader "    "   "   "   "   "   "
    | Create program, attach compiled shaders, link program
    | Detach and delete shaders
    |
    | Get attribute locations
    | Get uniform locations
    |
    | Delete program when done with it
    |
    **************************************************************************
    | Set up vao
    |
    | Bind VBO
    | Create and bind VAO
    |   Set VBO attrib pointers and enable them
    |   Unbind VBO
    |   Bind EBO
    | Unbind VAO
    | Unbind EBO
    |
    **************************************************************************
    | Update model
    |
    | Use program
    | Update uniforms
    | Update VBO (for animation)
    |
    **************************************************************************
    | Render model
    |
    | Bind program / VAO
    | Update uniforms (texture swapping??)
    | Draw elements
    | Unbind program / VAO
    |
    **************************************************************************
    | Clean up
    |
    | Delete shaders (if not already)
    | Delete program
    | Delete buffers (VBO, EBO)
    | Delete VAO
    | Delete textures
    |
    *************************************************************************/

    // VBO = Vertex Buffer Object
    // EBO = Element Buffer Object (indices into VBO, for vertex reuse)
    // VAO = "Vertex Array Object" (tracks EBO, and VBO pointers, pointer state)

    //--------------------------------------------------------------------------
    // Preparing and using VAOs
    //--------------------------------------------------------------------------

    // Step 1: Load data into buffers

    GLuint vvv = 0;
    GLuint eee = 0;

    glBindBuffer(GL_ARRAY_BUFFER, vvv);
    glBufferData(GL_ARRAY_BUFFER, vvv);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eee);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, eee);

    // Step 2: Compile shader program

    GLuint program_id = 0;

    //Load/compile shaders and program...

    glLinkProgram(program_id);
    GLint aaa = glGetAttribLocation(program_id, "aaa");
    GLint bbb = glGetAttribLocation(program_id, "bbb");
    GLint ccc = glGetAttribLocation(program_id, "ccc");

    // Step 3: Bind attrib pointers, attrib enables and ebo to vao

    GLuint vao = 0;
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vvv); //needed for attrib pointer calls,
                                        //not stored in vao
    glVertexAttribPointer(aaa, ...);
    glVertexAttribPointer(bbb, ...);
    glVertexAttribPointer(ccc, ...);
    glEnableVertexAttribArray(aaa);
    glEnableVertexAttribArray(bbb);
    glEnableVertexAttribArray(ccc);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eee);

    //Note: Setting vec3 W component to 0.0f makes a mesh ignore the camera
    //      and be rendered relative to the viewport. This might be useful for
    //      effects like a skybox.
}

static void notes_geom()
{
#if 0
    internal inline struct vec3 *v3_add(struct vec3 *a, const struct vec3 *b)
    {
        a->x += b->x;
        a->y += b->y;
        a->z += b->z;
        return a;
    }
    internal inline struct vec3 *v3_sub(struct vec3 *a, const struct vec3 *b)
    {
        a->x -= b->x;
        a->y -= b->y;
        a->z -= b->z;
        return a;
    }
    internal inline struct vec3 *v3_scale(struct vec3 *v, const struct vec3 *s)
    {
        v->x *= s->x;
        v->y *= s->y;
        v->z *= s->z;
        return v;
    }
    internal inline struct vec3 *v3_scalef(struct vec3 *v, float s)
    {
        v->x *= s;
        v->y *= s;
        v->z *= s;
        return v;
    }
    internal inline float v3_dot(const struct vec3 *a, const struct vec3 *b)
    {
        return a->x*b->x + a->y*b->y + a->z*b->z;
    }
    internal inline struct vec3 v3_cross(const struct vec3 *a, const struct vec3 *b)
    {
        struct vec3 c;
        c.x = a->y * b->z - a->z * b->y;
        c.y = a->z * b->x - a->x * b->z;
        c.z = a->x * b->y - a->y * b->x;
        return c;
    }
    internal inline float v3_length(const struct vec3 *v)
    {
        return sqrtf(
            v->x * v->x +
            v->y * v->y +
            v->z * v->z
        );
    }
    internal inline struct vec3 *v3_negate(struct vec3 *v)
    {
        v->x = -v->x;
        v->y = -v->y;
        v->z = -v->z;
        return v;
    }
    internal inline struct vec3 *v3_normalize(struct vec3 *v)
    {
        float len = 1.0f / v3_length(v);
        v->x *= len;
        v->y *= len;
        v->z *= len;
        return v;
    }
    internal inline struct vec3 *v3_positive(struct vec3 *v)
    {
        if (v->x < 0) v->x *= -1;
        if (v->y < 0) v->y *= -1;
        if (v->z < 0) v->z *= -1;
        return v;
    }
    internal inline int v3_equals(const struct vec3 *a, const struct vec3 *b)
    {
        return (a->x == b->x && a->y == b->y && a->z == b->z);
    }

    // TODO: Refactor *all* printf stuff out into a single file
    internal inline void v3_print(struct vec3 *v)
    {
        printf("Vec XYZ: %10f %10f %10f\n", v->x, v->y, v->z);
    }
#elif 0
    internal inline struct vec3 v3_add(const struct vec3 *a, const struct vec3 *b)
    {
        struct vec3 c;
        c.x = a->x + b->x;
        c.y = a->y + b->y;
        c.z = a->z + b->z;
        return c;
    }
    internal inline struct vec3 v3_sub(const struct vec3 *a, const struct vec3 *b)
    {
        struct vec3 c;
        c.x = a->x - b->x;
        c.y = a->y - b->y;
        c.z = a->z - b->z;
        return c;
    }
    internal inline struct vec3 v3_scale(const struct vec3 *v, const struct vec3 *s)
    {
        struct vec3 r;
        r.x = v->x * s->x;
        r.y = v->y * s->y;
        r.z = v->z * s->z;
        return r;
    }
    internal inline struct vec3 v3_scalef(const struct vec3 *v, float s)
    {
        struct vec3 r;
        r.x = v->x * s;
        r.y = v->y * s;
        r.z = v->z * s;
        return r;
    }
    internal inline float v3_dot(const struct vec3 *a, const struct vec3 *b)
    {
        return a->x*b->x + a->y*b->y + a->z*b->z;
    }
    internal inline struct vec3 v3_cross(const struct vec3 *a, const struct vec3 *b)
    {
        struct vec3 c;
        c.x = a->y * b->z - a->z * b->y;
        c.y = a->z * b->x - a->x * b->z;
        c.z = a->x * b->y - a->y * b->x;
        return c;
    }
    internal inline float v3_length(const struct vec3 *v)
    {
        return sqrtf(
            v->x * v->x +
            v->y * v->y +
            v->z * v->z
        );
    }
    internal inline struct vec3 v3_negate(const struct vec3 *v)
    {
        struct vec3 r;
        r.x = -v->x;
        r.y = -v->y;
        r.z = -v->z;
        return r;
    }
    internal inline void v3_normalize(struct vec3 *v)
    {
        float len = 1.0f / v3_length(v);
        v->x *= len;
        v->y *= len;
        v->z *= len;
        return v;
    }
    internal inline struct vec3 v3_positive(const struct vec3 *v)
    {
        struct vec3 r;
        r.x = fabsf(v->x);
        r.y = fabsf(v->y);
        r.z = fabsf(v->z);
        return r;
    }
    internal inline int v3_equals(const struct vec3 *a, const struct vec3 *b)
    {
        return (a->x == b->x && a->y == b->y && a->z == b->z);
    }

    // TODO: Refactor *all* printf stuff out into a single file
    internal inline void v3_print(struct vec3 *v)
    {
        printf("Vec XYZ: %10f %10f %10f\n", v->x, v->y, v->z);
    }
#elif 0
    internal inline struct vec3 v3_add(struct vec3 a, struct vec3 b)
    {
        a.x += b.x;
        a.y += b.y;
        a.z += b.z;
        return a;
    }
    internal inline struct vec3 v3_sub(struct vec3 a, struct vec3 b)
    {
        a.x -= b.x;
        a.y -= b.y;
        a.z -= b.z;
        return a;
    }
    internal inline struct vec3 v3_scale(struct vec3 v, struct vec3 s)
    {
        v.x *= s.x;
        v.y *= s.y;
        v.z *= s.z;
        return v;
    }
    internal inline struct vec3 v3_scalef(struct vec3 v, float s)
    {
        v.x *= s;
        v.y *= s;
        v.z *= s;
        return v;
    }
    internal inline float v3_dot(struct vec3 a, struct vec3 b)
    {
        return a.x*b.x + a.y*b.y + a.z*b.z;
    }
    internal inline struct vec3 v3_cross(struct vec3 a, struct vec3 b)
    {
        struct vec3 c;
        c.x = a.y * b.z - a.z * b.y;
        c.y = a.z * b.x - a.x * b.z;
        c.z = a.x * b.y - a.y * b.x;
        return c;
    }
    internal inline float v3_length(struct vec3 v)
    {
        return sqrtf(
            v.x * v.x +
            v.y * v.y +
            v.z * v.z
        );
    }
    internal inline struct vec3 v3_negate(struct vec3 v)
    {
        v.x = -v.x;
        v.y = -v.y;
        v.z = -v.z;
        return v;
    }
    internal inline struct vec3 v3_normalize(struct vec3 v)
    {
        float len = 1.0f / v3_length(v);
        v.x *= len;
        v.y *= len;
        v.z *= len;
        return v;
    }
    internal inline struct vec3 v3_positive(struct vec3 v)
    {
        v.x = fabsf(v.x);
        v.y = fabsf(v.y);
        v.z = fabsf(v.z);
        return v;
    }
    internal inline int v3_equals(struct vec3 a, struct vec3 b)
    {
        return (a.x == b.x && a.y == b.y && a.z == b.z);
    }

    // TODO: Refactor *all* printf stuff out into a single file
    internal inline void v3_print(struct vec3 v)
    {
        printf("Vec XYZ: %10f %10f %10f\n", v.x, v.y, v.z);
    }
#endif
}
