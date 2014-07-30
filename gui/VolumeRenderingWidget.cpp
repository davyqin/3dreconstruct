#include "VolumeRenderingWidget.h"

#include <QMouseEvent>
#include <QGLShaderProgram>
#include <QMatrix4x4>

#include "model/Image.h"

#include <boost/shared_ptr.hpp>
#include <vector>
#include <fstream>
#include <iostream>

// #define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform2.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

namespace {
  float g_stepSize = 0.005f;
  // GLfloat dOrthoSize = 1.0f;

  GLfloat vertices[24] = {
    0.0, 0.0, 0.0,
    0.0, 0.0, 1.0,
    0.0, 1.0, 0.0,
    0.0, 1.0, 1.0,
    1.0, 0.0, 0.0,
    1.0, 0.0, 1.0,
    1.0, 1.0, 0.0,
    1.0, 1.0, 1.0
  };

  // draw the six faces of the boundbox by drawwing triangles
  // draw it contra-clockwise
  // front: 1 5 7 3
  // back: 0 2 6 4
  // left£º0 1 3 2
  // right:7 5 4 6    
  // up: 2 3 7 6
  // down: 1 0 4 5
  GLuint indices[36] = {
    1,5,7,
    7,3,1,
    0,2,6,
    6,4,0,
    0,1,3,
    3,2,0,
    7,5,4,
    4,6,7,
    2,3,7,
    7,6,2,
    1,0,4,
    4,5,1
  };
}

class VolumeRenderingWidget::Pimpl {
public:
  Pimpl()
  : qtRed(QColor::fromRgb(255,0,0))
  , qtDark(QColor::fromRgb(0,0,0))
  , qtPurple(QColor::fromCmykF(0.39, 0.39, 0.0, 0.0))
  , zoomFlag(false)
  , zoomValue(1.0)
  , texId(0)
  , xRot(0.0)
  , zRot(0.0)
  , rotFlag(false)
  , dataLoaded(false)
  , vao(0)
  , tffTexObj(0)
  , bfTexObj(0)
  , frameBuffer(0)
  , winWidth(512)
  , winHeight(512)
  , texWidth(512)
  , texHeight(512) {}

  QColor qtRed;
  QColor qtDark;
  QColor qtPurple;
  bool zoomFlag;
  double zoomValue;

  QGLShaderProgram program;
  boost::shared_ptr<QGLShader> bfVertHandle;
  boost::shared_ptr<QGLShader> bfFragHandle;
  boost::shared_ptr<QGLShader> rcVertHandle;
  boost::shared_ptr<QGLShader> rcFragHandle;

  QMatrix4x4 view;
  QMatrix4x4 projection;
  GLuint bufferObjects[3];

  GLuint texId;
  int width;
  int height;
  int depth;

  float xRot;
  float zRot;
  bool rotFlag;
  bool dataLoaded;
  QPoint lastPoint;

  GLuint gbo[2];
  GLuint vao;
  GLuint tffTexObj;
  GLuint bfTexObj;
  GLuint frameBuffer;

  GLuint winWidth;
  GLuint winHeight;
  GLuint texWidth;
  GLuint texHeight;
};

//! [0]
VolumeRenderingWidget::VolumeRenderingWidget(QWidget *parent)
  : QGLWidget(QGLFormat(QGL::DoubleBuffer), parent)
  , _pimpl(new Pimpl()) {}
//! [0]

//! [1]
VolumeRenderingWidget::~VolumeRenderingWidget() {}
//! [1]

//! [2]
QSize VolumeRenderingWidget::minimumSizeHint() const
{
    return QSize(100, 100);
}
//! [2]

//! [3]
QSize VolumeRenderingWidget::sizeHint() const
//! [3] //! [4]
{
    return QSize(512, 512);
}
//! [4]


//! [6]
void VolumeRenderingWidget::initializeGL()
{
  glewExperimental = GL_TRUE;
  glewInit();
  // initializeGLFunctions();
  qglClearColor(_pimpl->qtPurple);

  initVBO();
  initShaders();
  _pimpl->tffTexObj = initTFF1DTex("tff.dat");
  _pimpl->bfTexObj = initFace2DTex(_pimpl->texWidth, _pimpl->texHeight);
  // GL_ERROR();
  initFrameBuffer(_pimpl->bfTexObj, _pimpl->texWidth, _pimpl->texHeight);
  // GL_ERROR();
}
//! [6]

