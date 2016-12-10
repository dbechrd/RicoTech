#include <GL/gl3w.h>

//Don't actually call this, it's just notes (w/ syntax highlighting)
void notes()
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