#include "NGLScene.h"
#include <QMouseEvent>
#include <QGuiApplication>

#include <ngl/Camera.h>
#include <ngl/Light.h>
#include <ngl/Material.h>
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/ShaderLib.h>
#include <ngl/NGLStream.h>
#include <ngl/Transformation.h>
#include <QtImGui.h>
#include <imgui.h>

extern bool ColorSelector(const char* pLabel, ngl::Vec4& oRGBA);

//----------------------------------------------------------------------------------------------------------------------
/// @brief the increment for x/y translation with mouse movement
//----------------------------------------------------------------------------------------------------------------------
const static float INCREMENT=0.01f;
//----------------------------------------------------------------------------------------------------------------------
/// @brief the increment for the wheel zoom
//----------------------------------------------------------------------------------------------------------------------
const static float ZOOM=0.1f;

NGLScene::NGLScene()
{
  // re-size the widget to that of the parent (in that case the GLFrame passed in on construction)
  m_rotate=false;
  // mouse rotation values set to 0
  m_spinXFace=0.0f;
  m_spinYFace=0.0f;
  setTitle("Qt5 Simple NGL Demo");
}


NGLScene::~NGLScene()
{
  std::cout<<"Shutting down NGL, removing VAO's and Shaders\n";
}



void NGLScene::resizeGL(int _w , int _h)
{
  m_cam.setShape(45.0f,static_cast<float>(_w)/_h,0.05f,350.0f);
  m_width=static_cast<int>(_w*devicePixelRatio());
  m_height=static_cast<int>(_h*devicePixelRatio());
}

void NGLScene::setLight(const ngl::Vec4 &_position,const ngl::Vec4 &_ambient,const ngl::Vec4 &_specular,const ngl::Vec4 &_diffuse )
{
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  shader->use("Phong");
  shader->setUniform("light.position",_position);
  shader->setUniform("light.ambient",_ambient);
  shader->setUniform("light.specular",_specular);
  shader->setUniform("light.diffuse",_diffuse);

}

void NGLScene::setMaterial(const ngl::Vec4 &_ambient,const ngl::Vec4 &_specular,const ngl::Vec4 &_diffuse, float _specPower )
{
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  shader->use("Phong");
  shader->setUniform("material.ambient",_ambient);
  shader->setUniform("material.specular",_specular);
  shader->setUniform("material.diffuse",_diffuse);
  shader->setUniform("material.shininess",_specPower);
}

void NGLScene::initializeGL()
{
  QtImGui::initialize(this);

  // we must call that first before any other GL commands to load and link the
  // gl commands from the lib, if that is not done program will crash
  ngl::NGLInit::instance();
  glClearColor(0.4f, 0.4f, 0.4f, 1.0f);			   // Grey Background
  // enable depth testing for drawing
  glEnable(GL_DEPTH_TEST);
  // enable multisampling for smoother drawing
#ifndef USINGIOS_
  glEnable(GL_MULTISAMPLE);
#endif
   // now to load the shader and set the values
  // grab an instance of shader manager
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  // we are creating a shader called Phong to save typos
  // in the code create some constexpr
  constexpr auto shaderProgram="Phong";
  constexpr auto vertexShader="PhongVertex";
  constexpr auto fragShader="PhongFragment";
  // create the shader program
  shader->createShaderProgram(shaderProgram);
  // now we are going to create empty shaders for Frag and Vert
  shader->attachShader(vertexShader,ngl::ShaderType::VERTEX);
  shader->attachShader(fragShader,ngl::ShaderType::FRAGMENT);
  // attach the source
  shader->loadShaderSource(vertexShader,"shaders/PhongVertex.glsl");
  shader->loadShaderSource(fragShader,"shaders/PhongFragment.glsl");
  // compile the shaders
  shader->compileShader(vertexShader);
  shader->compileShader(fragShader);
  // add them to the program
  shader->attachShaderToProgram(shaderProgram,vertexShader);
  shader->attachShaderToProgram(shaderProgram,fragShader);


  // now we have associated that data we can link the shader
  shader->linkProgramObject(shaderProgram);
  // and make it active ready to load values
  (*shader)[shaderProgram]->use();
  // the shader will use the currently active material and light0 so set them
  ngl::Material m(ngl::STDMAT::GOLD);
  // load our material values to the shader into the structure material (see Vertex shader)
  m.loadToShader("material");
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
  // now create our light that is done after the camera so we can pass the
  // transpose of the projection matrix to the light to do correct eye space
  // transformations
  ngl::Mat4 iv=m_cam.getViewMatrix();
  iv.transpose();
  ngl::Light light(ngl::Vec3(-2,5,2),ngl::Colour(1,1,1,1),ngl::Colour(1,1,1,1),ngl::LightModes::POINTLIGHT );
  light.setTransform(iv);
  // load these values to the shader as well
  light.loadToShader("light");


}

