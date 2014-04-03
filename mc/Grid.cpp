#include "Grid.h"
#include "Cube.h"

using namespace std;

class Grid::Pimpl
{
public:
  Pimpl(const std::vector<Cube>& cubes)
  :cubes(cubes) {}

  /* data */
  vector<Cube> cubes;
};

Grid::Grid(const std::vector<Cube>& cubes)
:_pimpl(new Pimpl(cubes)) {}

Grid::~Grid() {}
