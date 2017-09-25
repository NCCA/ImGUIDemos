#include "NGLDraw.h"
#include <ngl/ShaderLib.h>
#include <ngl/NGLInit.h>
#include <ngl/Material.h>
#include <ngl/Transformation.h>


const static float INCREMENT=0.01;
const static float ZOOM=0.05;
NGLDraw::NGLDraw()
{
  m_rotate=false;
  // mouse rotation values set to 0
  m_spinXFace=0;
  m_spinYFace=0;

  glClearColor(0.4f, 0.4f, 0.4f, 1.0f);			   // Grey Background
  // enable depth testing for drawing
  glEnable(GL_DEPTH_TEST);
   // now to load the shader and set the values
  // grab an instance of shader manager
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  // we are creating a shader called Phong here we create some
  // aliases as constexpr to avoid typos later
  constexpr auto ShaderName="Phong";
  constexpr auto VertexName="PhongVertex";
  constexpr auto FragmentName="PhongFragment";

  shader->createShaderProgram(ShaderName);
  // now we are going to create empty shaders for Frag and Vert
  shader->attachShader(VertexName,ngl::ShaderType::VERTEX);
  shader->attachShader(FragmentName,ngl::ShaderType::FRAGMENT);
  // attach the source
  shader->loadShaderSource(VertexName,"shaders/PhongVertex.glsl");
  shader->loadShaderSource(FragmentName,"shaders/PhongFragment.glsl");
  // compile the shaders
  shader->compileShader(VertexName);
  shader->compileShader(FragmentName);
  // add them to the program
  shader->attachShaderToProgram(ShaderName,VertexName);
  shader->attachShaderToProgram(ShaderName,FragmentName);

  // now we have associated this data we can link the shader
  shader->linkProgramObject(ShaderName);
  // and make it active ready to load values
  (*shader)[ShaderName]->use();
  // Now we will create a basic Camera from the graphics library
  // This is a static camera so it only needs to be set once
  // First create Values for the camera position
  ngl::Vec3 from(0,1,1);
  ngl::Vec3 to(0,0,0);
  ngl::Vec3 up(0,1,0);
  // now load to our new camera
  m_cam.set(from,to,up);
  // set the shape using FOV 45 Aspect Ratio based on Width and Height
  // The final two are near and far clipping planes of 0.5 and 10
  m_cam.setShape(45.0f,720.0f/576.0f,0.05f,350.0f);
  shader->setUniform("viewerPos",m_cam.getEye().toVec3());
  setLight(ngl::Vec4(-2.0f,5.0f,2.0f),ngl::Vec4::zero(),ngl::Vec4(1.0f,1.0f,1.0f),ngl::Vec4(1.0f,1.0f,1.0f));

  setMaterial({0.274725f,0.1995f,0.0745f},{0.628281f, 0.555802f,0.3666065f},{0.75164f,0.60648f,0.22648f},51.2f);

}

NGLDraw::~NGLDraw()
{
  std::cout<<"Shutting down NGL, removing VAO's and Shaders\n";
}

void NGLDraw::resize(int _w, int _h)
{
  glViewport(0,0,_w,_h);
  // now set the camera size values as the screen size has changed
  m_cam.setShape(45.0f,static_cast<float>(_w)/_h,0.05f,350.0f);
}

void NGLDraw::draw()
{
  // clear the screen and depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // grab an instance of the shader manager
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  (*shader)["Phong"]->use();

  // Rotation based on the mouse position for our global transform
  ngl::Transformation trans;
  ngl::Mat4 rotX;
  ngl::Mat4 rotY;
  // create the rotation matrices
  rotX.rotateX(m_spinXFace);
  rotY.rotateY(m_spinYFace);
  // multiply the rotations
  m_mouseGlobalTX=rotY*rotX;
  // add the translations
  m_mouseGlobalTX.m_m[3][0] = m_modelPos.m_x;
  m_mouseGlobalTX.m_m[3][1] = m_modelPos.m_y;
  m_mouseGlobalTX.m_m[3][2] = m_modelPos.m_z;

   // get the VBO instance and draw the built in teapot
  ngl::VAOPrimitives *prim=ngl::VAOPrimitives::instance();
  // draw
  loadMatricesToShader();
  if(m_wireframe)
    glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
  else
    glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
  switch(m_modelID)
   {
    case 0 : m_localScale.scale(1.0f, 1.0f, 1.0f); prim->draw("teapot"); break;
    case 1 : m_localScale.scale(1.0f, 1.0f, 1.0f); prim->draw("troll"); break;
    case 2 : m_localScale.scale(0.1f, 0.1f, 0.1f); prim->draw("bunny"); break;
    case 3 : m_localScale.scale(0.1f, 0.1f, 0.1f); prim->draw("dragon"); break;
    case 4 : m_localScale.scale(0.1f, 0.1f, 0.1f); prim->draw("buddah"); break;
    case 5 : m_localScale.scale(1.0f, 1.0f, 1.0f); prim->draw("cube"); break;

  }
  glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
}

