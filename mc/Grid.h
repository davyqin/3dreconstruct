#pragma once

#include <boost/shared_ptr.hpp>
#include <memory>
#include <vector>

class Cube;

/*This class describes a grid containing some cubes.*/
class Grid
{
public:
  Grid(const std::vector<Cube>& cubes);

  Grid(const Grid& other);

  ~Grid();

  std::vector<Cube> cubes() const;

private:
  class Pimpl;
  std::unique_ptr<Pimpl> _pimpl;
};
