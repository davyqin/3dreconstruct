#include <QtGui>
#include <QtOpenGL>
#include <QGLShaderProgram>
#include <QMatrix4x4>

#include "GLWidget.h"
#include "model/Image.h"
#include "cuda/cuda_kernels.h"
#include "cuda/cuda_common.h"

#include <boost/shared_ptr.hpp>
#include <vector>
#include <cuda_runtime.h>
#include <cuda_gl_interop.h>

using namespace std;

namespace {
 GLfloat textCoord[] = {0.0f, 0.0f, 
                        0.0f, 1.0f, 
                        1.0f, 1.0f, 
                        1.0f, 0.0f};

 GLuint indexes[] = {0, 1, 2, 3};

 const GLfloat kernel[9] = {0.0, -1.0, 0.0, -1.0, 4.0, -1.0, 0.0, -1.0, 0.0};
}

class GLWidget::Pimpl {
public:
  Pimpl()
  : qtRed(QColor::fromRgb(255,0,0))
  , qtDark(QColor::fromRgb(0,0,0))
  , qtPurple(QColor::fromCmykF(0.39, 0.39, 0.0, 0.0))
  , dataType(Image::CHARBIT)
  , zoomFlag(false)
  , zoomValue(1.0)
  , edgeDetection(false)
  , cudaFilter(true)
  , filterType(GLWidget::NONE) {}

  QColor qtRed;
  QColor qtDark;
  QColor qtPurple;
  boost::shared_ptr<const Image> image;
  Image::DataType dataType;
  bool zoomFlag;
  double zoomValue;
  QPoint lastPoint;

  QGLShaderProgram program;
  QMatrix4x4 view;
  QMatrix4x4 projection;
  GLuint bufferObjects[3];
  GLuint cudaBuffer;
  bool edgeDetection;
  bool cudaFilter;
  GLuint texId;
  FilterType filterType;

  struct cudaGraphicsResource* cuda_pbo_resource;
};

//! [0]
GLWidget::GLWidget(QWidget *parent)
  : QGLWidget(QGLFormat(QGL::DoubleBuffer), parent)
  , _pimpl(new Pimpl()) {}
//! [0]

//! [1]
GLWidget::~GLWidget() {}
//! [1]

//! [2]
QSize GLWidget::minimumSizeHint() const
{
    return QSize(100, 100);
}
//! [2]

//! [3]
QSize GLWidget::sizeHint() const
//! [3] //! [4]
{
    return QSize(512, 512);
}
//! [4]


//! [6]
void GLWidget::initializeGL()
{
  initializeGLFunctions();
  qglClearColor(_pimpl->qtPurple);
  initShaders();
  glEnable(GL_DEPTH_TEST);
}
//! [6]

