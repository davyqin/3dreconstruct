#pragma once

#include <boost/shared_ptr.hpp>
#include <memory>
#include <vector>

/*This class describes a grid containing some cubes.*/
class Grid
{
public:

  Grid();

  ~Grid();

private:
  class Pimpl;
  std::unique_ptr<Pimpl> _pimpl;
};
