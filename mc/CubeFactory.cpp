#include "CubeFactory.h"
#include "Cube.h"
#include "model/Vertex.h"
#include "model/Image.h"

#include <boost/progress.hpp>

using namespace std;

class CubeFactory::Pimpl
{
public:
  Pimpl() {}

  /* data */
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

 std::vector<boost::shared_ptr<const Cube> > CubeFactory::cubes() const {
  if (!_pimpl->downImage || !_pimpl->upImage) {
    return  std::vector<boost::shared_ptr<const Cube> >();
  }

  // boost::progress_timer timer;
  std::vector<boost::shared_ptr<const Vertex> > downVertices = _pimpl->downImage->vertices();

  std::vector<boost::shared_ptr<const Vertex> > upVertices = _pimpl->upImage->vertices();

  if (downVertices.size() != upVertices.size()) {
    return  std::vector<boost::shared_ptr<const Cube> >();
  }

  std::vector<boost::shared_ptr<const Cube> > cubes;
  const int cols = _pimpl->upImage->width() / 4;
  const int rows = _pimpl->upImage->height() / 4;
  const int step = 1; //1; //63 //31; //15; //7; //3;
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < cols; ++j) {
      const unsigned int index = i * rows + j;
      std::vector<boost::shared_ptr<const Vertex> >  vertices;
      vertices.push_back(downVertices.at(index));
      vertices.push_back(downVertices.at(index + step));
      vertices.push_back(downVertices.at(index + rows + step));
      vertices.push_back(downVertices.at(index + rows));
      vertices.push_back(upVertices.at(index));
      vertices.push_back(upVertices.at(index + step));
      vertices.push_back(upVertices.at(index + rows + step));
      vertices.push_back(upVertices.at(index + rows));
      cubes.push_back(boost::shared_ptr<Cube>(new Cube(vertices)));
      //++j;
      j += step;
    }
    // ++i;
    i += step;
  }

  return cubes;
}
