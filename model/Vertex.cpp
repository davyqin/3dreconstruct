#include "Vertex.h"

using namespace std;

class Vertex::Pimpl
{
public:

  Pimpl(const vector<float>& position,
  	    const int value)
  : position(position)
  , value(value) {}

  /* data */
  vector<float> position;
  const int value;
};

Vertex::Vertex()
:_pimpl(new Pimpl({0.0, 0.0, 0.0}, 0)) {}

Vertex::Vertex(float x, float y, float z, int value) 
:_pimpl(new Pimpl({x, y, z}, value)) {}

Vertex::Vertex(const Vertex& vertex)
:_pimpl(new Pimpl(vertex.position(), vertex.value())) {}

Vertex& Vertex::operator=(const Vertex& vertex)
{
  if (this != &vertex) {
    _pimpl.reset(new Pimpl(vertex.position(), vertex.value()));
  }

  return *this;
}

Vertex::~Vertex() {}

float Vertex::x() const {
  return _pimpl->position.at(0);
}

float Vertex::y() const {
  return _pimpl->position.at(1);
}

float Vertex::z() const {
  return _pimpl->position.at(2);
}

std::vector<float> Vertex::position() const {
  return _pimpl->position;
}

int Vertex::value() const {
  return _pimpl->value;
}
