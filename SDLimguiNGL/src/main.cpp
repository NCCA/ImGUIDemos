#include <SDL.h>
#include <cstdlib>
#include <iostream>
#include "NGLDraw.h"
#include <ngl/NGLInit.h>
#include <array>
#include <imgui.h>
#include "imgui_impl_sdl_gl3.h"
extern bool ColorSelector(const char* pLabel, ngl::Vec4& oRGBA);

/// @brief function to quit SDL with error message
/// @param[in] _msg the error message to send
void SDLErrorExit(const std::string &_msg);

/// @brief initialize SDL OpenGL context
SDL_GLContext createOpenGLContext( SDL_Window *window);



int main()
{

  // Initialize SDL's Video subsystem
  if (SDL_Init(SDL_INIT_VIDEO) < 0 )
  {
    // Or die on error
    SDLErrorExit("Unable to initialize SDL");
  }

  // now get the size of the display and create a window we need to init the video
  SDL_Rect rect;
  SDL_GetDisplayBounds(0,&rect);
  // now create our window
  SDL_Window *window=SDL_CreateWindow("SDL NGL and imgui",
                                      SDL_WINDOWPOS_CENTERED,
                                      SDL_WINDOWPOS_CENTERED,
                                      rect.w/2,
                                      rect.h/2,
                                      SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
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
     SDLErrorExit("Problem creating OpenGL context");
   }
   // make this our current GL context (we can have more than one window but in this case not)
   SDL_GL_MakeCurrent(window, glContext);
  /* This makes our buffer swap syncronized with the monitor's vertical refresh */
  SDL_GL_SetSwapInterval(1);
  // we need to initialise the NGL lib which will load all of the OpenGL functions, this must
  // be done once we have a valid GL context but before we call any GL commands. If we dont do
  // this everything will crash
  ngl::NGLInit::instance();

  // Setup ImGui binding
  ImGui_ImplSdlGL3_Init(window);


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
  // resize the ngl to set the screen size and camera stuff
  ngl.resize(rect.w,rect.h);
  ImGuiIO& io = ImGui::GetIO();

  bool showModelControls=true;
  bool showLightControls=true;
  bool showMaterialControls=true;

  while(!quit)
  {

    while ( SDL_PollEvent(&event) )
    {
      ImGui_ImplSdlGL3_ProcessEvent(&event);
      ImGui_ImplSdlGL3_NewFrame(window);
      if(showModelControls)
      {
          static ngl::Vec3 rot(0,0,0);
          static ngl::Vec3 pos(0,0,0);
          static ngl::Vec3 scale(1,1,1);
          static ngl::Vec4 clearColour= {0.5,0.5,0.5,1.0};

          ImGui::Begin("Model");
          ImGui::SliderFloat3("rotation",rot.openGL(),-180.0f,180.f);
          ImGui::SliderFloat3("position",pos.openGL(),-10.0f,10.f);
          ImGui::SliderFloat3("scale",scale.openGL(),-2.0f,2.f);

          //ImGui::ColorEdit3("clear color", clearColour.openGL());
          ColorSelector("clear color",clearColour);
          const char* items[]={ "Teapot", "Troll", "Bunny", "Dragon", "Buddah", "Cube" };
          static int modelID = 0;
          ImGui::Combo("Model", &modelID, items,6);   // Combo using proper array. You can also pass a callback to retrieve array value, no need to create/copy an array just for that.

          ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
          glClearColor(clearColour.m_r,clearColour.m_g,clearColour.m_b,clearColour.m_a);
          ngl.setModelRotation(rot);
          ngl.setModelPosition(pos);
          ngl.setModelScale(scale);
          ngl.setModelID(modelID);

          ImGui::End();

      }
      if(showLightControls)
      {
          static ngl::Vec4 position={-2.0f,5.0f,2.0f};
          static ngl::Vec4 ambient={0.0f,0.0f,0.0f};
          static ngl::Vec4 specular={1.0f,1.0f,1.0f};
          static ngl::Vec4 diffuse={1.0f,1.0f,1.0f};
          ImGui::Begin("Light");
          ImGui::SliderFloat3("position",position.openGL(),-10,10);
          ImGui::ColorEdit3("Ambient", ambient.openGL());
          ImGui::ColorEdit3("Specular", specular.openGL());
          ImGui::ColorEdit3("Diffuse", diffuse.openGL());
          ngl.setLight(position,ambient,specular,diffuse);
          ImGui::End();

      }
      if(showMaterialControls)
      {
          static ngl::Vec4 ambient={0.274725f,0.1995f,0.0745f};
          static ngl::Vec4 specular={0.628281f, 0.555802f,0.3666065f};
          static ngl::Vec4 diffuse={0.75164f,0.60648f,0.22648f};
          static float specPower=51.2f;
          ImGui::Begin("Material");
          ImGui::ColorEdit3("Ambient", ambient.openGL());
          ImGui::ColorEdit3("Specular", specular.openGL());
          ImGui::ColorEdit3("Diffuse", diffuse.openGL());
          ImGui::SliderFloat("Cos Power", &specPower,0.0f,200.0f);

          ngl.setMaterial(ambient,specular,diffuse,specPower);
          ImGui::End();

      }


      if(!io.WantCaptureMouse)
      {
      switch (event.type)
      {
        // this is the window x being clicked.
        case SDL_QUIT : quit = true; break;
        // process the mouse data by passing it to ngl class
        case SDL_MOUSEMOTION : ngl.mouseMoveEvent(event.motion); break;
        case SDL_MOUSEBUTTONDOWN : ngl.mousePressEvent(event.button); break;
        case SDL_MOUSEBUTTONUP : ngl.mouseReleaseEvent(event.button); break;
        case SDL_MOUSEWHEEL : ngl.wheelEvent(event.wheel); break;
        // if the window is re-sized pass it to the ngl class to change gl viewport
        // note this is slow as the context is re-create by SDL each time
        case SDL_WINDOWEVENT :
          int w,h;
          // get the new window size
          SDL_GetWindowSize(window,&w,&h);
          ngl.resize(w,h);
        break;

        // now we look for a keydown event
        case SDL_KEYDOWN:
        {
          switch( event.key.keysym.sym )
          {
            // if it's the escape key quit
            case SDLK_ESCAPE :  quit = true; break;
            case SDLK_w : glPolygonMode(GL_FRONT_AND_BACK,GL_LINE); break;
            case SDLK_s : glPolygonMode(GL_FRONT_AND_BACK,GL_FILL); break;
            case SDLK_f :
            SDL_SetWindowFullscreen(window,SDL_TRUE);
            glViewport(0,0,rect.w,rect.h);
            case SDLK_m : showModelControls^=true; break;
            break;

            case SDLK_g : SDL_SetWindowFullscreen(window,SDL_FALSE); break;
            default : break;
          } // end of key process
        } // end of keydown

        default : break;
      } // end of event switch
      } // end of not want from IO
    } // end of poll events

    // now we draw ngl
    ngl.draw();
    ImGui::Render();

    // swap the buffers
    SDL_GL_SwapWindow(window);

  }
  // now tidy up and exit SDL
 SDL_Quit();
}


SDL_GLContext createOpenGLContext(SDL_Window *window)
{
  // Request an opengl 3.2 context first we setup our attributes, if you need any
  // more just add them here before the call to create the context
  // SDL doesn't have the ability to choose which profile at this time of writing,
  // but it should default to the core profile
  // for some reason we need this for mac but linux crashes on the latest nvidia drivers
  // under centos
  #ifdef __APPLE__
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
  #else

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

  #endif
  // set multi sampling else we get really bad graphics that alias
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES,4);
  // Turn on double buffering with a 24bit Z buffer.
  // You may need to change this to 16 or 32 for your system
  // on mac up to 32 will work but under linux centos build only 16
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
  // enable double buffering (should be on by default)
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  //
  return SDL_GL_CreateContext(window);

}

void SDLErrorExit(const std::string &_msg)
{
  std::cerr<<_msg<<"\n";
  std::cerr<<SDL_GetError()<<"\n";
  SDL_Quit();
  exit(EXIT_FAILURE);
}
