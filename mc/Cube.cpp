#include "Cube.h"
#include "Vertex.h"

#include <exception>

using namespace std;

class Cube::Pimpl
{
public:
  Pimpl() {}

  Pimpl(const std::vector<Vertex>& vertices)
  :vertices(vertices) {}

  /* data */
  vector<Vertex> vertices;
};

Cube::Cube()
:_pimpl(new Pimpl()) {}

Cube::Cube(const std::vector<Vertex>& vertices)
:_pimpl(new Pimpl(vertices)) {}

Cube::Cube(const Cube& other)
:_pimpl(new Pimpl(other.vertices())) {}

Cube& Cube::operator=(const Cube& other)
{
  if (this != &other) {
    _pimpl.reset(new Pimpl(other.vertices()));
  }

  return *this;
}

Cube::~Cube() {}

void Cube::setVertices(const std::vector<Vertex>& vertices) {
  _pimpl->vertices = vertices;
}

std::vector<Vertex> Cube::vertices() const {
  return _pimpl->vertices;
}

Vertex Cube::vertex(unsigned int index) const {
  if (index > 7) {
    throw std::runtime_error("wrong index of vertex");
  }

  return _pimpl->vertices.at(index);
}
