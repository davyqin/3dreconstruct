#include "Image.h"

using namespace std;

class Image::Pimpl
{
public:
  Pimpl() {}


  /* data */
};

Image::Image()
:_pimpl(new Pimpl()) {}

Image::~Image() {}

