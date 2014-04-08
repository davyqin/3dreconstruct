#pragma once

#include "Cube.h"

#include <boost/shared_ptr.hpp>
#include <memory>
#include <vector>

class Cube;

/*This class describes a grid containing some cubes.*/
class Grid
{
public:
  Grid();

  Grid(const std::vector<Cube>& cubes);

  Grid(const Grid& other);

  ~Grid();

  void set3D(int xSize, int ySize, int zSize);

  int xSize() const;

  int ySize() const;

  int zSize() const;

  void setCubes(const std::vector<Cube>& cubes);

  std::vector<Cube> cubes() const;

private:
  class Pimpl;
  std::unique_ptr<Pimpl> _pimpl;
};