void NGLScene::drawIMGUI()
{


  QtImGui::newFrame();
  if(showModelControls)
  {

      ImGui::Begin("Model");
      ImGui::SliderFloat3("rotation",m_modelRot.openGL(),-180.0f,180.f);
      ImGui::SliderFloat3("position",m_modelPosition.openGL(),-10.0f,10.f);
      ImGui::SliderFloat3("scale",m_modelScale.openGL(),-2.0f,2.f);

      //ImGui::ColorEdit3("clear color", clearColour.openGL());
      ColorSelector("clear color",m_clearColour);
      const char* items[]={ "Teapot", "Troll", "Bunny", "Dragon", "Buddah", "Cube" };
      ImGui::Combo("Model", &m_modelID, items,6);   // Combo using proper array. You can also pass a callback to retrieve array value, no need to create/copy an array just for that.

      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
      glClearColor(m_clearColour.m_r,m_clearColour.m_g,m_clearColour.m_b,m_clearColour.m_a);
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
      setLight(position,ambient,specular,diffuse);
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
      setMaterial(ambient,specular,diffuse,specPower);

      ImGui::End();

  }
  ImGui::Render();

}



void NGLScene::loadMatricesToShader()
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

void NGLScene::paintGL()
{
  glViewport(0,0,m_width,m_height);
  // clear the screen and depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // grab an instance of the shader manager
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  (*shader)["Phong"]->use();

  // Rotation based on the mouse position for our global transform
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
  switch(m_modelID)
     {
      case 0 : m_localScale.scale(1.0f, 1.0f, 1.0f); prim->draw("teapot"); break;
      case 1 : m_localScale.scale(1.0f, 1.0f, 1.0f); prim->draw("troll"); break;
      case 2 : m_localScale.scale(0.1f, 0.1f, 0.1f); prim->draw("bunny"); break;
      case 3 : m_localScale.scale(0.1f, 0.1f, 0.1f); prim->draw("dragon"); break;
      case 4 : m_localScale.scale(0.1f, 0.1f, 0.1f); prim->draw("buddah"); break;
      case 5 : m_localScale.scale(1.0f, 1.0f, 1.0f); prim->draw("cube"); break;

    }
  drawIMGUI();

}


void NGLScene::setMouseState(QMouseEvent *_event)
{
  for(auto &b : m_mouseButtons)
    b=false;
  if(_event->buttons() == Qt::LeftButton)
    m_mouseButtons[0]=true;
  if(_event->buttons() == Qt::RightButton)
    m_mouseButtons[1]=true;
  if(_event->buttons() == Qt::MiddleButton)
    m_mouseButtons[2]=true;

}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mouseMoveEvent (QMouseEvent * _event)
{
//  setMouseState(_event);
//  // note the method buttons() is the button state when event was called
//  // that is different from button() which is used to check which button was
//  // pressed when the mousePress/Release event is generated
//  if(m_rotate && _event->buttons() == Qt::LeftButton)
//  {
//    int diffx=_event->x()-m_origX;
//    int diffy=_event->y()-m_origY;
//    m_spinXFace += static_cast<int>( 0.5f * diffy);
//    m_spinYFace += static_cast<int>( 0.5f * diffx);
//    m_origX = _event->x();
//    m_origY = _event->y();
//    update();

//  }
//        // right mouse translate code
//  else if(m_translate && _event->buttons() == Qt::RightButton)
//  {
//    int diffX = static_cast<int>(_event->x() - m_origXPos);
//    int diffY = static_cast<int>(_event->y() - m_origYPos);
//    m_origXPos=_event->x();
//    m_origYPos=_event->y();
//    m_modelPos.m_x += INCREMENT * diffX;
//    m_modelPos.m_y -= INCREMENT * diffY;
//    update();

//   }
  update();
}


//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mousePressEvent ( QMouseEvent * _event)
{
//  setMouseState(_event);
//  // that method is called when the mouse button is pressed in this case we
//  // store the value where the maouse was clicked (x,y) and set the Rotate flag to true
//  if(_event->button() == Qt::LeftButton)
//  {
//    m_origX = _event->x();
//    m_origY = _event->y();
//    m_rotate =true;
//  }
//  // right mouse translate mode
//  else if(_event->button() == Qt::RightButton)
//  {
//    m_origXPos = _event->x();
//    m_origYPos = _event->y();
//    m_translate=true;
//  }
update();
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mouseReleaseEvent ( QMouseEvent * _event )
{
//  setMouseState(_event);
//  // that event is called when the mouse button is released
//  // we then set Rotate to false
//  if (_event->button() == Qt::LeftButton)
//  {
//    m_rotate=false;
//  }
//        // right mouse translate mode
//  if (_event->button() == Qt::RightButton)
//  {
//    m_translate=false;
//  }
update();
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::wheelEvent(QWheelEvent *_event)
{

//	// check the diff of the wheel position (0 means no change)
//	if(_event->delta() > 0)
//	{
//		m_modelPos.m_z+=ZOOM;
//	}
//	else if(_event->delta() <0 )
//	{
//		m_modelPos.m_z-=ZOOM;
//	}
  update();
}
//----------------------------------------------------------------------------------------------------------------------

void NGLScene::keyPressEvent(QKeyEvent *_event)
{
  // that method is called every time the main window recives a key event.
  // we then switch on the key value and set the camera in the GLWindow
  switch (_event->key())
  {
  // escape key to quit
  case Qt::Key_Escape : QGuiApplication::exit(EXIT_SUCCESS); break;
  // turn on wirframe rendering
#ifndef USINGIOS_

  case Qt::Key_W : glPolygonMode(GL_FRONT_AND_BACK,GL_LINE); break;
  // turn off wire frame
  case Qt::Key_S : glPolygonMode(GL_FRONT_AND_BACK,GL_FILL); break;
#endif
  // show full screen
  case Qt::Key_F : showFullScreen(); break;
  // show windowed
  case Qt::Key_N : showNormal(); break;
  default : break;
  }
 update();
}
