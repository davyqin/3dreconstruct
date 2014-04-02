#include "Grid.h"
#include "Cube.h"

using namespace std;

class Grid::Pimpl
{
public:
  Pimpl() {}

  /* data */
  vector<Cube> cubes;
};

Grid::Grid()
:_pimpl(new Pimpl()) {}

Grid::~Grid() {}
