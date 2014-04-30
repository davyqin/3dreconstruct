#include <QtGui>
#include <QtOpenGL>
#include <QGLFunctions>
#include <QGLShaderProgram>

#include "D3Widget.h"
#include "mc/Triangle.h"

// #include <GL/glut.h>
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

class D3Widget::Pimpl : protected QGLFunctions {
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
  , rotFlag(false)
  , indexes(0)
  , points(0)
  , normals(0)
  , indexCount(0)
  , vertexCount(0)
  , dataLoaded(false) {}

  QColor qtRed;
  QColor qtDark;
  QColor qtPurple;
  // std::vector<boost::shared_ptr<const Triangle> > data;
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
  // std::vector<std::vector<float> > normals;
  // std::vector<std::vector<float> > points;
  // std::vector<short> indexes;
  GLushort *indexes;
  GLfloat *points;
  GLfloat *normals;
  int indexCount;
  int vertexCount;

  GLuint m_VertexObject;
  GLuint m_VertexArray;
  GLuint m_IndexArray;
  GLuint m_NormalArray;
  GLuint m_TextureVertexArray;

  GLuint bufferObjects[3];
  QGLShaderProgram program;
  bool dataLoaded;

  ~Pimpl() {
    glDeleteBuffers(3, bufferObjects);
  }

  void clear() {
    delete [] normals;
    normals = 0;
    delete [] points;
    points = 0;
    delete [] indexes;
    indexes = 0;
  }

  void init() {
    initializeGLFunctions();
    glGenBuffers(3, bufferObjects);
  }

  void setData() 
  {
    // std::cout<<vertexCount<<"*********"<<indexCount<<std::endl;
    glBindBuffer(GL_ARRAY_BUFFER, bufferObjects[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertexCount * 3, points, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, bufferObjects[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertexCount * 3, normals, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferObjects[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * indexCount, indexes, GL_STATIC_DRAW);
  }

  void drawData() {
    if (dataLoaded) {
      glPushMatrix();
      glColor3i(244, 164, 96);
      glScalef(zoom, zoom, zoom);
      glRotatef(270.0f, 1.0f, 0.0f, 0.0f);
      glRotatef(zRot, 0.0f, 0.0f, 1.0f);
      glTranslated(-centerX, -centerY, -centerZ);
      
      glEnableClientState(GL_VERTEX_ARRAY);
      glEnableClientState(GL_NORMAL_ARRAY);
      
      glBindBuffer(GL_ARRAY_BUFFER, bufferObjects[0]);
      glVertexPointer(3, GL_FLOAT, 0, 0);

      glBindBuffer(GL_ARRAY_BUFFER, bufferObjects[1]);
      glNormalPointer(GL_FLOAT, 0, 0);

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferObjects[2]);
      glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_SHORT, 0);

      // glDisableClientState(GL_VERTEX_ARRAY);
      // glDisableClientState(GL_NORMAL_ARRAY);
      glPopMatrix();
    }
  }
};

D3Widget::D3Widget(QWidget *parent)
  : QGLWidget(QGLFormat(QGL::DoubleBuffer), parent)
  , _pimpl(new Pimpl()) {}

D3Widget::~D3Widget() {
  delete [] _pimpl->normals;
  delete [] _pimpl->points;
  delete [] _pimpl->indexes;
}

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
  initializeGLFunctions();

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

  _pimpl->init();
}

void D3Widget::paintGL()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  _pimpl->drawData();
  
  glFlush(); /* clear buffers */
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

  _pimpl->clear();

  _pimpl->dataLoaded = false;

  if (data.empty()) return;

  const unsigned int dataSize = data.size() * 3;
  _pimpl->indexes = new GLushort[dataSize];
  _pimpl->points = new GLfloat[dataSize * 3];
  _pimpl->normals = new GLfloat[dataSize * 3];

  _pimpl->indexCount = 0;
  _pimpl->vertexCount = 0;
  for (auto triangle : data) {
    const std::vector<float> normal = triangle->normal();
    const std::vector<Vertex> vertices = triangle->vertices();
    for (auto vertex : vertices) {
      _pimpl->points[_pimpl->vertexCount] = vertex.x();
      _pimpl->points[_pimpl->vertexCount + 1] = vertex.y();
      _pimpl->points[_pimpl->vertexCount + 2] = vertex.z();

      _pimpl->normals[_pimpl->vertexCount] = normal.at(0);
      _pimpl->normals[_pimpl->vertexCount + 1] = normal.at(1);
      _pimpl->normals[_pimpl->vertexCount + 2] = normal.at(2);

      if (vertex.x() < _pimpl->minX) { _pimpl->minX = vertex.x(); }
      if (vertex.x() > _pimpl->maxX) { _pimpl->maxX = vertex.x(); }
      if (vertex.y() < _pimpl->minY) { _pimpl->minY = vertex.y(); }
      if (vertex.y() > _pimpl->maxY) { _pimpl->maxY = vertex.y(); }
      if (vertex.z() < _pimpl->minZ) { _pimpl->minZ = vertex.z(); }
      if (vertex.z() > _pimpl->maxZ) { _pimpl->maxZ = vertex.z(); }
    }

    _pimpl->indexes[_pimpl->indexCount] = _pimpl->vertexCount;
    _pimpl->vertexCount += 3;
    _pimpl->indexCount += 1;
  }
  _pimpl->centerX = (_pimpl->minX + _pimpl->maxX) / 2;
  _pimpl->centerY = (_pimpl->minY + _pimpl->maxY) / 2;
  _pimpl->centerZ = (_pimpl->minZ + _pimpl->maxZ) / 2;
  cout <<"Input "<<data.size()<<" triangles"<<endl;
  cout <<_pimpl->minX<<" "<<_pimpl->centerX<<" "<<_pimpl->maxX<<endl;
  cout <<_pimpl->minY<<" "<<_pimpl->centerY<<" "<<_pimpl->maxY<<endl;
  cout <<_pimpl->minZ<<" "<<_pimpl->centerZ<<" "<<_pimpl->maxZ<<endl;

  _pimpl->setData();
  _pimpl->dataLoaded = true;

  // _pimpl->clear();
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
    if(_pimpl->dataLoaded) updateGL();
  }
}

void D3Widget::mouseReleaseEvent(QMouseEvent *event) {
  if (_pimpl->rotFlag && (event->buttons() & Qt::LeftButton)) {
    _pimpl->rotFlag = false;
  }
}
