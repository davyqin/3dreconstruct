#pragma once

#include <boost/shared_ptr.hpp>
#include <memory>
#include <vector>

class Vertex;
/*This class describes a cube containing 8 vertices. The cube locates between 2 parallel images.
The vertices of each side contain the pixel values and positions on related image. 
If the orientation of image is transverse, the vertices are ordered as (x, y, z1), (x+1, y, z1), 
(x, y+1, z1), (x+1, y+1, z1), (x, y, z2), (x+1, y, z2), (x, y+1, z2), (x+1, y+1, z2) .*/
class Cube
{
public:

  Cube();

  Cube(const std::vector<Vertex>& vertices);

  Cube(const Cube& other);

  Cube& operator=(const Cube& other);

  ~Cube();

  void setVertices(const std::vector<Vertex>& vertices);

  std::vector<Vertex> vertices() const;

  Vertex vertex(unsigned int index) const;

private:
  class Pimpl;
  std::unique_ptr<Pimpl> _pimpl;
};
