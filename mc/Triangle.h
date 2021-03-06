#pragma once

#include "model/Vertex.h"

#include <memory>
#include <vector>

class Vertex;

class Triangle
{
public:
  Triangle(const std::vector<boost::shared_ptr<const Vertex> >& vertices);

  Triangle(const Triangle& triangle);

  Triangle& operator=(const Triangle& triangle);

  ~Triangle();

  std::vector<boost::shared_ptr<const Vertex> > vertices() const;

  std::vector<float> normal() const;

private:
  class Pimpl;
  std::unique_ptr<Pimpl> _pimpl;
};
