#include "VertexFactory.h"
#include "Vertex.h"
#include "model/Image.h"

using namespace std;

class VertexFactory::Pimpl
{
public:
  Pimpl() {}

  /* data */
  boost::shared_ptr<Image> image;
};

VertexFactory::VertexFactory()
:_pimpl(new Pimpl()) {}

VertexFactory::~VertexFactory() {}

void VertexFactory::setImage(boost::shared_ptr<Image> image) {
  _pimpl->image = image;
}

vector<Vertex> VertexFactory::vertices() const {
  if (!_pimpl->image) {
    return vector<Vertex>();
  }

  vector<Vertex> vertices;

  const double xInc = _pimpl->image->pixelSpacing().at(0);
  const double yInc = _pimpl->image->pixelSpacing().at(1);
  const int cols = _pimpl->image->width();
  const int rows = _pimpl->image->height();
  const vector<double> originPosition = _pimpl->image->position();
  const Image::Orientation orientation = _pimpl->image->orientation();

  for (int i = 0; i < rows; ++i) {
  	for (int j = 0; j < cols; ++j) {
      const double value = _pimpl->image->pixelValue(i * rows + j);
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
  	  vertices.push_back(Vertex(position, value));
  	}
  }
  return vertices;
}
