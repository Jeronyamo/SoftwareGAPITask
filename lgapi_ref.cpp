#include "lgapi.h"

#include <memory>
#include <stdio.h>
#include <stdlib.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include "GL/glut.h"

void ExitProgram();

struct BatchRenderGL : public IRender
{
  int glut_window_id;
  BatchRenderGL(int win_id) : glut_window_id{win_id} {}
  ~BatchRenderGL() override { ExitProgram(); }
  
  unsigned int AddImage(Image2D a_img) override;

  void BeginRenderPass(Image2D fb) override;
  void Draw(PipelineStateObject a_state, Geom a_geom) override;
  void EndRenderPass(Image2D fb) override;
};



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ExitProgram()
{
  exit(0);
}

extern uint32_t WIN_WIDTH ;
extern uint32_t WIN_HEIGHT;

std::shared_ptr<IRender> MakeReferenceImpl() 
{
  int argc = 1;
  char **argv = nullptr;

  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
  glutInitWindowSize(WIN_WIDTH, WIN_HEIGHT);
  int glut_window_id = glutCreateWindow("OpenGL Animation");

  int  width = glutGet(GLUT_WINDOW_WIDTH);
  int height = glutGet(GLUT_WINDOW_HEIGHT);

  glClearColor(0.00, 0.00, 0.40, 1.00);
  glViewport(0, 0, width, height);

  return std::make_shared<BatchRenderGL>(glut_window_id); 
}

static void transposeMatrix(const float in_matrix[16], float out_matrix[16])
{
  for(int i=0;i<4;i++)
    for(int j=0;j<4;j++)
      out_matrix[i*4+j] = in_matrix[j*4+i];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BatchRenderGL::BeginRenderPass(Image2D fb)
{
  glViewport(0, 0, fb.width, fb.height);
  glClearColor(0,0,0,0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

  // usually we don't have to actually clear framebuffer if we are not going to immediately use it 
  // In this implementation we don't, because OpenGL render inside it's owk framebuffer. 
  // So, we should not do anything with 'fb'
  // In you software implementation you will likely to clear fb here unless you don't plan to use some specific internal framebuffer format and copy final image to fb at the end 

  // so, you could save input fb and later just directly draw to it! 

  glDisable(GL_CULL_FACE);
  glDisable(GL_LIGHTING);
  glEnable (GL_DEPTH_TEST);
}

unsigned int BatchRenderGL::AddImage(Image2D a_img)
{
  GLuint texture = (GLuint)(-1);
  glGenTextures(1, &texture);					// Create The Texture

  glBindTexture(GL_TEXTURE_2D, texture);
  glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, a_img.width, a_img.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, a_img.data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  return (unsigned int)texture;
}

void BatchRenderGL::Draw(PipelineStateObject a_state, Geom a_geom)
{
  float tempMat[16];  
  glMatrixMode(GL_PROJECTION);
  transposeMatrix(a_state.projMatrix, tempMat);      // GL assume col-major matrices (which is usually better), while in our API thet are row-major
  glLoadMatrixf(tempMat);

  glMatrixMode(GL_MODELVIEW);
  transposeMatrix(a_state.worldViewMatrix, tempMat); // GL assume col-major matrices (which is usually better), while in our API thet are row-major
  glLoadMatrixf(tempMat);
  
  if(a_state.mode == MODE_TEXURE_3D)
  {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, a_state.imgId);
  }
  else
  {
    glDisable(GL_TEXTURE_2D);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  }

  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);
  glEnableClientState(GL_VERTEX_ARRAY);

  glColorPointer (4, GL_FLOAT, 0,  a_geom.vcol4f);
  glVertexPointer(4, GL_FLOAT, 0,  a_geom.vpos4f);  
  glTexCoordPointer(2, GL_FLOAT, 0, a_geom.vtex2f);

  glDrawElements (GL_TRIANGLES, a_geom.primsNum*3, GL_UNSIGNED_INT, a_geom.indices);
}

void BatchRenderGL::EndRenderPass(Image2D fb)
{
  glFlush();
  glReadPixels(0, 0, fb.width, fb.height, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)fb.data);
}