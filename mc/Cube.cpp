#include "Cube.h"


class Cube::Pimpl
{
public:
  Pimpl() {}

  /* data */
};

Cube::Cube()
:_pimpl(new Pimpl()) {}

Cube::~Cube() {}