void NGLDraw::loadMatricesToShader()
{
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();

  ngl::Mat4 MV;
  ngl::Mat4 MVP;
  ngl::Mat3 normalMatrix;
  ngl::Mat4 M;
  ngl::Transformation t;
  t.setRotation(m_modelRot);
  t.setPosition(m_modelPosition);
  t.setScale(m_modelScale);
  M=m_mouseGlobalTX*t.getMatrix()*m_localScale;
  MV=  m_cam.getViewMatrix()*M;
  MVP= m_cam.getVPMatrix()*M;
  normalMatrix=MV;
  normalMatrix.inverse().transpose();
  shader->setUniform("MV",MV);
  shader->setUniform("MVP",MVP);
  shader->setUniform("normalMatrix",normalMatrix);
  shader->setUniform("M",M);
}

void NGLDraw::setLight(const ngl::Vec4 &_position,const ngl::Vec4 &_ambient,const ngl::Vec4 &_specular,const ngl::Vec4 &_diffuse )
{
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  shader->use("Phong");
  shader->setUniform("light.position",_position);
  shader->setUniform("light.ambient",_ambient);
  shader->setUniform("light.specular",_specular);
  shader->setUniform("light.diffuse",_diffuse);

}

void NGLDraw::setMaterial(const ngl::Vec4 &_ambient,const ngl::Vec4 &_specular,const ngl::Vec4 &_diffuse, float _specPower )
{
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  shader->use("Phong");
  shader->setUniform("material.ambient",_ambient);
  shader->setUniform("material.specular",_specular);
  shader->setUniform("material.diffuse",_diffuse);
  shader->setUniform("material.shininess",_specPower);
}


//----------------------------------------------------------------------------------------------------------------------
void NGLDraw::mouseMoveEvent (const SDL_MouseMotionEvent &_event)
{
  if(m_rotate && _event.state &SDL_BUTTON_LMASK)
  {
    int diffx=_event.x-m_origX;
    int diffy=_event.y-m_origY;
    m_spinXFace += (float) 0.5f * diffy;
    m_spinYFace += (float) 0.5f * diffx;
    m_origX = _event.x;
    m_origY = _event.y;

  }
  // right mouse translate code
  else if(m_translate && _event.state &SDL_BUTTON_RMASK)
  {
    int diffX = (int)(_event.x - m_origXPos);
    int diffY = (int)(_event.y - m_origYPos);
    m_origXPos=_event.x;
    m_origYPos=_event.y;
    m_modelPos.m_x += INCREMENT * diffX;
    m_modelPos.m_y -= INCREMENT * diffY;
  }
}


//----------------------------------------------------------------------------------------------------------------------
void NGLDraw::mousePressEvent (const SDL_MouseButtonEvent &_event)
{
  // this method is called when the mouse button is pressed in this case we
  // store the value where the maouse was clicked (x,y) and set the Rotate flag to true
  if(_event.button == SDL_BUTTON_LEFT)
  {
    m_origX = _event.x;
    m_origY = _event.y;
    m_rotate =true;
  }
  // right mouse translate mode
  else if(_event.button == SDL_BUTTON_RIGHT)
  {
    m_origXPos = _event.x;
    m_origYPos = _event.y;
    m_translate=true;
  }
}

//----------------------------------------------------------------------------------------------------------------------
void NGLDraw::mouseReleaseEvent (const SDL_MouseButtonEvent &_event)
{
  // this event is called when the mouse button is released
  // we then set Rotate to false
  if (_event.button == SDL_BUTTON_LEFT)
  {
    m_rotate=false;
  }
  // right mouse translate mode
  if (_event.button == SDL_BUTTON_RIGHT)
  {
    m_translate=false;
  }
}

//----------------------------------------------------------------------------------------------------------------------
void NGLDraw::wheelEvent(const SDL_MouseWheelEvent &_event)
{

  // check the diff of the wheel position (0 means no change)
  if(_event.y > 0)
  {
    m_modelPos.m_z+=ZOOM;
  }
  else if(_event.y <0 )
  {
    m_modelPos.m_z-=ZOOM;
  }

  // check the diff of the wheel position (0 means no change)
  if(_event.x > 0)
  {
    m_modelPos.m_x-=ZOOM;
  }
  else if(_event.x <0 )
  {
    m_modelPos.m_x+=ZOOM;
  }
}
//----------------------------------------------------------------------------------------------------------------------
