#ifndef IMAGESTACK_H
#define IMAGESTACK_H

#include <boost/shared_ptr.hpp>
#include <memory>
#include <string>

class Image;

class ImageStack
{
public:
  ImageStack();

  ~ImageStack();

  void setImageFolder(const std::string& imageFolder);

  boost::shared_ptr<const Image> fetchImage(int index) const;

  int imageCount() const;

private:
  class Pimpl;
  std::unique_ptr<Pimpl> _pimpl;
};

#endif // IMAGESTACK_H
