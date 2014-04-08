#include "Vertex.h"

using namespace std;

class Vertex::Pimpl
{
public:

  Pimpl(const vector<double>& position,
  	    const double value)
  : position(position)
  , value(value) {}

  /* data */
  vector<double> position;
  double value;
};

Vertex::Vertex()
:_pimpl(new Pimpl({0.0, 0.0, 0.0}, 0.0)) {}

Vertex::Vertex(double x, double y, double z, double value) 
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

double Vertex::x() const {
  return _pimpl->position.at(0);
}

double Vertex::y() const {
  return _pimpl->position.at(1);
}

double Vertex::z() const {
  return _pimpl->position.at(2);
}

void Vertex::setPosition(double x, double y, double z) {
  _pimpl->position = {x, y, z};
}

std::vector<double> Vertex::position() const {
  return _pimpl->position;
}

double Vertex::value() const {
  return _pimpl->value;
}
