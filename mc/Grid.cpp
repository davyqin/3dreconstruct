#include "Grid.h"
#include "Cube.h"

#include <vector>

using namespace std;

class Grid::Pimpl
{
public:
  Pimpl()
  : xSize(0), ySize(0), zSize(0) {}

  Pimpl(const std::vector<Cube>& cubes)
  :cubes(cubes), xSize(0), ySize(0), zSize(0) {}

  Pimpl(const std::vector<Cube>& cubes, int xSize, int ySize, int zSize)
  :cubes(cubes), xSize(xSize), ySize(ySize), zSize(zSize) {}

  /* data */
  vector<Cube> cubes;
  int xSize;
  int ySize;
  int zSize;
};

Grid::Grid()
:_pimpl(new Pimpl()) {}

Grid::Grid(const std::vector<Cube>& cubes)
:_pimpl(new Pimpl(cubes)) {}

Grid::Grid(const Grid& other)
:_pimpl(new Pimpl(other.cubes(), other.xSize(), other.ySize(), other.zSize())) {}

Grid::~Grid() {}  

void Grid::set3D(int xSize, int ySize, int zSize) {
  _pimpl->xSize = xSize;
  _pimpl->ySize = ySize;
  _pimpl->zSize = zSize;
}

int Grid::xSize() const {
  return _pimpl->xSize;
}

int Grid::ySize() const {
  return _pimpl->ySize;
}

int Grid::zSize() const {
  return _pimpl->zSize;
}

void Grid::setCubes(const std::vector<Cube>& cubes) {
  _pimpl->cubes = cubes;
}

std::vector<Cube> Grid::cubes() const {
  return _pimpl->cubes;
}
