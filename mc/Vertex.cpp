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

Vertex::Vertex(const vector<double>& position,
  	           const double value)
:_pimpl(new Pimpl(position, value)) {}

Vertex::Vertex(const Vertex& vertex)
:_pimpl(new Pimpl(vertex.position(), vertex.value())) {}

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

std::vector<double> Vertex::position() const {
  return _pimpl->position;
}

double Vertex::value() const {
  return _pimpl->value;
}
