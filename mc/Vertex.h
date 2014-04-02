#pragma once

#include <boost/shared_ptr.hpp>
#include <memory>
#include <vector>

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
