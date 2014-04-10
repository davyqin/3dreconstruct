#include <QtGui>
#include <QtOpenGL>

#include "D3Widget.h"
#include "mc/Triangle.h"

#include <GL/glut.h>
#include <boost/shared_ptr.hpp>
#include <vector>

using namespace std;

class D3Widget::Pimpl {
public:
  Pimpl()
  : qtRed(QColor::fromRgb(255,0,0))
  , qtDark(QColor::fromRgb(0,0,0))
  , qtPurple(QColor::fromCmykF(0.39, 0.39, 0.0, 0.0)) {}

  QColor qtRed;
  QColor qtDark;
  QColor qtPurple;
  std::vector<Triangle> data;
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
  qglClearColor(_pimpl->qtPurple);
//    qglColor(qtRed); /* draw in red */

  glEnable(GL_DEPTH_TEST);
//    glShadeModel(GL_SMOOTH);
//    glEnable(GL_MULTISAMPLE);
}

void D3Widget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    qglColor(_pimpl->qtRed); /* draw in red */
#if 0
    if (!_pimpl->data.empty()) {
      gluLookAt(-300, -300, -1000, -50, -50, -900, 0.5, 0.5, 0.5);
      // glOrtho(-500.0, 500.0, -500.0, 500.0, -1000.0, 1000.0);
      // glColor3f(1.0, 1.0, 1.0);
          
      // glPushMatrix();
      // glMatrixMode(GL_MODELVIEW);
      // glLoadIdentity();
      for (auto triangle : _pimpl->data) {
        glBegin(GL_TRIANGLES);
        
        // cout<<"********************************"<<endl;
        const std::vector<double> normals = triangle.normals();
        glNormal3d(normals.at(0), normals.at(1), normals.at(2));
        // glNormal3dv(&normals[0]);
        const std::vector<Vertex> vertices = triangle.vertices();
        for (auto vertex : vertices) {
          glVertex3d(vertex.x(), vertex.y(), (vertex.z()));
        }
        glEnd();
        // cout<<"********************************"<<endl;
      }
      // glPopMatrix();
    }
  #endif

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

void D3Widget::setData(const std::vector<Triangle>& data) {
  _pimpl->data = data;
}
