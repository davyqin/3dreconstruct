#include <QtGui>
#include <QtOpenGL>
#include <QGLShaderProgram>
#include <QMatrix4x4>

#include "D3Widget.h"
#include "mc/Triangle.h"
#include <glm/glm.hpp>
// #include <glm/gtc/matrix_transform.hpp>
// #include <glm/gtx/transform2.hpp>

#include <boost/shared_ptr.hpp>
#include <vector>
#include <limits>


using namespace std;

namespace {
  const GLfloat ambient[4] = {1.0f, 0.0f, 0.0f, 1.0f};
  const GLfloat spec[4] = {1.0, 1.0, 1.0, 1.0};
  const GLfloat light_position[] = {-300.0f, -300.0f, 0.0f, 0.0f};
}

class D3Widget::Pimpl {
public:
  Pimpl()
  : qtDark(QColor::fromRgb(0,0,0))
  , zoom(1.0)
  , minX(std::numeric_limits<float>::max())
  , minY(std::numeric_limits<float>::max())
  , minZ(std::numeric_limits<float>::max())
  , maxX(-2000.0)
  , maxY(-2000.0)
  , maxZ(-2000.0)
  , xRot(270.0)
  , zRot(225.0)
  , rotFlag(false)
  , dataLoaded(false)
  // , view(glm::mat4(1.0))
  // , projection(glm::mat4(1.0))
  {}

  QColor qtDark;
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
  float xRot;
  float zRot;
  bool rotFlag;
  QPoint lastPoint;
  bool dataLoaded;

  GLuint* indexes;
  glm::vec3* points;
  glm::vec3* normals;
  int vertexCount;
  GLuint bufferObjects[3];

  QGLShaderProgram program;
  // glm::mat4 view;
  // glm::mat4 projection;
  QMatrix4x4 view;
  QMatrix4x4 projection;

  void initScene() {
    //view = glm::lookAt(glm::vec3(0.0f,0.0f,2.0f), glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0.0f,1.0f,0.0f));
    view.lookAt(QVector3D(0.0f,0.0f,2.0f), QVector3D(0.0f,0.0f,0.0f), QVector3D(0.0f,1.0f,0.0f));
    const glm::vec4 worldLight = glm::vec4(-300.0f, -300.0f, 0.0f, 1.0f);
    program.setUniformValue("Light.Position", QVector4D(worldLight[0], worldLight[1], worldLight[2], worldLight[3]));

    program.setUniformValue("Material.Kd", QVector3D(0.5f, 0.5f, 0.5f));
    program.setUniformValue("Light.Ld", QVector3D(1.0f, 1.0f, 1.0f));
    program.setUniformValue("Material.Ka", QVector3D(0.2f, 0.2f, 0.2f));
    program.setUniformValue("Light.La", QVector3D(1.0f, 0.0f, 0.0f));
    program.setUniformValue("Material.Ks", QVector3D(1.0f, 1.0f, 1.0f));
    program.setUniformValue("Light.Ls", QVector3D(1.0f, 1.0f, 1.0f));
    program.setUniformValue("Material.Shininess", 20.0f);
  }
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
  initializeGLFunctions();
  glGenBuffers(3, _pimpl->bufferObjects);
  qglClearColor(_pimpl->qtDark);
  initShaders();
  _pimpl->initScene(); 
  glEnable(GL_DEPTH_TEST);
}

void D3Widget::paintGL()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  if (!_pimpl->dataLoaded) return;

#if 0
  _pimpl->model = glm::mat4(1.0);
  _pimpl->model *= glm::scale(glm::mat4(1.0), glm::vec3(_pimpl->zoom, _pimpl->zoom, _pimpl->zoom));
  _pimpl->model *= glm::rotate(glm::mat4(1.0), _pimpl->xRot, glm::vec3(1.0f, 0.0f, 0.0f));
  _pimpl->model *= glm::rotate(glm::mat4(1.0), _pimpl->zRot, glm::vec3(0.0f, 0.0f, 1.0f));
  _pimpl->model *= glm::translate(glm::mat4(1.0), glm::vec3(-_pimpl->centerX, -_pimpl->centerY, -_pimpl->centerZ));
  const glm::mat4 mvp = _pimpl->proj * _pimpl->model;
  _pimpl->program.setUniformValue("MVP", QMatrix4x4(mvp[0][0], mvp[1][0], mvp[2][0], mvp[3][0],
                                                    mvp[0][1], mvp[1][1], mvp[2][1], mvp[3][1],
                                                    mvp[0][2], mvp[1][2], mvp[2][2], mvp[3][2],
                                                    mvp[0][3], mvp[1][3], mvp[2][3], mvp[3][3]));

  const glm::mat4 mv = _pimpl->view * _pimpl->model;
  _pimpl->program.setUniformValue("ModelViewMatrix", QMatrix4x4(mv[0][0], mv[1][0], mv[2][0], mv[3][0],
                                                                mv[0][1], mv[1][1], mv[2][1], mv[3][1],
                                                                mv[0][2], mv[1][2], mv[2][2], mv[3][2],
                                                                mv[0][3], mv[1][3], mv[2][3], mv[3][3]));


  const glm::mat3 nm = glm::mat3( glm::vec3(mv[0]), glm::vec3(mv[1]), glm::vec3(mv[2]) );
  _pimpl->program.setUniformValue("NormalMatrix", QMatrix3x3(nm[0][0], nm[1][0], nm[2][0],
                                                             nm[0][1], nm[1][1], nm[2][1],
                                                             nm[0][2], nm[1][2], nm[2][2]));