//! [7]
void GLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    if (_pimpl->image) {
      const Image& image = *_pimpl->image;

      if (_pimpl->dataType == Image::CHARBIT && _pimpl->filterType == GLWidget::CUDA) {
         // Sobel operation
        unsigned char *data = NULL;

        // map PBO to get CUDA device pointer
        cudaGraphicsMapResources(1, &_pimpl->cuda_pbo_resource, 0);
        size_t num_bytes;
        cudaGraphicsResourceGetMappedPointer((void **)&data, &num_bytes, _pimpl->cuda_pbo_resource);

        sobelFilter(data, image.width(), image.height());
        cudaGraphicsUnmapResources(1, &_pimpl->cuda_pbo_resource, 0);

        glBindTexture(GL_TEXTURE_2D, _pimpl->texId);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, _pimpl->cudaBuffer);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image.width(), image.height(), GL_LUMINANCE, GL_UNSIGNED_BYTE, NULL);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
      }
      
      _pimpl->program.setUniformValue("texture", 0);

      double widthZoom = _pimpl->zoomValue;
      double heightZoom = _pimpl->zoomValue;
      if (image.width() > image.height()) heightZoom *= (double)image.width() / (double)image.height();
      if (image.width() < image.height()) widthZoom *= (double)image.height() / (double)image.width();

      QMatrix4x4 model;
      model.scale(widthZoom, heightZoom);
      _pimpl->program.setUniformValue("MVP", _pimpl->projection * model);

      glBindBuffer(GL_ARRAY_BUFFER, _pimpl->bufferObjects[0]);
      const int vertexLocation = _pimpl->program.attributeLocation("VertexPosition");
      _pimpl->program.enableAttributeArray(vertexLocation);
      glVertexAttribPointer(vertexLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);

      glBindBuffer(GL_ARRAY_BUFFER, _pimpl->bufferObjects[1]);
      int texcoordLocation = _pimpl->program.attributeLocation("VertexTexCoord");
      _pimpl->program.enableAttributeArray(texcoordLocation);
      glVertexAttribPointer(texcoordLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _pimpl->bufferObjects[2]);
      glDrawElements(GL_QUADS, 4, GL_UNSIGNED_INT, 0);
    }

 #if 0
    // qglColor(_pimpl->qtRed); /* draw in red */

    /* define a point data type */

    typedef GLfloat point2[2];

    point2 vertices[3]={{0.0,0.0},{250.0,500.0},{500.0,0.0}}; /* A triangle */

    int j, k;
    int rand();       /* standard random number generator */
    point2 p ={75.0,50.0};  /* An arbitrary initial point inside traingle */

    glClear(GL_COLOR_BUFFER_BIT);  /*clear the window */


    /* compute and plots 5000 new points */

    for( k=0; k<10000; k++)
    {
        j=rand()%3; /* pick a vertex at random */


        /* Compute point halfway between selected vertex and old point */

        p[0] = (p[0]+vertices[j][0])/2.0;
        p[1] = (p[1]+vertices[j][1])/2.0;

        /* plot new point */

        glBegin(GL_POINTS);
        glVertex2fv(p);
        glEnd();

    }
#endif
    glFlush(); /* clear buffers */

}
//! [7]

//! [8]
void GLWidget::resizeGL(int width, int height)
{
  int side = qMin(width, height);
  glViewport((width - side) / 2, (height - side) / 2, side, side);
#if 0
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glOrtho(-300.0, 300.0, -300.0, 300.0, -300.0, 300.0);
  glMatrixMode(GL_MODELVIEW);
#endif  
  _pimpl->projection.setToIdentity();
  _pimpl->projection.ortho(-300.0f, 300.0f, -300.0f, 300.0f, -300.0f, 300.0f);
}
//! [8]

void GLWidget::wheelEvent(QWheelEvent * event) {
  const QPoint angle = event->angleDelta();
  event->accept();
  if (angle.y() > 0) {
    emit requestNextImage();
  }
  else {
    emit requestPrevImage();
  }
}

void GLWidget::setImage(boost::shared_ptr<const Image> image) {
  _pimpl->image = image;
  initScene();
}

void GLWidget::setDataType(int dataType) {
  _pimpl->dataType = static_cast<Image::DataType>(dataType);
}

void GLWidget::mousePressEvent(QMouseEvent *event) {
  if (!_pimpl->image) return;

  if (event->button() == Qt::RightButton) {
    _pimpl->lastPoint = event->pos();
    _pimpl->zoomFlag = true;
  }

  if (event->button() == Qt::MidButton) {
    _pimpl->zoomValue = 1.0;
    updateGL();
  }
}

void GLWidget::mouseMoveEvent(QMouseEvent * event) {
  if (_pimpl->image && _pimpl->zoomFlag && (event->buttons() & Qt::RightButton)) {
    const QPoint newPoint = event->pos();
    bool needUpdate = false;
    if (newPoint.x() < _pimpl->lastPoint.x()) {
      _pimpl->zoomValue -= 0.1;
      needUpdate = true;
    }
    
    if (newPoint.x() > _pimpl->lastPoint.x()) {
      _pimpl->zoomValue += 0.1;
      needUpdate = true;
    }

    if (_pimpl->zoomValue < 1.0) {
      _pimpl->zoomValue = 1.0;
      needUpdate = false;
    }

    if (needUpdate) updateGL();
  }
}

