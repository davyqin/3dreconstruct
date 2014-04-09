#include "CubeFactory.h"
#include "Cube.h"
#include "VertexFactory.h"
#include "Vertex.h"
#include "model/Image.h"

#include <boost/progress.hpp>

using namespace std;

class CubeFactory::Pimpl
{
public:
  Pimpl() {}

  /* data */
  vector<Cube> cubes;
  boost::shared_ptr<const Image> upImage;
  boost::shared_ptr<const Image> downImage;
};

CubeFactory::CubeFactory()
:_pimpl(new Pimpl()) {}

CubeFactory::~CubeFactory() {}

void CubeFactory::setImages(boost::shared_ptr<const Image> downsideImage, 
                            boost::shared_ptr<const Image> upsideImage) 
{
  _pimpl->downImage = downsideImage;
  _pimpl->upImage = upsideImage;
}

std::vector<Cube> CubeFactory::cubes() const {
  if (!_pimpl->downImage || !_pimpl->upImage) {
    return vector<Cube>();
  }
  // boost::progress_timer timer;
  VertexFactory vertexFactory(_pimpl->downImage);
  std::vector<Vertex> downVertices = vertexFactory.vertices();

  vertexFactory.setImage(_pimpl->upImage);
  std::vector<Vertex> upVertices = vertexFactory.vertices();

  if (downVertices.size() != upVertices.size()) {
    return vector<Cube>();
  }
  std::vector<Cube> cubes;

  const int cols = _pimpl->upImage->width();
  const int rows = _pimpl->upImage->height();
  const int step = 1; //31; //15; //7; //3;
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < cols; ++j) {
      const unsigned int index = i * rows + j;
      std::vector<Vertex> vertices;
      vertices.push_back(downVertices.at(index));
      vertices.push_back(downVertices.at(index + step));
      vertices.push_back(downVertices.at(index + rows + step));
      vertices.push_back(downVertices.at(index + rows));
      vertices.push_back(upVertices.at(index));
      vertices.push_back(upVertices.at(index + step));
      vertices.push_back(upVertices.at(index + rows + step));
      vertices.push_back(upVertices.at(index + rows));
      cubes.push_back(Cube(vertices));
      //++j;
      j += step;
    }
    // ++i;
    i += step;
  }

  return cubes;
}