#define MAP_3DTEXT( TexIndex ) \
        glTexCoord3f(0.0f, 0.0f, ((float)TexIndex+1.0f)/2.0f);  \
        glVertex3f(-dOrthoSize,-dOrthoSize,TexIndex);\
        glTexCoord3f(1.0f, 0.0f, ((float)TexIndex+1.0f)/2.0f);  \
        glVertex3f(dOrthoSize,-dOrthoSize,TexIndex);\
        glTexCoord3f(1.0f, 1.0f, ((float)TexIndex+1.0f)/2.0f);  \
        glVertex3f(dOrthoSize,dOrthoSize,TexIndex);\
        glTexCoord3f(0.0f, 1.0f, ((float)TexIndex+1.0f)/2.0f);  \
        glVertex3f(-dOrthoSize,dOrthoSize,TexIndex);

//! [7]
void VolumeRenderingWidget::paintGL() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  if (!_pimpl->dataLoaded) return;

  glEnable(GL_DEPTH_TEST);

  // render to texture
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _pimpl->frameBuffer);

  
  glViewport(0, 0, _pimpl->winWidth, _pimpl->winHeight);
  linkShader(_pimpl->program, _pimpl->bfVertHandle.get(), _pimpl->bfFragHandle.get());
  render(GL_FRONT);
  _pimpl->program.release();

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, _pimpl->winWidth, _pimpl->winHeight);
  linkShader(_pimpl->program, _pimpl->rcVertHandle.get(), _pimpl->rcFragHandle.get());
  rcSetUinforms();

  // cull back face
  render(GL_BACK);

  _pimpl->program.release();

#if 0
    glEnable( GL_ALPHA_TEST );
    glAlphaFunc( GL_GREATER, 0.05f );

    glEnable(GL_BLEND);
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    glLoadIdentity();

    // Translate and make 0.5f as the center 
    // (texture co ordinate is from 0 to 1. so center of rotation has to be 0.5f)
    glMatrixMode( GL_TEXTURE );
    glTranslatef( 0.5f, 0.5f, 0.5f );
    glRotatef(_pimpl->xRot, 1.0, 0.0, 0.0);
    glRotatef(_pimpl->zRot, 0.0, 0.0, 1.0);

    // A scaling applied to normalize the axis 
    // (Usually the number of slices will be less so if this is not - 
    // normalized then the z axis will look bulky)
    // Flipping of the y axis is done by giving a negative value in y axis.
    // This can be achieved either by changing the y co ordinates in -
    // texture mapping or by negative scaling of y axis
    glScaled(1.0, -1.0, (float)_pimpl->width/(float)_pimpl->depth);
    
    glTranslatef( -0.5f,-0.5f, -0.5f );

    glEnable(GL_TEXTURE_3D);
    glBindTexture(GL_TEXTURE_3D, _pimpl->texId);
    for ( float fIndx = -1.0f; fIndx <= 1.0f; fIndx+=0.005f )
    {
        glBegin(GL_QUADS);
            MAP_3DTEXT( fIndx );
        glEnd();
     }
#endif     
}
//! [7]

//! [8]
void VolumeRenderingWidget::resizeGL(int width, int height)
{
  _pimpl->winWidth = width;
  _pimpl->winHeight = height;
  _pimpl->texWidth = width;
  _pimpl->texHeight = height;

  // int side = qMin(width, height);
  // glViewport((width - side) / 2, (height - side) / 2, side, side);
#if 0
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  //glOrtho(-300.0, 300.0, -300.0, 300.0, -300.0, 300.0);
  glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
  glMatrixMode(GL_MODELVIEW);
#endif  
  // _pimpl->projection.setToIdentity();
  // _pimpl->projection.ortho(-300.0f, 300.0f, -300.0f, 300.0f, -300.0f, 300.0f);
}
//! [8]
#if 1
void VolumeRenderingWidget::wheelEvent(QWheelEvent * event) {
  // const QPoint angle = event->angleDelta();
  event->accept();
  // if (angle.y() > 0) {
  //   emit requestNextImage();
  // }
  // else {
  //   emit requestPrevImage();
  // }
}