void GLWidget::mouseReleaseEvent(QMouseEvent *event) {
  if (_pimpl->image && _pimpl->zoomFlag && (event->buttons() & Qt::RightButton)) {
    _pimpl->zoomFlag = false;
  }
}

void GLWidget::initShaders() {
  // Override system locale until shaders are compiled
  setlocale(LC_NUMERIC, "C");

  // Compile vertex shader
  if (!_pimpl->program.addShaderFromSourceFile(QGLShader::Vertex, ":/shader/glwidget.vs"))
    close();

  // Compile fragment shader
  if (!_pimpl->program.addShaderFromSourceFile(QGLShader::Fragment, ":/shader/glwidget.fs"))
    close();

  // Link shader pipeline
  if (!_pimpl->program.link())
    close();

  // Bind shader pipeline for use
  if (!_pimpl->program.bind())
    close();

  // Restore system locale
  setlocale(LC_ALL, "");
}

void GLWidget::initScene() {
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

  initImageTexture(_pimpl->filterType);
}

void GLWidget::initImageTexture(FilterType filterType) {
  if (!_pimpl->image) return;

  const Image& image = *_pimpl->image;
  const float width = image.width();
  const float height = image.height();

  //GLuint texId;
  glActiveTexture(GL_TEXTURE0);
  glGenTextures(1, &_pimpl->texId);
  glBindTexture(GL_TEXTURE_2D, _pimpl->texId);

  // no edge detection
  if (_pimpl->dataType == Image::SHORTBIT) {
    _pimpl->program.setUniformValue("edgeDetection", false);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height, 0, GL_LUMINANCE, GL_UNSIGNED_SHORT, image.pixelData().get());
  }

  if (_pimpl->dataType == Image::CHARBIT && _pimpl->filterType == GLWidget::NONE) {
    _pimpl->program.setUniformValue("edgeDetection", false);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, image.pixelData8bit().get());
  }

  // glsl edge detection
  if (_pimpl->dataType == Image::CHARBIT && _pimpl->filterType == GLWidget::GLSL) {
    _pimpl->program.setUniformValue("edgeDetection", true);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, image.pixelData8bit().get());

    _pimpl->program.setUniformValueArray("kernelValue", kernel, 9, 1);

    const QVector2D offset[9] = {QVector2D(-1.0/width, -1.0/height), QVector2D(0.0, -1.0/height), QVector2D(1.0/width, -1.0/height),
                                 QVector2D(-1.0/width,  0.0), QVector2D(0.0,  0.0), QVector2D(1.0/width,  0.0/height),
                                 QVector2D(-1.0/width,  1.0), QVector2D(0.0,  1.0/height), QVector2D(1.0/width,  1.0/height)};

    _pimpl->program.setUniformValueArray("texOffset", offset, 9);
  }

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_LINEAR); 
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_LINEAR); 
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // cuda edge detection
  if (_pimpl->dataType == Image::CHARBIT && _pimpl->filterType == GLWidget::CUDA) {
    _pimpl->program.setUniformValue("edgeDetection", false);

    setupTexture(width, height, image.pixelData8bit().get());
    glGenBuffers(1, &_pimpl->cudaBuffer);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, _pimpl->cudaBuffer);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, sizeof(unsigned char) * width * height, image.pixelData8bit().get(), GL_STREAM_DRAW);

    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    // register this buffer object with CUDA
    checkCudaErrors(cudaGraphicsGLRegisterBuffer(&_pimpl->cuda_pbo_resource,  _pimpl->cudaBuffer, cudaGraphicsMapFlagsWriteDiscard));

    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
  }
}

void GLWidget::setFilter(int filter) {
  if (filter == 0) _pimpl->filterType = NONE;
  if (filter == 1) _pimpl->filterType = GLSL;
  if (filter == 2) _pimpl->filterType = CUDA;

  initImageTexture(_pimpl->filterType);
  updateGL();
}
