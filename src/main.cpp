#ifdef WIN32
    #define SDL_MAIN_HANDLED
#endif
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>

#include <cstdlib>
#include <iostream>
#include "NGLDraw.h"
#include <ngl/NGLInit.h>

/// @brief function to quit SDL with error message
/// @param[in] _msg the error message to send
void SDLErrorExit(const std::string &_msg);

/// @brief initialize SDL OpenGL context
SDL_GLContext createOpenGLContext( SDL_Window *window);



int main(int argc, char * argv[])
{
  // under windows we must use main with argc / v so jus flag unused for params
  NGL_UNUSED(argc);
  NGL_UNUSED(argv);
  // Initialize SDL's Video subsystem
  if (!SDL_Init(SDL_INIT_VIDEO)  )
  {
    // Or die on error
    SDLErrorExit("Unable to initialize SDL");
  }

  // now get the size of the display and create a window we need to init the video
  SDL_Rect rect;
  SDL_GetDisplayBounds(0,&rect);
  // now create our window
  SDL_Window *window=SDL_CreateWindow("SDLNGL",
                                      rect.w/2,
                                      rect.h/2,
                                      SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY
                                     );
  // check to see if that worked or exit
  if (!window)
  {
    SDLErrorExit("Unable to create window");
  }

  // Create our opengl context and attach it to our window

   SDL_GLContext glContext=createOpenGLContext(window);
   if(!glContext)
   {
     SDLErrorExit("Problem creating OpenGL context ");
   }
   // make this our current GL context (we can have more than one window but in this case not)
   SDL_GL_MakeCurrent(window, glContext);
  /* This makes our buffer swap syncronized with the monitor's vertical refresh */
  SDL_GL_SetSwapInterval(1);
  // we need to initialise the NGL lib which will load all of the OpenGL functions, this must
  // be done once we have a valid GL context but before we call any GL commands. If we dont do
  // this everything will crash
  ngl::NGLInit::initialize();
  // now clear the screen and swap whilst NGL inits (which may take time)
  glClear(GL_COLOR_BUFFER_BIT);
  SDL_GL_SwapWindow(window);
  // flag to indicate if we need to exit
  bool quit=false;
  // sdl event processing data structure
  SDL_Event event;
  // now we create an instance of our ngl class, this will init NGL and setup basic
  // opengl stuff ext. When this falls out of scope the dtor will be called and cleanup
  // our gl stuff
  NGLDraw ngl;
  while(!quit)
  {

    while ( SDL_PollEvent(&event) )
    {
      switch (event.type)
      {
        // this is the window x being clicked.
        case SDL_EVENT_QUIT : quit = true; break;
        // process the mouse data by passing it to ngl class
        case SDL_EVENT_MOUSE_MOTION : ngl.mouseMoveEvent(event.motion); break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN : ngl.mousePressEvent(event.button); break;
        case SDL_EVENT_MOUSE_BUTTON_UP : ngl.mouseReleaseEvent(event.button); break;
        case SDL_EVENT_MOUSE_WHEEL : ngl.wheelEvent(event.wheel); break;
        // if the window is re-sized pass it to the ngl class to change gl viewport
        // note this is slow as the context is re-create by SDL each time
        case SDL_EVENT_WINDOW_RESIZED :
          int w,h;
          // get the new window size
          SDL_GetWindowSize(window,&w,&h);
          #ifdef __APPLE__
            ngl.resize(w*2,h*2);
          #else
            ngl.resize(w,h);
          #endif
        break;

        // now we look for a keydown event
        case SDL_EVENT_KEY_DOWN:
        {
          switch( event.key.key)
          {
            // if it's the escape key quit
            case SDLK_ESCAPE :  quit = true; break;
            case SDLK_W : glPolygonMode(GL_FRONT_AND_BACK,GL_LINE); break;
            case SDLK_S : glPolygonMode(GL_FRONT_AND_BACK,GL_FILL); break;
            case SDLK_F :
            SDL_SetWindowFullscreen(window,true);
            glViewport(0,0,rect.w*2,rect.h*2);
            break;

            case SDLK_G : SDL_SetWindowFullscreen(window,false); break;
            default : break;
          } // end of key process
        } // end of keydown

        break;
      default : break;
      } // end of event switch
    } // end of poll events

    // now we draw ngl
    ngl.draw();
    // swap the buffers
    SDL_GL_SwapWindow(window);

  }
  // now tidy up and exit SDL
 SDL_Quit();
 // whilst this code will never execute under windows we need to have a return from
 // SDL_Main!
 return EXIT_SUCCESS;
}


SDL_GLContext createOpenGLContext(SDL_Window *window)
{
  // Note you may have to change this depending upon the driver (Windows is fussy)
  // stick to 4.5 as the core base level for NGL works ok
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

   // set multi sampling else we get really bad graphics that alias
   SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
   SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES,4);
   // Turn on double buffering with a 24bit Z buffer.
   // You may need to change this to 16 or 32 for your system
   // on mac up to 32 will work but under linux centos build only 16
   SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
   // enable double buffering (should be on by default)
   SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  return SDL_GL_CreateContext(window);

}

void SDLErrorExit(const std::string &_msg)
{
  std::cerr<<_msg<<"\n";
  std::cerr<<SDL_GetError()<<"\n";
  SDL_Quit();
  exit(EXIT_FAILURE);
}