#endif

  QMatrix4x4 model;
  model.scale(_pimpl->zoom);
  model.rotate(_pimpl->xRot, 1.0f, 0.0f, 0.0f);
  model.rotate(_pimpl->zRot, 0.0f, 0.0f, 1.0f);
  model.translate(-_pimpl->centerX, -_pimpl->centerY, -_pimpl->centerZ);
  _pimpl->program.setUniformValue("MVP", _pimpl->projection * model);

  const QMatrix4x4 mv = _pimpl->view * model;
  _pimpl->program.setUniformValue("ModelViewMatrix", mv);

  _pimpl->program.setUniformValue("NormalMatrix", mv.normalMatrix());

  glBindBuffer(GL_ARRAY_BUFFER, _pimpl->bufferObjects[0]);
  int vertexLocation = _pimpl->program.attributeLocation("VertexPosition");
  _pimpl->program.enableAttributeArray(vertexLocation);
  glVertexAttribPointer(vertexLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);

  glBindBuffer(GL_ARRAY_BUFFER, _pimpl->bufferObjects[1]);
  int normalLocation = _pimpl->program.attributeLocation("VertexNormal");
  _pimpl->program.enableAttributeArray(normalLocation);
  glVertexAttribPointer(normalLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _pimpl->bufferObjects[2]);
  glDrawElements(GL_TRIANGLES, _pimpl->vertexCount, GL_UNSIGNED_INT, 0); 
}

void D3Widget::resizeGL(int width, int height)
{
  const int side = qMin(width, height);
  glViewport((width - side) / 2, (height - side) / 2, side, side);

#if 0
  _pimpl->projection = glm::mat4(1.0);
  _pimpl->projection = glm::ortho(-300.0f, 300.0f, -300.0f, 300.0f, -300.0f, 300.0f);
#endif  

  _pimpl->projection.setToIdentity();
  _pimpl->projection.ortho(-300.0f, 300.0f, -300.0f, 300.0f, -300.0f, 300.0f);
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
  _pimpl->dataLoaded = false;
  if (data.empty()) return;

  const unsigned int dataSize = data.size() * 3;
  _pimpl->indexes = new GLuint[dataSize];
  _pimpl->points = new glm::vec3[dataSize];
  _pimpl->normals = new glm::vec3[dataSize];

  _pimpl->vertexCount = 0;
  for (auto triangle : data) {
    const std::vector<float> normal = triangle->normal();
    const std::vector<Vertex> vertices = triangle->vertices();
    for (auto vertex : vertices) {
      _pimpl->points[_pimpl->vertexCount] = glm::vec3(vertex.x(), vertex.y(), vertex.z());
      _pimpl->normals[_pimpl->vertexCount] = glm::vec3(normal.at(0), normal.at(1), normal.at(2));
      _pimpl->indexes[_pimpl->vertexCount] = _pimpl->vertexCount;
      _pimpl->vertexCount += 1;

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
  
  glBindBuffer(GL_ARRAY_BUFFER, _pimpl->bufferObjects[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * _pimpl->vertexCount * 3, _pimpl->points, GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, _pimpl->bufferObjects[1]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * _pimpl->vertexCount * 3, _pimpl->normals, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _pimpl->bufferObjects[2]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * _pimpl->vertexCount, _pimpl->indexes, GL_STATIC_DRAW);

  delete [] _pimpl->normals;
  _pimpl->normals = 0;
  delete [] _pimpl->points;
  _pimpl->points = 0;
  delete [] _pimpl->indexes;
  _pimpl->indexes = 0;

  _pimpl->dataLoaded = true;
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

    if (newPoint.y() < _pimpl->lastPoint.y())
      _pimpl->xRot -= 1;
    else
      _pimpl->xRot += 1;

    if (_pimpl->xRot < 0) _pimpl->xRot = 360;
    if (_pimpl->xRot > 360) _pimpl->xRot = 0;

    if (_pimpl->dataLoaded) updateGL();
  }
}

void D3Widget::mouseReleaseEvent(QMouseEvent *event) {
  if (_pimpl->rotFlag && (event->buttons() & Qt::LeftButton)) {
    _pimpl->rotFlag = false;
  }
}

void D3Widget::initShaders() {
  // Override system locale until shaders are compiled
  setlocale(LC_NUMERIC, "C");

  // Compile vertex shader
  if (!_pimpl->program.addShaderFromSourceFile(QGLShader::Vertex, ":/shader/basic.vs"))
    close();

  // Compile fragment shader
  if (!_pimpl->program.addShaderFromSourceFile(QGLShader::Fragment, ":/shader/basic.fs"))
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