void VolumeRenderingWidget::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    _pimpl->lastPoint = event->pos();
    _pimpl->rotFlag = true;
  }
}

void VolumeRenderingWidget::mouseMoveEvent(QMouseEvent * event) {
  if (_pimpl->rotFlag && (event->buttons() & Qt::LeftButton)) {
    const QPoint newPoint = event->pos();
    if (newPoint.x() < _pimpl->lastPoint.x())
      _pimpl->zRot -= 1;
    else
      _pimpl->zRot += 1;

    if (_pimpl->zRot < 0) _pimpl->zRot = 360;
    if (_pimpl->zRot > 360) _pimpl->zRot = 0;

    if (newPoint.y() < _pimpl->lastPoint.y())
      _pimpl->xRot -= 1;
    else
      _pimpl->xRot += 1;

    if (_pimpl->xRot < 0) _pimpl->xRot = 360;
    if (_pimpl->xRot > 360) _pimpl->xRot = 0;

    if (_pimpl->dataLoaded) updateGL();
  }
}

void VolumeRenderingWidget::mouseReleaseEvent(QMouseEvent *event) {
  if (_pimpl->rotFlag && (event->buttons() & Qt::LeftButton)) {
    _pimpl->rotFlag = false;
  }
}
#endif

void VolumeRenderingWidget::setImages(std::vector<boost::shared_ptr<const Image> >& images) {
  const int width = images.at(0)->width();
  _pimpl->width = width;
  const int height = images.at(0)->height();
  _pimpl->height = height;
  const int count = images.size();
  _pimpl->depth = count;

  const int imageSize = width * height;
  const int bufferSize = imageSize * count;

  char* rawBuffer = new char[bufferSize];
  int offset = 0;
  for (auto image : images) {
    memcpy(&rawBuffer[offset], image->rawPixelData8bit().get(), imageSize);
    offset += imageSize;
  }

  char* rgbaBuffer = new char[width * height * count * 4];
  for(int nIndx = 0; nIndx < bufferSize; ++nIndx ) {
    rgbaBuffer[nIndx*4] = rawBuffer[nIndx];
    rgbaBuffer[nIndx*4+1] = rawBuffer[nIndx];
    rgbaBuffer[nIndx*4+2] = rawBuffer[nIndx];
    rgbaBuffer[nIndx*4+3] = rawBuffer[nIndx];
  }

  if(_pimpl->texId != 0) {
    glDeleteTextures(1, &(_pimpl->texId));
  }

  glActiveTexture(GL_TEXTURE0);
  glGenTextures(1, &(_pimpl->texId));
  glBindTexture(GL_TEXTURE_3D, _pimpl->texId);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glPixelStorei(GL_UNPACK_ALIGNMENT,1);
  // glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, width, height, count, 0,
  //              GL_RGBA, GL_UNSIGNED_BYTE, rgbaBuffer);

  glTexImage3D(GL_TEXTURE_3D, 0, GL_LUMINANCE, width, height, count, 0,
               GL_LUMINANCE, GL_UNSIGNED_BYTE, rawBuffer);

  glBindTexture(GL_TEXTURE_3D, 0);

  delete[] rawBuffer;
  delete[] rgbaBuffer;
  _pimpl->dataLoaded = true;
}

void VolumeRenderingWidget::initShaders() {
  _pimpl->bfVertHandle = boost::shared_ptr<QGLShader>(new QGLShader(QGLShader::Vertex));
  _pimpl->bfVertHandle->compileSourceFile(":/shader/backface.vert");

  _pimpl->bfFragHandle = boost::shared_ptr<QGLShader>(new QGLShader(QGLShader::Fragment));
  _pimpl->bfFragHandle->compileSourceFile(":/shader/backface.frag");

  _pimpl->rcVertHandle = boost::shared_ptr<QGLShader>(new QGLShader(QGLShader::Vertex));
  _pimpl->rcVertHandle->compileSourceFile(":/shader/raycasting.vert");

  _pimpl->rcFragHandle = boost::shared_ptr<QGLShader>(new QGLShader(QGLShader::Fragment));
  _pimpl->rcFragHandle->compileSourceFile(":/shader/raycasting.frag");

#if 0  
  // Override system locale until shaders are compiled
  setlocale(LC_NUMERIC, "C");

  // Compile vertex shader
  if (!_pimpl->program.addShaderFromSourceFile(QGLShader::Vertex, ":/shader/VolumeRenderingWidget.vs"))
    close();

  // Compile fragment shader
  if (!_pimpl->program.addShaderFromSourceFile(QGLShader::Fragment, ":/shader/VolumeRenderingWidget.fs"))
    close();

  // Link shader pipeline
  if (!_pimpl->program.link())
    close();

  // Bind shader pipeline for use
  if (!_pimpl->program.bind())
    close();

  // Restore system locale
  setlocale(LC_ALL, "");
#endif  
}

