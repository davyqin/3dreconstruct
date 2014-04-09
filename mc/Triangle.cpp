#include "Triangle.h"

#include <cmath>
#include <exception>

using namespace std;

class Triangle::Pimpl
{
public:

  Pimpl(const std::vector<Vertex>& vertices)
  : vertices(vertices)
  , normals({0.0, 0.0, 0.0})
  { 
    calcNormal();
  }

  /* data */
  vector<Vertex> vertices;
  std::vector<double> normals;

  void calcNormal() {
    if (vertices.size() != 3) {
      throw std::runtime_error("The number of triangle vertices is not equal to 3.");
    }

    const double vi = vertices.at(1).x() - vertices.at(0).x();
    const double vj = vertices.at(1).y() - vertices.at(0).y();
    const double vk = vertices.at(1).z() - vertices.at(0).z();

    const double wi = vertices.at(2).x() - vertices.at(0).x();
    const double wj = vertices.at(2).y() - vertices.at(0).y();
    const double wk = vertices.at(2).z() - vertices.at(0).z();

    const double nr = sqrt((vj * wk - vk * wj) * (vj * wk - vk * wj) + 
                           (vk * wi - vi * wk) * (vk * wi - vi * wk) +
                           (vi * wj - vj * wi) * (vi * wj - vj * wi));
    normals.at(0) = 0 - (vj * wk - vk * wj) / nr;
    normals.at(1) = 0 - (vk * wi - vi * wk) / nr;
    normals.at(2) = 0 - (vi * wj - vj * wi) / nr;
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

std::vector<double> Triangle::normals() const {
  return _pimpl->normals;
}