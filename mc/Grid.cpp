#include "Grid.h"
#include "Cube.h"

#include <vector>

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

Grid::Grid(const Grid& other)
:_pimpl(new Pimpl(other.cubes())) {}

Grid::~Grid() {}

std::vector<Cube> Grid::cubes() const {
  return _pimpl->cubes;
}
