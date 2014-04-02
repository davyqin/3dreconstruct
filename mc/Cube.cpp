#include "Cube.h"
#include "Vertex.h"

using namespace std;

class Cube::Pimpl
{
public:
  Pimpl() {}

  /* data */
  vector<Vertex> vertices;
};

Cube::Cube()
:_pimpl(new Pimpl()) {}

Cube::~Cube() {}
