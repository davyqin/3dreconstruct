#pragma once

#include <boost/shared_ptr.hpp>
#include <memory>
#include <vector>

/*This class describes a vetex of an image. It contains the values and positions of 1 pixel on image. */
class Vertex
{
public:

  Vertex(const std::vector<double>& position,
  	     const double value);

  Vertex(const Vertex& vertex);

  ~Vertex();

  double x() const;

  double y() const;

  double z() const;

  std::vector<double> position() const;

  double value() const;

private:
  class Pimpl;
  std::unique_ptr<Pimpl> _pimpl;
};
