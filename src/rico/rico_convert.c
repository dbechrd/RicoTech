#define MAX_TOKENS 32 * 1024 * 1024

#define OBJ_TOKEN_TYPES(f) \
    f(OBJ_TOK_UNKNOWN)     \
    f(OBJ_TOK_COMMENT)         /* [www.blender.org] */ \
    f(OBJ_TOK_KEYWORD)         /* [vt]              */ \
    f(OBJ_TOK_SEPARATOR)       /* [ ]               */ \
    f(OBJ_TOK_IDENTIFIER)      /* [fabRICO_plants]   */ \
    f(OBJ_TOK_LITERAL)         /* [0.8542]          */ \
    f(OBJ_TOK_INDEX)           /* [3]               */ \
    f(OBJ_TOK_INDEX_SEPARATOR) /* [/]               */ \
    f(OBJ_TOK_NEWLINE)         /* [ ]               */ \
    f(OBJ_TOK_COUNT)

enum obj_token_type { OBJ_TOKEN_TYPES(GEN_LIST) };
static const char *obj_token_type_string[] = { OBJ_TOKEN_TYPES(GEN_STRING) };

#define OBJ_KEYWORD_TYPES(f) \
    f(OBJ_KW_INVALID,        "\0") \
    f(OBJ_KW_IGNORED,        "\0") \
    /* Used */ \
    f(OBJ_KW_VERTEX,         "v")      /* [v]  geometric vertices */ \
    f(OBJ_KW_UVCOORD,        "vt")     /* [vt] texture vertices   */ \
    f(OBJ_KW_NORMAL,         "vn")     /* [vn] vertex normals     */ \
    f(OBJ_KW_FACE,           "f")      /* [f]                     */ \
    f(OBJ_KW_GROUP_NAME,     "g")      /* [g]                     */ \
    f(OBJ_KW_OBJECT_NAME,    "o")      /* [o]                     */ \
    f(OBJ_KW_MATERIAL_NAME,  "usemtl") /* [usemtl]                */ \
    f(OBJ_KW_MATERIAL_LIB,   "mtllib") /* [mtllib]                */ \
    /* Ignored */ \
    f(OBJ_KW_SMOOTHING_GROUP, "s")     /* [s]                     */ \
    f(OBJ_KW_COUNT, "\0")

enum obj_keyword_type { OBJ_KEYWORD_TYPES(GEN_LIST) };
static const char *obj_keyword_type_string[] = { OBJ_KEYWORD_TYPES(GEN_STRING) };
static const char *obj_keyword_values[] = { OBJ_KEYWORD_TYPES(GEN_STRING_VALUES) };

struct obj_token
{
    enum obj_token_type type;
    enum obj_keyword_type keyword;
    u8 len;
    const char *value; // BE CAREFUL THIS IS *NOT* A NULL-TERMINATED STRING!!!
};

struct obj_file_header
{
    int vertex_count;
    int uvcoord_count;
    int normal_count;
    int face_count;
    int object_count;
};
struct obj_file
{
    struct obj_file_header header;
    struct obj_token *tokens;
};