void VolumeRenderingWidget::initScene() {
#if 0  
  if (!_pimpl->image) return;
  glDeleteBuffers(3, _pimpl->bufferObjects);

  _pimpl->view.lookAt(QVector3D(0.0f,0.0f,2.0f), QVector3D(0.0f,0.0f,0.0f), QVector3D(0.0f,1.0f,0.0f));
  const Image& image = *_pimpl->image;
  const float width = image.width();
  const float height = image.height();

  GLfloat vertices[] = 
    {-width/2.0f, -height/2.0f, 0.0f,
     -width/2.0f,  height/2.0f, 0.0f,
     width/2.0f,  height/2.0f, 0.0f,
     width/2.0f, -height/2.0f, 0.0f};

  glGenBuffers(3, _pimpl->bufferObjects);
  glBindBuffer(GL_ARRAY_BUFFER, _pimpl->bufferObjects[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 12, vertices, GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, _pimpl->bufferObjects[1]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 8,  textCoord, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _pimpl->bufferObjects[2]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * 4, indexes, GL_STATIC_DRAW);

  GLuint texId;
  glActiveTexture(GL_TEXTURE0);
  glGenTextures(1, &texId);
  glBindTexture(GL_TEXTURE_2D, texId);
  if (_pimpl->dataType == Image::SHORTBIT) {
    _pimpl->program.setUniformValue("edgeDetection", false);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height, 0, GL_LUMINANCE, GL_UNSIGNED_SHORT, image.pixelData().get());
  }
  else {
    _pimpl->program.setUniformValue("edgeDetection", _pimpl->edgeDetection);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, image.pixelData8bit().get());
  }

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_LINEAR); 
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_LINEAR); 
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  _pimpl->program.setUniformValueArray("kernelValue", kernel, 9, 1);

  const QVector2D offset[9] {QVector2D(-1.0/width, -1.0/height), QVector2D(0.0, -1.0/height), QVector2D(1.0/width, -1.0/height),
                             QVector2D(-1.0/width,  0.0), QVector2D(0.0,  0.0), QVector2D(1.0/width,  0.0/height),
                             QVector2D(-1.0/width,  1.0), QVector2D(0.0,  1.0/height), QVector2D(1.0/width,  1.0/height)};

  _pimpl->program.setUniformValueArray("texOffset", offset, 9);
#endif  
}


void VolumeRenderingWidget::initVBO() {
  glGenBuffers(2, _pimpl->gbo);
  GLuint vertexdat = _pimpl->gbo[0];
  GLuint veridxdat = _pimpl->gbo[1];
  glBindBuffer(GL_ARRAY_BUFFER, vertexdat);
  glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

  // used in glDrawElement()
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, veridxdat);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, 36 * sizeof(GLuint), indices, GL_STATIC_DRAW);

  GLuint vao;
  glGenVertexArrays(1, &vao);
  // vao like a closure binding 3 buffer object: verlocdat vercoldat and veridxdat
  glBindVertexArray(vao);
  glEnableVertexAttribArray(0); // for vertexloc
  glEnableVertexAttribArray(1); // for vertexcol

  // the vertex location is the same as the vertex color
  glBindBuffer(GL_ARRAY_BUFFER, vertexdat);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLfloat *)NULL);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLfloat *)NULL);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, veridxdat);
  // glBindVertexArray(0);
  _pimpl->vao = vao;
}

