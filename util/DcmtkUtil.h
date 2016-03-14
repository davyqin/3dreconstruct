#pragma once

#include "DicomUtil.h"
#include "Exception.h"

#include <boost/shared_ptr.hpp>
#include <string>
#include <memory>

class Image;

class DcmtkUtilException : public Exception
{
public:
  DcmtkUtilException() {};
};

class DcmtkUtil : public DicomUtil
{
public:
  DcmtkUtil();

  DcmtkUtil(const std::string& fileName);

  ~DcmtkUtil();

  void setFileName(const std::string& filename);

  boost::shared_ptr<unsigned short> pixel();

  boost::shared_ptr<Image> fetchImage() const;

  int pixelLength() const;

  int imageHeight() const;

  int imageWidth() const;

  bool hasPixelData() const;

private:

  void readFile();

  class Pimpl;
  std::unique_ptr<Pimpl> _pimpl;
};

