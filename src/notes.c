#include <GL/gl3w.h>

//Don't actually call these, it's just notes (w/ syntax highlighting)

void notes_casey()
{
    // TODO: Clarify static?
    #define internal static
    #define local_persist static
    #define global_variable static  //Only true if single translation unit

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
    //
    char *buffer = 0;
    char *onePastLastSlash = buffer;
    for (char *scan = buffer; *scan; ++scan) {
        if (*scan == '\\') {
            onePastLastSlash = scan + 1;
            break;
        }
    }

    //          Casey uses C++ like C with oper/func overloading
}

void notes_gl()
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