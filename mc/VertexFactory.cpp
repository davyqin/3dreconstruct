#include "VertexFactory.h"
#include "Vertex.h"
#include "model/Image.h"

using namespace std;

class VertexFactory::Pimpl
{
public:
  Pimpl() {}

  Pimpl(boost::shared_ptr<const Image> image)
  :image(image) {}

  /* data */
  boost::shared_ptr<const Image> image;
};

VertexFactory::VertexFactory()
:_pimpl(new Pimpl()) {}

VertexFactory::VertexFactory(boost::shared_ptr<const Image> image)
:_pimpl(new Pimpl(image)) {}

VertexFactory::~VertexFactory() {}

void VertexFactory::setImage(boost::shared_ptr<const Image> image) {
  _pimpl->image = image;
}

vector<Vertex> VertexFactory::vertices() const {
  if (!_pimpl->image) {
    return vector<Vertex>();
  }

  const double xInc = _pimpl->image->pixelSpacing().at(0);
  const double yInc = _pimpl->image->pixelSpacing().at(1);
  const int cols = _pimpl->image->width();
  const int rows = _pimpl->image->height();
  const vector<double> originPosition = _pimpl->image->position();
  const Image::Orientation orientation = _pimpl->image->orientation();
  vector<Vertex> vertices;

  for (int i = 0; i < rows; ++i) {
  	for (int j = 0; j < cols; ++j) {
      const unsigned int index = i * rows + j;
      const double value = (double)(_pimpl->image->pixelValue(index));
      vector<double> position = originPosition;
  	  switch (orientation)
  	  {
  	  	case (Image::TRAN):
  	  	{
  	  	  position.at(0) += (j * xInc);
  	  	  position.at(1) += (i * yInc);
  	  	  break;
  	  	}
  	  	default:
  	  	{
  	  	  // Add codes later
  	  	  break;
  	  	}
  	  }
  	  vertices.push_back(Vertex(position.at(0), position.at(1), position.at(2), value));
  	}
  }
  return vertices;
}
