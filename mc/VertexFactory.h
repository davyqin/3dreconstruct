#pragma once

#include <boost/shared_ptr.hpp>
#include <memory>
#include <vector>

class Image;
class Vertex;

class VertexFactory
{
public:

  VertexFactory();

  ~VertexFactory();

  void setImage(boost::shared_ptr<const Image> image);

  std::vector<Vertex> vertices() const;

private:
  class Pimpl;
  std::unique_ptr<Pimpl> _pimpl;
};
