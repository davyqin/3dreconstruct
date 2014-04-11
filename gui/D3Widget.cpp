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
  , qtPurple(QColor::fromCmykF(0.39, 0.39, 0.0, 0.0)) {}

  QColor qtRed;
  QColor qtDark;
  QColor qtPurple;
  std::vector<boost::shared_ptr<const Triangle> > data;
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
      gluLookAt(-300, -300, -1000, -50, -50, -900, 0.5, 0.5, 0.5);
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

    glFlush(); /* clear buffers */
}

void D3Widget::resizeGL(int width, int height)
{
  int side = qMin(width, height);
  glViewport((width - side) / 2, (height - side) / 2, side, side);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glOrtho(-300.0, 300.0, -300.0, 300.0, -1000.0, 1000.0);
  glMatrixMode(GL_MODELVIEW);
}

void D3Widget::wheelEvent(QWheelEvent * event) {
  // const QPoint angle = event->angleDelta();
  event->accept();
  // if (angle.y() > 0) {
  //   emit requestNextImage();
  // }
  // else {
  //   emit requestPrevImage();
  // }
}

void D3Widget::setData(const std::vector<boost::shared_ptr<const Triangle> >& data) {
  _pimpl->data = data;
}
