#include "Triangle.h"

// #include <cmath>
#include <exception>

#define GLM_FORCE_CXX11
#include <glm/glm.hpp>
#include <glm/gtx/normal.hpp>

using namespace std;

class Triangle::Pimpl
{
public:

  Pimpl(const std::vector<Vertex>& vertices)
  : vertices(vertices)
  , normal(0.0f)
  { 
    calcNormal();
  }

  /* data */
  vector<Vertex> vertices;
  glm::vec3 normal;

  void calcNormal() {
    if (vertices.size() != 3) {
      throw std::runtime_error("The number of triangle vertices is not equal to 3.");
    }

    glm::vec3 point0(vertices.at(0).x(), vertices.at(0).y(), vertices.at(0).z());
    glm::vec3 point1(vertices.at(1).x(), vertices.at(1).y(), vertices.at(1).z());
    glm::vec3 point2(vertices.at(2).x(), vertices.at(2).y(), vertices.at(2).z());

    normal = glm::triangleNormal(point0, point1, point2);
  }
};

Triangle::Triangle(const std::vector<Vertex> vertices) 
:_pimpl(new Pimpl(vertices)) {}

Triangle::Triangle(const Triangle& triangle)
:_pimpl(new Pimpl(triangle.vertices())) {}

Triangle& Triangle::operator=(const Triangle& triangle) {
  if (this != &triangle) {
    _pimpl.reset(new Pimpl(triangle.vertices()));
  }

  return *this;
}

Triangle::~Triangle() {}

std::vector<Vertex> Triangle::vertices() const {
  return _pimpl->vertices;
}

std::vector<float> Triangle::normal() const {
  return std::vector<float>({_pimpl->normal[0], _pimpl->normal[1], _pimpl->normal[2]});
}
