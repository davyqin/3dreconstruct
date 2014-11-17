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

  Pimpl(boost::shared_ptr<const Image> downImage, 
        boost::shared_ptr<const Image> upImage)
  :downImage(downImage)
  ,upImage(upImage) {}

  /* data */
  boost::shared_ptr<const Image> downImage;
  boost::shared_ptr<const Image> upImage;
};

CubeFactory::CubeFactory()
:_pimpl(new Pimpl()) {}

CubeFactory::CubeFactory(boost::shared_ptr<const Image> downImage, 
                         boost::shared_ptr<const Image> upImage)
:_pimpl(new Pimpl(downImage, upImage)) {}

CubeFactory::~CubeFactory() {}

CubeFactory::CubeFactory(const CubeFactory& other)
:_pimpl(new Pimpl(other.downImage(), other.upImage())) {}


void CubeFactory::setImages(boost::shared_ptr<const Image> downImage, 
                            boost::shared_ptr<const Image> upImage) 
{
  _pimpl->downImage = downImage;
  _pimpl->upImage = upImage;
}

boost::shared_ptr<const Image> CubeFactory::downImage() const {
  return _pimpl->downImage;
}

boost::shared_ptr<const Image> CubeFactory::upImage() const {
  return _pimpl->upImage;
}

std::vector<boost::shared_ptr<Cube> > CubeFactory::cubes() const {
  if (!_pimpl->downImage || !_pimpl->upImage) {
    return  std::vector<boost::shared_ptr<Cube> >();
  }
#if 1
  std::vector<boost::shared_ptr<const Vertex> > downVertices = std::move(_pimpl->downImage->vertices());
  std::vector<boost::shared_ptr<const Vertex> > upVertices = std::move(_pimpl->upImage->vertices());

  if (downVertices.size() != upVertices.size()) {
    return  std::vector<boost::shared_ptr<Cube> >();
  }

  std::vector<boost::shared_ptr<Cube> > cubes;
  const int cols = _pimpl->upImage->width() / _pimpl->upImage->sampleStep();
  const int rows = _pimpl->upImage->height() / _pimpl->upImage->sampleStep();
  const int step = 1;
  for (int i = 0; i < (rows - step); i += step) {
    for (int j = 0; j < (cols - step); j += step) {
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
    }
  }
#endif
#if 0
  boost::shared_ptr<float> downXPos = _pimpl->downImage->xPos();
  boost::shared_ptr<float> downYPos = _pimpl->downImage->yPos();
  double downZPos = _pimpl->downImage->position().at(2);
  boost::shared_ptr<const unsigned char> downValue = _pimpl->downImage->rawPixelData8bit();

  boost::shared_ptr<const float> upXPos = _pimpl->upImage->xPos();
  boost::shared_ptr<const float> upYPos = _pimpl->upImage->yPos();
  double upZPos = _pimpl->upImage->position().at(2);
  boost::shared_ptr<const unsigned char> upValue = _pimpl->upImage->rawPixelData8bit();

  std::vector<boost::shared_ptr<Cube> > cubes;
  
  const int cols = _pimpl->upImage->width() / _pimpl->upImage->sampleStep();
  const int rows = _pimpl->upImage->height() / _pimpl->upImage->sampleStep();
  const int step = 1;
  for (int i = 0; i < (rows - step); i += step) {
    for (int j = 0; j < (cols - step); j += step) {
      const unsigned int index = i * rows + j;

      boost::shared_ptr<Cube> cube(new Cube());
      cube->addVertex(boost::shared_ptr<const Vertex>(new Vertex(*(downXPos.get() + index), *(downYPos.get() + index), downZPos, *(downValue.get() + index))));
      cube->addVertex(boost::shared_ptr<const Vertex>(new Vertex(*(downXPos.get() + index + step), *(downYPos.get() + index + step), downZPos, *(downValue.get() + index + step))));
      cube->addVertex(boost::shared_ptr<const Vertex>(new Vertex(*(downXPos.get() + index + step + rows), *(downYPos.get() + index + step + rows), downZPos, *(downValue.get() + index + step + rows))));
      cube->addVertex(boost::shared_ptr<const Vertex>(new Vertex(*(downXPos.get() + index + rows), *(downYPos.get() + index + rows), downZPos, *(downValue.get() + index + rows))));

      cube->addVertex(boost::shared_ptr<const Vertex>(new Vertex(*(upXPos.get() + index), *(upYPos.get() + index), upZPos, *(upValue.get() + index))));
      cube->addVertex(boost::shared_ptr<const Vertex>(new Vertex(*(upXPos.get() + index + step), *(upYPos.get() + index + step), upZPos, *(upValue.get() + index + step))));
      cube->addVertex(boost::shared_ptr<const Vertex>(new Vertex(*(upXPos.get() + index + step + rows), *(upYPos.get() + index + step + rows), upZPos, *(upValue.get() + index + step + rows))));
      cube->addVertex(boost::shared_ptr<const Vertex>(new Vertex(*(upXPos.get() + index + rows), *(upYPos.get() + index + rows), upZPos, *(upValue.get() + index + rows))));
      cubes.push_back(cube);
    }
  }
#endif
  return cubes;
}