static int rico_convert(int argc, char **argv)
{
    enum RICO_error err = ERR_INVALID_PARAMS;

    for (int i = 1; i < argc; ++i)
    {
        printf("args[%d] %s\n", i, argv[i]);
    }

    if (argc == 3 &&
        strcmp(argv[1], "convert-obj") == 0)
    {
        err = rico_convert_obj(argv[2]);
    }
    else
    {
        printf("Usage: rico convert-obj <filename>\n");
    }

    return err;
}
static int rico_convert_obj(const char *filename)
{
    enum RICO_error err = SUCCESS;
    printf("Converting %s\n", filename);
    return load_obj_file_new(filename);
    return err;
}
static int load_obj_file_new(const char *filename)
{
    UNUSED(obj_token_type_string);
    UNUSED(obj_keyword_type_string);

    enum RICO_error err;

    u32 length;
    char *buffer;
    int line_number = 1;

    printf("[ obj][load] filename=%s\n", filename);
    err = file_contents(filename, &length, &buffer);
    if (err) goto cleanup;

    char *bp = buffer;
	char *value;
    struct obj_file file = { 0 };
    // TODO: Memory arena, push tokens, pop when done parsing.
    file.tokens = calloc(MAX_TOKENS, sizeof(*file.tokens));
    int tok_idx = 0;
    struct obj_token *keyword = NULL;
    while (tok_idx < MAX_TOKENS)
    {
        value = bp;

        if (*value == '\n')
        {
            file.tokens[tok_idx].type = OBJ_TOK_NEWLINE;
            bp++;

            // TODO: Validate current line grammar

            keyword = NULL;
            line_number++;
        }
        else if (*value == '#')
        {
            file.tokens[tok_idx].type = OBJ_TOK_COMMENT;
            while (*bp && *bp != '\n') bp++;
        }
        else if (keyword == NULL)
        {
            while (*bp && *bp != ' ') bp++;
            u32 len = (u32)(bp - value);

            for (int kw = 0; kw < OBJ_KW_COUNT; ++kw)
            {
                if (dlb_strlen(obj_keyword_values[kw]) != len) continue;

                u32 c = 0;
                while (value[c] == obj_keyword_values[kw][c]) c++;

                if (c == len)
                {
                    file.tokens[tok_idx].type = OBJ_TOK_KEYWORD;
                    file.tokens[tok_idx].keyword = (enum obj_keyword_type)kw;
                    keyword = &file.tokens[tok_idx];
                    break;
                }
            }

            if (keyword == NULL)
            {
                err = RICO_ERROR(
                    ERR_OBJ_PARSE_FAILED,
                    "Expected comment or keyword at beginning of line %d. Found: %.*s\n",
                    line_number, len, value
                );
                goto cleanup;
            }
            // File header
            else if (keyword->keyword == OBJ_KW_VERTEX)
                file.header.vertex_count++;
            else if (keyword->keyword == OBJ_KW_UVCOORD)
                file.header.uvcoord_count++;
            else if (keyword->keyword == OBJ_KW_NORMAL)
                file.header.normal_count++;
            else if (keyword->keyword == OBJ_KW_FACE)
                file.header.face_count++;
            else if (keyword->keyword == OBJ_KW_OBJECT_NAME)
                file.header.object_count++;
            // Ignored keywords
            else if (keyword->keyword == OBJ_KW_SMOOTHING_GROUP)
            {
                keyword->keyword = OBJ_KW_IGNORED;
                while (*bp && *bp != '\n') bp++;
            }
        }
        else if (*value == ' ')
        {
            file.tokens[tok_idx].type = OBJ_TOK_SEPARATOR;
            bp++;
        }
        else if (*value == '/')
        {
            file.tokens[tok_idx].type = OBJ_TOK_INDEX_SEPARATOR;
            bp++;
        }
        else if (*value == '-' || (*value >= '0' && *value <= '9'))
        {
            if (keyword && (keyword->keyword == OBJ_KW_VERTEX ||
                            keyword->keyword == OBJ_KW_UVCOORD ||
                            keyword->keyword == OBJ_KW_NORMAL))
            {
                file.tokens[tok_idx].type = OBJ_TOK_LITERAL;
            }
            else if (keyword && keyword->keyword == OBJ_KW_FACE)
            {
                file.tokens[tok_idx].type = OBJ_TOK_INDEX;
            }
            else
            {
                err = RICO_ERROR(ERR_OBJ_PARSE_FAILED,
                                  "Unexpected digit '%c' on line %d\n",
                                  *value, line_number);
                goto cleanup;
            }
            while (*bp && *bp != '\n' && *bp != ' ' && *bp != '/') bp++;
        }
        else if (keyword && (keyword->keyword == OBJ_KW_GROUP_NAME ||
                             keyword->keyword == OBJ_KW_OBJECT_NAME ||
                             keyword->keyword == OBJ_KW_MATERIAL_NAME ||
                             keyword->keyword == OBJ_KW_MATERIAL_LIB))
        {
            file.tokens[tok_idx].type = OBJ_TOK_LITERAL;
            while (*bp && *bp != '\n' && *bp != ' ') bp++;
        }

        if (file.tokens[tok_idx].type == OBJ_TOK_UNKNOWN)
        {
            err = RICO_ERROR(ERR_OBJ_PARSE_FAILED,
                              "Unexpected character '%c' on line %d\n",
                              *value, line_number);
            goto cleanup;
        }

        file.tokens[tok_idx].value = value;

        // If length greater than 255 characters, truncate to 255
        // NOTE: If this is ever a problem, change obj_token.len to u16
        u32 len = (u32)(bp - value);
        file.tokens[tok_idx].len = (len > 255) ? 255 : (u8)len;

        tok_idx++;
        if (*bp == 0) break;
    }

    printf("---------------------\n"
           "   TOKENS: %d\n"
           " vertices: %d\n"
           " uvcoords: %d\n"
           "  normals: %d\n"
           "    faces: %d\n"
           "  objects: %d\n"
           "---------------------\n",
           tok_idx,
           file.header.vertex_count,
           file.header.uvcoord_count,
           file.header.normal_count,
           file.header.face_count,
           file.header.object_count);

    if (tok_idx == MAX_TOKENS && *bp)
    {
        err = RICO_ERROR(ERR_OBJ_PARSE_FAILED,
                          "Ran out of memory for tokens on line %d.\n",
                          line_number);
        goto cleanup;
    }

#if 0
    struct obj_token *token = file.tokens;
    for (int i = 0 ; i < tok_idx; ++i)
    {
        printf("%s ", obj_token_type_string[token->type]);
        if (token->type == OBJ_TOK_KEYWORD)
            printf("%s ", obj_keyword_type_string[token->keyword]);
        printf("%.*s\n", token->len, token->value);
        token++;
    }
#endif

cleanup:
    //printf("Press [enter] to kill tokens\n");
    //fflush(stdout);
    //getchar();
    if (file.tokens) free(file.tokens);
    //printf("Press [enter] to kill buffer\n");
    //fflush(stdout);
    //getchar();
    if (buffer) free(buffer);
    //printf("Press [enter] to exit program\n");
    //fflush(stdout);
    //getchar();
    return err;
}

