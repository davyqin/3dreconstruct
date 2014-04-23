#include <QtGui>
#include <QtOpenGL>

#include "D3Widget.h"
#include "mc/Triangle.h"

#include <GL/glut.h>
#include <boost/shared_ptr.hpp>
#include <vector>
#include <limits>

using namespace std;

namespace {
  const GLfloat ambient[4] = {1.0f, 0.0f, 0.0f, 1.0f};
  const GLfloat difffuse[] = {0.0f, 0.0f, 1.0f, 1.0f};
  const GLfloat spec[4] = {1.0, 1.0, 1.0, 1.0};
  const GLfloat light_position[] = {-300.0f, -300.0f, 0.0f, 0.0f};
}

class D3Widget::Pimpl {
public:
  Pimpl()
  : qtRed(QColor::fromRgb(255,0,0))
  , qtDark(QColor::fromRgb(0,0,0))
  , qtPurple(QColor::fromCmykF(0.39, 0.39, 0.0, 0.0))
  , zoom(1.0)
  , minX(std::numeric_limits<float>::max())
  , minY(std::numeric_limits<float>::max())
  , minZ(std::numeric_limits<float>::max())
  , maxX(-2000.0)
  , maxY(-2000.0)
  , maxZ(-2000.0)
  , zRot(225.0)
  , rotFlag(false) {}

  QColor qtRed;
  QColor qtDark;
  QColor qtPurple;
  std::vector<boost::shared_ptr<const Triangle> > data;
  GLfloat zoom;
  float minX;
  float minY;
  float minZ;
  float maxX;
  float maxY;
  float maxZ;
  float centerX;
  float centerY;
  float centerZ;
  float zRot;
  bool rotFlag;
  QPoint lastPoint;
};

D3Widget::D3Widget(QWidget *parent)
  : QGLWidget(QGLFormat(QGL::DoubleBuffer), parent)
  , _pimpl(new Pimpl()) {}

D3Widget::~D3Widget() {}

QSize D3Widget::minimumSizeHint() const
{
  return QSize(100, 100);
}

QSize D3Widget::sizeHint() const
{
  return QSize(512, 512);
}

void D3Widget::initializeGL()
{
  qglClearColor(_pimpl->qtDark);
  glShadeModel (GL_SMOOTH);

  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
  glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
  glMaterialf(GL_FRONT, GL_SHININESS, 20);
  glLightfv(GL_LIGHT0, GL_SPECULAR, spec);
  glLightfv(GL_LIGHT0, GL_POSITION, light_position);

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_DEPTH_TEST);
}

void D3Widget::paintGL()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (!_pimpl->data.empty()) {
    glPushMatrix();
    glColor3i(244, 164, 96);
    glScalef(_pimpl->zoom, _pimpl->zoom, _pimpl->zoom);
    glRotatef(270.0f, 1.0f, 0.0f, 0.0f);
    glRotatef(_pimpl->zRot, 0.0f, 0.0f, 1.0f);
    glTranslated(-_pimpl->centerX, -_pimpl->centerY, -_pimpl->centerZ);

    for (auto triangle : _pimpl->data) {
      glBegin(GL_TRIANGLES);    
      const std::vector<float> normal = std::move(triangle->normal());
      glNormal3fv(&normal[0]);
      const std::vector<Vertex> vertices = std::move(triangle->vertices());
      for (auto vertex : vertices) {
        glVertex3f(vertex.x(), vertex.y(), vertex.z());
      }
      glEnd();
    }

    glPopMatrix();
  }
  
  //glFlush(); /* clear buffers */
}

void D3Widget::resizeGL(int width, int height)
{
  const int side = qMin(width, height);
  glViewport((width - side) / 2, (height - side) / 2, side, side);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-300, 300, -300, 300, -300.0, 300.0);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

void D3Widget::wheelEvent(QWheelEvent * event) {
  const QPoint angle = event->angleDelta();
  event->accept();
  if (angle.y() > 0) {
    _pimpl->zoom *= 1.1;
  }
  else {
    _pimpl->zoom /= 1.1;
  }

  updateGL();
}

void D3Widget::setData(const std::vector<boost::shared_ptr<const Triangle> >& data) {
  _pimpl->data = data;
  if (_pimpl->data.empty()) return;

  for (auto triangle : _pimpl->data) {
    const std::vector<Vertex> vertices = triangle->vertices();
    for (auto vertex : vertices) {
      if (vertex.x() < _pimpl->minX) { _pimpl->minX = vertex.x(); }
      if (vertex.x() > _pimpl->maxX) { _pimpl->maxX = vertex.x(); }
      if (vertex.y() < _pimpl->minY) { _pimpl->minY = vertex.y(); }
      if (vertex.y() > _pimpl->maxY) { _pimpl->maxY = vertex.y(); }
      if (vertex.z() < _pimpl->minZ) { _pimpl->minZ = vertex.z(); }
      if (vertex.z() > _pimpl->maxZ) { _pimpl->maxZ = vertex.z(); }
    }
  }
  _pimpl->centerX = (_pimpl->minX + _pimpl->maxX) / 2;
  _pimpl->centerY = (_pimpl->minY + _pimpl->maxY) / 2;
  _pimpl->centerZ = (_pimpl->minZ + _pimpl->maxZ) / 2;
  cout <<"Input "<<data.size()<<" triangles"<<endl;
  cout <<_pimpl->minX<<" "<<_pimpl->centerX<<" "<<_pimpl->maxX<<endl;
  cout <<_pimpl->minY<<" "<<_pimpl->centerY<<" "<<_pimpl->maxY<<endl;
  cout <<_pimpl->minZ<<" "<<_pimpl->centerZ<<" "<<_pimpl->maxZ<<endl;
}

void D3Widget::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    _pimpl->lastPoint = event->pos();
    _pimpl->rotFlag = true;
  }
}

void D3Widget::mouseMoveEvent(QMouseEvent * event) {
  if (_pimpl->rotFlag && (event->buttons() & Qt::LeftButton)) {
    const QPoint newPoint = event->pos();
    if (newPoint.x() < _pimpl->lastPoint.x())
      _pimpl->zRot -= 1;
    else
      _pimpl->zRot += 1;

    if (_pimpl->zRot < 0) _pimpl->zRot = 360;
    if (_pimpl->zRot > 360) _pimpl->zRot = 0;
    if(!_pimpl->data.empty()) updateGL();
  }
}

void D3Widget::mouseReleaseEvent(QMouseEvent *event) {
  if (_pimpl->rotFlag && (event->buttons() & Qt::LeftButton)) {
    _pimpl->rotFlag = false;
  }
}