GLuint VolumeRenderingWidget::initTFF1DTex(const char* filename) {
  ifstream inFile(filename, ifstream::in);
  if (!inFile) {
    cerr << "Error openning file: " << filename << endl;
    return 0;
  }
    
  const int MAX_CNT = 10000;
  GLubyte *tff = (GLubyte *) calloc(MAX_CNT, sizeof(GLubyte));
  inFile.read(reinterpret_cast<char *>(tff), MAX_CNT);
  if (inFile.eof()) {
    size_t bytecnt = inFile.gcount();
    *(tff + bytecnt) = '\0';
    cout << "bytecnt " << bytecnt << endl;
  }
  else if(inFile.fail()) {
    cout << filename << "read failed " << endl;
  }
  else {
    cout << filename << "is too large" << endl;
  }   

  GLuint tff1DTex;
  glGenTextures(1, &tff1DTex);
  glBindTexture(GL_TEXTURE_1D, tff1DTex);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA8, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, tff);
  free(tff);    
  return tff1DTex;
}

GLuint VolumeRenderingWidget::initFace2DTex(GLuint texWidth, GLuint texHeight) {
  GLuint backFace2DTex;
  glGenTextures(1, &backFace2DTex);
  glBindTexture(GL_TEXTURE_2D, backFace2DTex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, texWidth, texHeight, 0, GL_RGBA, GL_FLOAT, NULL);
  return backFace2DTex;
}

void VolumeRenderingWidget::initFrameBuffer(GLuint texObj, GLuint texWidth, GLuint texHeight) {
  // create a depth buffer for our framebuffer
  GLuint depthBuffer;
  glGenRenderbuffers(1, &depthBuffer);
  glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, texWidth, texHeight);

  // attach the texture and the depth buffer to the framebuffer
  glGenFramebuffers(1, &(_pimpl->frameBuffer));
  glBindFramebuffer(GL_FRAMEBUFFER, _pimpl->frameBuffer);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texObj, 0);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

  GLenum complete = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (complete != GL_FRAMEBUFFER_COMPLETE) {
    cerr << "framebuffer is not complete" << endl;
    return;
  }

  glEnable(GL_DEPTH_TEST);
}

void VolumeRenderingWidget::render(GLenum cullFace) {
  glClearColor(0.2f,0.2f,0.2f,1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  //  transform the box
  glm::mat4 projection = glm::perspective(50.0f, (GLfloat)_pimpl->winWidth/_pimpl->winHeight, 0.1f, 400.f);
  //glm::mat4 projection = glm::perspective(60.0f, (GLfloat)_pimpl->winWidth/_pimpl->winHeight, 0.1f, 200.f);
  // glm::mat4 projection = glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, -2.0f, 2.0f);
  glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 2.0f),
                               glm::vec3(0.0f, 0.0f, 0.0f), 
                               glm::vec3(0.0f, 1.0f, 0.0f));

  glm::mat4 model = glm::mat4(1.0f);

 // to make the "head256.raw" i.e. the volume data stand up.
  // model *= glm::scale(glm::mat4(1.0), glm::vec3(3.0f, 3.0f, 3.0f));
  model *= glm::rotate(_pimpl->zRot, glm::vec3(0.0f, 0.0f, 1.0f));
  model *= glm::rotate(_pimpl->xRot, glm::vec3(1.0f, 0.0f, 0.0f));
  model *= glm::translate(glm::vec3(-0.5f, -0.5f, -0.5f)); 
  // notice the multiplication order: reverse order of transform
  glm::mat4 mvp = projection * view * model;
  GLuint mvpIdx = glGetUniformLocation(_pimpl->program.programId(), "MVP");
  glUniformMatrix4fv(mvpIdx, 1, GL_FALSE, &mvp[0][0]);

  drawBox(cullFace);
}

void VolumeRenderingWidget::drawBox(GLenum glFaces)
{
    glEnable(GL_CULL_FACE);
    glCullFace(glFaces);
    glBindVertexArray(_pimpl->vao);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (GLuint *)NULL);
    glDisable(GL_CULL_FACE);
}