#if 0
enum obj_keyword_type
{
    OBJ_KW_INVALID,

    //-------------------------------------------
    // Vertex data
    //-------------------------------------------
    OBJ_KW_VERTEX,       // [v]  geometric vertices
    OBJ_KW_UVCOORD,      // [vt] texture vertices
    OBJ_KW_NORMAL,       // [vn] vertex normals
    OBJ_KW_PARAM,        // [vp] parameter space vertices free-form
                         //      curve/surface attributes
    OBJ_KW_CS_TYPE,      // [cstype] rational or non-rational forms of curve
                         //          or surface type: basis matrix, Bezier,
                         //          B-spline, Cardinal, Taylor
    OBJ_KW_DEGREE,       // [deg]
    OBJ_KW_BASIS_MATRIX, // [bmat]
    OBJ_KW_STEP_SIZE,    // [step]

    //-------------------------------------------
    // Elements
    //-------------------------------------------
    OBJ_KW_POINT,    // [p]
    OBJ_KW_LINE,     // [l]
    OBJ_KW_FACE,     // [f]
    OBJ_KW_CURVE,    // [curv]
    OBJ_KW_CURVE_2D, // [curv2]
    OBJ_KW_SURFACE,  // [surf]

    //-------------------------------------------
    // Free-form curve/surface body statements
    //-------------------------------------------
    OBJ_KW_PARAM_VALUES,    // [parm]
    OBJ_KW_OUTER_TRIM_LOOP, // [trim]
    OBJ_KW_INNER_TRIM_LOOP, // [hole]
    OBJ_KW_SPECIAL_CURVE,   // [scrv]
    OBJ_KW_SPECIAL_POINT,   // [sp]
    OBJ_KW_END,             // [end]

    //-------------------------------------------
    // Connectivity between free-form surfaces
    //-------------------------------------------
    OBJ_KW_CONNECT,         // [con]

    //-------------------------------------------
    // Grouping
    //-------------------------------------------
    OBJ_KW_GROUP_NAME,      // [g]
    OBJ_KW_SMOOTHING_GROUP, // [s]
    OBJ_KW_MERGING_GROUP,   // [mg]
    OBJ_KW_OBJECT_NAME,     // [o]

    //-------------------------------------------
    // Display/render attributes
    //-------------------------------------------
    OBJ_KW_BEVEL_INTERP,    // [bevel]
    OBJ_KW_COLOR_INTERP,    // [c_interp]
    OBJ_KW_DISSOLVE_INTERP, // [d_interp]
    OBJ_KW_LOD,             // [lod]
    OBJ_KW_MATERIAL_NAME,   // [usemtl]
    OBJ_KW_MATERIAL_LIB,    // [mtllib]
    OBJ_KW_SHADOW,          // [shadow_obj]
    OBJ_KW_RAYTRACE,        // [trace_obj]
    OBJ_KW_CURVE_APPROX,    // [ctech]
    OBJ_KW_SURFACE_APPROX,  // [stech]

    OBJ_KW_COUNT
};
#endif
