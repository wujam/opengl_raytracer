#include "SDLGraphicsProgram.h"

// Initialization function
// Returns a true or false value based on successful completion of setup.
// Takes in dimensions of window.
SDLGraphicsProgram::SDLGraphicsProgram(int w, int h):screenWidth(w),screenHeight(h){
	tick = 0;
    // Initialization flag
    bool success = true;
    // String to hold any errors that occur.
    std::stringstream errorStream;
    // The window we'll be rendering to
    gWindow = NULL;
    
    std::cout << "didn't segfault from loading object" << std::endl;
    // Render flag
    // Initialize SDL
    if(SDL_Init(SDL_INIT_VIDEO)< 0){
        errorStream << "SDL could not initialize! SDL Error: " << SDL_GetError() << "\n";
        success = false;
    }
    else{
        //Use OpenGL 3.3 core
        SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
        SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 3 );
        SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
        // We want to request a double buffer for smooth updating.
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

        //Create window
        gWindow = SDL_CreateWindow( "Lab",
                SDL_WINDOWPOS_UNDEFINED,
                SDL_WINDOWPOS_UNDEFINED,
                screenWidth,
                screenHeight,
                SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );

        // Check if Window did not create.
        if( gWindow == NULL ){
            errorStream << "Window could not be created! SDL Error: " << SDL_GetError() << "\n";
            success = false;
        }

        //Create an OpenGL Graphics Context
        gContext = SDL_GL_CreateContext( gWindow );
        if( gContext == NULL){
            errorStream << "OpenGL context could not be created! SDL Error: " << SDL_GetError() << "\n";
            success = false;
        }

        // Initialize GLAD Library
        if(!gladLoadGLLoader(SDL_GL_GetProcAddress)){
            errorStream << "Failed to iniitalize GLAD\n";
            success = false;
        }

        //Initialize OpenGL
        if(!initGL()){
            errorStream << "Unable to initialize OpenGL!\n";
            success = false;
        }

    }
    

    // If initialization did not work, then print out a list of errors in the constructor.
    if(!success){
        errorStream << "Failed to initialize!\n";
        std::string errors=errorStream.str();
        std::cout << errors << "\n";
    }else{
        std::cout << "No SDL, GLAD, or OpenGL, errors Detected\n\n";
    }

}

// Proper shutdown and destroy initialized objects
SDLGraphicsProgram::~SDLGraphicsProgram(){
    //Destroy window
    SDL_DestroyWindow( gWindow );
    // Point gWindow to NULL to ensure it points to nothing.
    gWindow = NULL;
    // Deallocate Program
    glDeleteProgram(shader);
    //Quit SDL subsystems
    SDL_Quit();
}


void SDLGraphicsProgram::GenerateBuffers(){
    // Rendering primitives from shader, don't do anything
    // Actually we need some junk to make it actually call the shaders
    GLuint id = 0;
    glGenVertexArrays(1, &id);
    glBindVertexArray(id);
    GLuint buf_id;
    glGenBuffers(1, &buf_id);
    glBindBuffer(GL_ARRAY_BUFFER, buf_id);
    int data[] = {0,0,0,0};
    glBufferData(GL_ARRAY_BUFFER, 4*sizeof(int), &data, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 1, GL_INT, GL_FALSE, 0, 0);


}

// Initialize OpenGL
// Setup any of our shaders here.
bool SDLGraphicsProgram::initGL(){
    //Success flag
    bool success = true;

    // Setup geometry
    GenerateBuffers();

    // Setup shaders
    std::string vertexShader = LoadShader("./shaders/vert.glsl");
    std::string fragmentShader = LoadShader("./shaders/frag.glsl");

    shader = CreateShader(vertexShader,fragmentShader);

    // Use our shader
    glUseProgram(shader);

    return success;
}


// Update OpenGL
void SDLGraphicsProgram::update()
{
	tick += 1;
    GLint timeLocation =  glGetUniformLocation(shaderID, "iTime");
	glUniform1i(timeLocation, tick);
    // glUniformMatrix4v means a 4x4 matrix of floats
    //glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1, GL_FALSE,&modelTransformMatrix[0][0]);
    //glUniformMatrix4fv(projectionMatrixUniformLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
}

// Render
// The render function gets called once per loop
void SDLGraphicsProgram::render(){
    // Initialize clear color
    // This is the background of the screen.
    glViewport(0, 0, screenWidth, screenHeight);
    // glClearColor( 1.0f, 1.0f, 1.0f, 1.f );

    //Clear color buffer and Depth Buffer
    // glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    //Render data
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 5);
}