// link the shader objects using the shader program
#if 0
void VolumeRenderingWidget::linkShader(GLuint shaderPgm, GLuint newVertHandle, GLuint newFragHandle)
{
    // setlocale(LC_NUMERIC, "C");
    const GLsizei maxCount = 2;
    GLsizei count;
    GLuint shaders[maxCount];
    glGetAttachedShaders(shaderPgm, maxCount, &count, shaders);
    // cout << "get VertHandle: " << shaders[0] << endl;
    // cout << "get FragHandle: " << shaders[1] << endl;
    // GL_ERROR();
    for (int i = 0; i < count; i++) {
      glDetachShader(shaderPgm, shaders[i]);
    }
    // Bind index 0 to the shader input variable "VerPos"
    glBindAttribLocation(shaderPgm, 0, "VerPos");
    // Bind index 1 to the shader input variable "VerClr"
    glBindAttribLocation(shaderPgm, 1, "VerClr");
    // GL_ERROR();
    glAttachShader(shaderPgm,newVertHandle);
    glAttachShader(shaderPgm,newFragHandle);

    // Link shader pipeline
    if (!_pimpl->program.link())
      close();

    // Bind shader pipeline for use
    if (!_pimpl->program.bind())
      close();

    // Restore system locale
    // setlocale(LC_ALL, "");



    // GL_ERROR();
    // glLinkProgram(shaderPgm);

    if (!_pimpl->program.isLinked())
    {
      cerr << "Failed to relink shader program!" << endl;
      return;
    }
    // GL_ERROR();
}
#endif

void VolumeRenderingWidget::linkShader(QGLShaderProgram& shaderPgm, QGLShader* vertShader, QGLShader* fragShader) {
  setlocale(LC_NUMERIC, "C");

  shaderPgm.removeAllShaders();

  shaderPgm.addShader(vertShader);
  shaderPgm.addShader(fragShader);

  // Bind index 0 to the shader input variable "VerPos"
  glBindAttribLocation(shaderPgm.programId(), 0, "VerPos");
  // Bind index 1 to the shader input variable "VerClr"
  glBindAttribLocation(shaderPgm.programId(), 1, "VerClr");

  // Link shader pipeline
  if (!shaderPgm.link()) {
    cerr<<"Failed to relink shader program!"<<endl;
    close();
  }

  // Bind shader pipeline for use
  if (!shaderPgm.bind()) {
    cerr<<"Failed to rebind shader program!"<<endl;
    close();
  }

  // cout<<"Link shaders successfully!"<<endl;

  // Restore system locale
  setlocale(LC_ALL, "");
}

void VolumeRenderingWidget::rcSetUinforms()
{
    // setting uniforms such as
    // ScreenSize 
    // StepSize
    // TransferFunc
    // ExitPoints i.e. the backface, the backface hold the ExitPoints of ray casting
    // VolumeTex the texture that hold the volume data i.e. head256.raw
    GLint screenSizeLoc = glGetUniformLocation(_pimpl->program.programId(), "ScreenSize");
    if (screenSizeLoc >= 0)
    {
      glUniform2f(screenSizeLoc, (float)_pimpl->winWidth, (float)_pimpl->winHeight);
    }
    else
    {
  cout << "ScreenSize"
       << "is not bind to the uniform"
       << endl;
    }
    GLint stepSizeLoc = glGetUniformLocation(_pimpl->program.programId(), "StepSize");
    // GL_ERROR();
    if (stepSizeLoc >= 0)
    {
      glUniform1f(stepSizeLoc, g_stepSize);
    }
    else
    {
  cout << "StepSize"
       << "is not bind to the uniform"
       << endl;
    }
    // GL_ERROR();
    GLint transferFuncLoc = glGetUniformLocation(_pimpl->program.programId(), "TransferFunc");
    if (transferFuncLoc >= 0)
    {
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_1D, _pimpl->tffTexObj);
      glUniform1i(transferFuncLoc, 0);
    }
    else
    {
  cout << "TransferFunc"
       << "is not bind to the uniform"
       << endl;
    }
    // GL_ERROR();    
    GLint backFaceLoc = glGetUniformLocation(_pimpl->program.programId(), "ExitPoints");
    if (backFaceLoc >= 0)
    {
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, _pimpl->bfTexObj);
  glUniform1i(backFaceLoc, 1);
    }
    else
    {
  cout << "ExitPoints"
       << "is not bind to the uniform"
       << endl;
    }
    // GL_ERROR();    
    GLint volumeLoc = glGetUniformLocation(_pimpl->program.programId(), "VolumeTex");
    if (volumeLoc >= 0)
    {
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_3D, _pimpl->texId);
  glUniform1i(volumeLoc, 2);
    }
    else
    {
  cout << "VolumeTex"
       << "is not bind to the uniform"
       << endl;
    }
    
}
