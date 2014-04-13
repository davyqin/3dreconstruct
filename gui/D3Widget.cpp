#include <QtGui>
#include <QtOpenGL>

#include "D3Widget.h"
#include "mc/Triangle.h"

#include <GL/glut.h>
#include <boost/shared_ptr.hpp>
#include <vector>

using namespace std;

namespace {
  const GLfloat ambient[4] = {1.0, 0.0, 0.0, 1.0};
  const GLfloat difffuse[] = {0.0, 0.0, 1.0, 1.0};
  const GLfloat spec[4] = {1.0, 1.0, 1.0, 1.0};
  const GLfloat light_position[] = {-1.0, -1.0, 1.0, 0.0};
}

class D3Widget::Pimpl {
public:
  Pimpl()
  : qtRed(QColor::fromRgb(255,0,0))
  , qtDark(QColor::fromRgb(0,0,0))
  , qtPurple(QColor::fromCmykF(0.39, 0.39, 0.0, 0.0))
  , zoom(1.0)
  , minX(std::numeric_limits<double>::max())
  , minY(std::numeric_limits<double>::max())
  , minZ(std::numeric_limits<double>::max())
  , maxX(std::numeric_limits<double>::min())
  , maxY(std::numeric_limits<double>::min())
  , maxZ(std::numeric_limits<double>::min()) {}

  QColor qtRed;
  QColor qtDark;
  QColor qtPurple;
  std::vector<boost::shared_ptr<const Triangle> > data;
  GLfloat zoom;
  double minX;
  double minY;
  double minZ;
  double maxX;
  double maxY;
  double maxZ;
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
  glMaterialf(GL_FRONT, GL_SHININESS, 50);
  glLightfv(GL_LIGHT0, GL_SPECULAR, spec);
  glLightfv(GL_LIGHT0, GL_POSITION, light_position);

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_DEPTH_TEST);
}

void D3Widget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    qglColor(_pimpl->qtRed); /* draw in red */

    if (!_pimpl->data.empty()) {
      // glPushMatrix();
      // const double centerX = (_pimpl->minX + _pimpl->maxX) / 2;
      // const double centerY = (_pimpl->minY + _pimpl->maxY) / 2;
      // const double centerZ = (_pimpl->minZ + _pimpl->maxZ) / 2;
      // glTranslatef(-centerX, -centerY, -centerZ);


      for (auto triangle : _pimpl->data) {
        glBegin(GL_TRIANGLES);    
        const std::vector<double> normals = triangle->normals();
        glNormal3d(normals.at(0), normals.at(1), normals.at(2));
        const std::vector<Vertex> vertices = triangle->vertices();
        for (auto vertex : vertices) {
          glVertex3d(vertex.x(), vertex.y(), (vertex.z()));
        }
        glEnd();
      }
    }

    // glPopMatrix();

    glRotatef(90.0, 0.0, 0.0, 1.0);

    glFlush(); /* clear buffers */
}

void D3Widget::resizeGL(int width, int height)
{
  const int side = qMin(width, height);
  glViewport((width - side) / 2, (height - side) / 2, side * _pimpl->zoom, side * _pimpl->zoom);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-300.0, 300.0, -300.0, 300.0, -1000.0, 1000.0);
  glMatrixMode(GL_MODELVIEW);
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

  resizeGL(width(), height());
  updateGL();
}

void D3Widget::setData(const std::vector<boost::shared_ptr<const Triangle> >& data) {
  _pimpl->data = data;

  for (auto triangle : _pimpl->data) {
    const std::vector<Vertex> vertices = triangle->vertices();
    for (auto vertex : vertices) {
      if (vertex.x() < _pimpl->minX) _pimpl->minX = vertex.x();
      if (vertex.x() > _pimpl->maxX) _pimpl->maxX = vertex.x();
      if (vertex.y() < _pimpl->minY) _pimpl->minY = vertex.y();
      if (vertex.y() > _pimpl->maxX) _pimpl->maxY = vertex.y();
      if (vertex.z() < _pimpl->minZ) _pimpl->minZ = vertex.z();
      if (vertex.z() > _pimpl->maxZ) _pimpl->maxZ = vertex.z();
    }
  }
}