//Loops forever!
void SDLGraphicsProgram::loop(){
    // Main loop flag
    // If this is quit = 'true' then the program terminates.
    bool quit = false;
    // Event handler that handles various events in SDL
    // that are related to input and output
    SDL_Event e;
    // Enable text input
    SDL_StartTextInput();
    // While application is running
    while(!quit){
        //Handle events on queue
        while(SDL_PollEvent( &e ) != 0){
            // User posts an event to quit
            // An example is hitting the "x" in the corner of the window.
            if (e.type == SDL_QUIT){
                quit = true;
            }
            if (e.type == SDL_KEYDOWN) {

                switch (e.key.keysym.sym) {
                    case SDLK_q:
                        exit(0);
                        break;
                    default:
                        break;
                }
            }

        }
        // Update our scene
        update();
        // Render using OpenGL
        render(); // TODO: potentially move this depending on your logic
        // for how you handle drawing a triangle or rectangle.
        //Update screen of our specified window
        SDL_GL_SwapWindow(getSDLWindow());
    }

    //Disable text input
    SDL_StopTextInput();
}


// Loads a shader and returns a string
std::string SDLGraphicsProgram::LoadShader(const std::string& fname){
    std::string result;
    // 1.) Get every line of data
    std::string line;
    std::ifstream myFile(fname.c_str());

    if(myFile.is_open()){
        while(getline(myFile,line)){
            result += line + '\n';
        }
    }
    // Close file
    myFile.close();
    return result;
}


unsigned int SDLGraphicsProgram::CreateShader(const std::string& vertexShaderSource, const std::string& fragmentShaderSource){

    // Create a new program
    unsigned int program = glCreateProgram();
    // Compile our shaders
    unsigned int myVertexShader = CompileShader(GL_VERTEX_SHADER, vertexShaderSource);
    unsigned int myFragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    // Link our program
    // These have been compiled already.
    glAttachShader(program,myVertexShader);
    glAttachShader(program,myFragmentShader);
    // Link our programs that have been 'attached'
    glLinkProgram(program);
    glValidateProgram(program);

    // Once the shaders have been linked in, we can delete them.
    glDetachShader(program,myVertexShader);
    glDetachShader(program,myFragmentShader);

    glDeleteShader(myVertexShader);
    glDeleteShader(myFragmentShader);

    if(!CheckLinkStatus(program)){
        std::cout << "ERROR, shader did not link!\n";
    }

    shaderID = program;
    return program;
}


unsigned int SDLGraphicsProgram::CompileShader(unsigned int type, const std::string& source){
    // Compile our shaders
    // id is the type of shader (Vertex, fragment, etc.)
    unsigned int id;

    if(type == GL_VERTEX_SHADER){
        id = glCreateShader(GL_VERTEX_SHADER);
    }else if(type == GL_FRAGMENT_SHADER){
        id = glCreateShader(GL_FRAGMENT_SHADER);
    }
    const char* src = source.c_str();
    // The source of our shader
    glShaderSource(id, 1, &src, nullptr);
    // Now compile our shader
    glCompileShader(id);

    // Retrieve the result of our compilation
    int result;
    // This code is returning any compilation errors that may have occurred!
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if(result == GL_FALSE){
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* errorMessages = new char[length]; // Could also use alloca here.
        glGetShaderInfoLog(id, length, &length, errorMessages);
        if(type == GL_VERTEX_SHADER){
            std::cout << "ERROR: GL_VERTEX_SHADER compilation failed!\n" << errorMessages << "\n";
        } else if(type == GL_FRAGMENT_SHADER){
            std::cout << "ERROR: GL_FRAGMENT_SHADER compilation failed!\n" << errorMessages << "\n";
        }
        // Reclaim our memory
        delete[] errorMessages;
        // Delete our broken shader
        glDeleteShader(id);
        return 0;
    }

    return id;
}

// Check to see if linking was successful
bool SDLGraphicsProgram::CheckLinkStatus(GLuint programID){
    // Retrieve the result of our compilation
    int result = 0;
    // This code is returning any Linker errors that may have occurred!
    glGetProgramiv(programID, GL_LINK_STATUS, &result);
    if(result == GL_FALSE){
        int length;
        glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &length);
        char* errorMessages = new char[length]; // Could also use alloca here.
        glGetProgramInfoLog(programID, length, &length, errorMessages);
        // Reclaim our memory
        std::cout << "ERROR in linking process " << errorMessages << "\n";
        delete[] errorMessages;
        return false;
    }
    return true;
}


// Get Pointer to Window
SDL_Window* SDLGraphicsProgram::getSDLWindow(){
    return gWindow;
}

// Helper Function to get OpenGL Version Information
void SDLGraphicsProgram::getOpenGLVersionInfo(){
    std::cout << "Vendor: " << glGetString(GL_VENDOR) << "\n";
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << "\n";
    std::cout << "Version: " << glGetString(GL_VERSION) << "\n";
    std::cout << "Shading language: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n";
}
