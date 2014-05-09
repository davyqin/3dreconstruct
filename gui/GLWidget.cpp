#include <QtGui>
#include <QtOpenGL>

#include "GLWidget.h"
#include "model/Image.h"

#include <boost/shared_ptr.hpp>
#include <vector>

using namespace std;

class GLWidget::Pimpl {
public:
  Pimpl()
  : qtRed(QColor::fromRgb(255,0,0))
  , qtDark(QColor::fromRgb(0,0,0))
  , qtPurple(QColor::fromCmykF(0.39, 0.39, 0.0, 0.0))
  , dataType(Image::SHORTBIT)
  , zoomFlag(false)
  , zoomValue(1.0) {}

  QColor qtRed;
  QColor qtDark;
  QColor qtPurple;
  boost::shared_ptr<const Image> image;
  Image::DataType dataType;
  bool zoomFlag;
  double zoomValue;
  QPoint lastPoint;
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
  qglClearColor(_pimpl->qtPurple);
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
      double widthZoom = _pimpl->zoomValue;
      double heightZoom = _pimpl->zoomValue;
      if (image.width() > image.height()) heightZoom *= (double)image.width() / (double)image.height();
      if (image.width() < image.height()) widthZoom *= (double)image.height() / (double)image.width();
      glPixelZoom(widthZoom, heightZoom);

      if (_pimpl->dataType == Image::SHORTBIT) {
        const unsigned short* pixel = image.pixelData().get();
        glDrawPixels(image.width(), image.height(), GL_LUMINANCE, GL_UNSIGNED_SHORT, pixel);
      }
      else {
        const unsigned char* pixel = image.pixelData8bit().get();
        glDrawPixels(image.width(), image.height(), GL_LUMINANCE, GL_UNSIGNED_BYTE, pixel);
      }
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

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glOrtho(-300.0, 300.0, -300.0, 300.0, -1000.0, 1000.0);
  glMatrixMode(GL_MODELVIEW);
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
