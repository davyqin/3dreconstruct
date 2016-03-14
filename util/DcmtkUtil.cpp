#include "DcmtkUtil.h"

#include "model/Image.h"

#include <dcmtk/config/osconfig.h>
#include <dcmtk/dcmdata/dcfilefo.h>  /* for DcmFileFormat */
#include <dcmtk/dcmdata/dcdeftag.h>  /* for DCM_ defines */

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include <vector>

namespace {

std::vector<double> convertDicomValue(const std::string& dicomValue) {
  std::vector<std::string> splittedStrings;
  boost::split(splittedStrings, dicomValue, boost::is_any_of("\\"));

  std::vector<double> result;
  for (auto item : splittedStrings) {
    result.push_back(atof(item.c_str()));
  }

  return result;
}

template <class T>
T readDicomValue(const DcmTagKey& tagKey, DcmDataset* srcDset)
{
  OFString value;
  OFCondition cond = srcDset->findAndGetOFString(tagKey, value);
  if (cond.bad())
    BOOST_THROW_EXCEPTION(DcmtkUtilException());

  return boost::lexical_cast<T>(value.data());
}

std::vector<double> readDicomValueArray(const DcmTagKey& tagKey, DcmDataset* srcDset)
{
  OFString value;
  OFCondition cond = srcDset->findAndGetOFStringArray(tagKey, value);
  if (cond.bad())
    BOOST_THROW_EXCEPTION(DcmtkUtilException());

  return convertDicomValue(std::string(value.data()));
}

boost::shared_ptr<unsigned char> 
UpSideDown(boost::shared_ptr<unsigned char> pixelData, int nCols, int nRows, int nBytesP, int pixelLength)
{
  const int nRowBytes = nCols * nBytesP;
  unsigned char* cc= pixelData.get() + (nRows - 1) * nRowBytes;
  boost::shared_ptr<unsigned char> pNewData(new unsigned char [pixelLength + 16]);
  unsigned char* dd = pNewData.get();
  for (int i = 0; i < nRows; ++i)
  {
    for(int j = 0; j < nRowBytes; ++j)
    {
      *(dd++)=*(cc+j);
    }
    cc -= nRowBytes;
  }
  return pNewData;
}

/**
* Converts an unsigned or signed value to an unsigned long by adding
* the constant @a makePos.
*
* @param value - the value to convert
* @param mask - mask for the bits containing the value to convert from
*   @a value
* @param signMask - mask for the sign bit (if any) in @a value
* @param makePos - added to @a value to make it positive
* @return the converted value.
*/
template <class T>
unsigned long convertToUnsigned(T value,
  unsigned long mask,
  unsigned long signMask,
  unsigned long makePos) {

  if (value & signMask) { // if negative
    // (((value ^ mask) & mask) + 1) gives the 2 complement to value
    // i.e. (- value)
    return (makePos - (((value ^ mask) & mask) + 1));
  }
  else {
    return (makePos + (value & mask));
  }
}

}

class DcmtkUtil::Pimpl {
public:
  Pimpl()
    :nCols(0), nRows(0), nBytesP(0), nFrameSize(0), nNumFrames(1), nHighBit(0), bIsSigned(false), nBitsAllocated(0)
    , fWindowCenter(-1.0), fWindowWidth(-1.0), fRescaleSlope(1.0), fRescaleIntercept(0), nLength(0), sliceThickness(0.0) {}

  /* data */
  int nCols;
  int nRows;
  int nBytesP;
  int nFrameSize;
  int nNumFrames;
  int nHighBit;
  bool bIsSigned;
  int nBitsAllocated;
  double fWindowCenter;
  double fWindowWidth;
  double fRescaleSlope;
  double fRescaleIntercept;
  unsigned long nLength;
  std::vector<double> pixelSpacing;
  std::vector<double> imagePosition;
  std::vector<double> imageOrientation;
  boost::shared_ptr<unsigned char> pixelData;
  boost::shared_ptr<unsigned short> pixelData16;
  boost::shared_ptr<unsigned char> pixelData8;
  std::string fileName;
  double sliceThickness;

  void imageAdjuestment() {

    // 1. Clip the high bits.
    if (nHighBit < 15)
    {
      short* pp = (short *)pixelData.get();
      short nMask;
      short nSignBit;

      int nCount = nLength / 2;

      if(bIsSigned == 0 ) // Unsigned integer
      {
        nMask = 0xffff << (nHighBit + 1);
        while( nCount-- > 0 )
          *(pp ++) &= ~nMask;
      }
      else
      {
        // 1's complement representation
        nSignBit = 1 << nHighBit;
        nMask = 0xffff << (nHighBit + 1);
        while( nCount -- > 0 )
        {
          if ((*pp & nSignBit) != 0)
            *(pp ++) |= nMask;
          else
            *(pp ++) &= ~nMask;
        }
      }
    }

    // 2. Rescale if needed (especially for CT)
    if ((fRescaleSlope != 1.0f) || (fRescaleIntercept != 0.0f))
    {
      float fValue;

      short* pp = (short*)pixelData.get();
      int nCount = nLength / 2;

      while( nCount-- > 0 )
      {
        fValue = (*pp) * fRescaleSlope + fRescaleIntercept;
        *pp ++ = (short)fValue;
      }
    }

    // 3. Apply window/level
    float fShift;
    float fSlope;
    if ((fWindowCenter > 0) || (fWindowWidth > 0))
    {
      // Since we have window level info, we will only map what are
      // within the Window.
      fShift = fWindowCenter - fWindowWidth / 2.0;
      fSlope = 255.0 / double(fWindowWidth);
    }
    else 
    {
      int minValue = 32767;
      int maxValue = -32768;
      short* p = (short*)pixelData.get();

      const int nCount = nLength / 2;
      for (int i = 0; i < nCount; ++i, ++p)
      {
        if (*p < minValue) minValue = *p;
        if (*p > maxValue) maxValue = *p;
      }

      if (maxValue != minValue)
        fSlope = 255.0f / (maxValue - minValue);
      else
        fSlope = 1.0f;

      fShift = minValue;
    }
    
    {
      pixelData8.reset(new unsigned char[nLength/2 + 8]);
      unsigned char* data8 = pixelData8.get();
      int nCount = nLength / 2;
      short* pp = (short*)pixelData.get();

      while (nCount-- > 0)
      {
        float fValue = ((*pp ++) - fShift) * fSlope;
        if (fValue < 0)
          fValue = 0;
        else if (fValue > 255)
          fValue = 255;

        *data8++ = (unsigned char)fValue;
      }
    }

    // 3. Convert to unsigned short(16bit)
    unsigned long mask, signMask, makePos;
    mask = 0xffff;
    if (bIsSigned == 1) 
    {    
      /* in this case, Signed values must be taken in account */
      signMask = 1 << nHighBit; /* Mask to find sign */
      makePos = 1 << (nBitsAllocated-1); /* Addition to make value positive */
    }
    else 
    { /* Unsigned values */
      signMask = 0;
      makePos = 0;
    }

    unsigned short pixelSize = 0;
    if ( nBitsAllocated <= 8 )
      pixelSize = 1;
    else if ( nBitsAllocated <= 16 )
      pixelSize = 2;

    pixelData16.reset(new unsigned short[nLength/2 + 16]);
    unsigned short* data16 = pixelData16.get();
    unsigned char* data8 = pixelData8.get();
    const unsigned char* p = (unsigned char*)pixelData.get();
    int nCount = nLength / 2;

    switch (nBitsAllocated) 
    {
      case 8: // 8 bit images
      {
        for (int i = 0; i < nCount; ++i) 
        {
          unsigned char value = p[0];
          p += pixelSize;
          data16[i] = convertToUnsigned(value, mask, signMask, makePos);
          data8[i] = value;
        }
        break;
      }
      case 16: // 16 bit images
      {
        for (int i = 0; i < nCount; ++i) 
        {
          unsigned short value = (p[0] | p[1]<<8);
          p += pixelSize;
          data16[i] = convertToUnsigned(value, mask, signMask, makePos);
        }
        break;
      }
      default:
      {
        //throw InternalFailure("Invalid bits allocated value");
        break;
      }
    }
  }

 }; 


DcmtkUtil::DcmtkUtil()
:_pimpl(new Pimpl())
{
}

DcmtkUtil::DcmtkUtil(const std::string& fileName)
:_pimpl(new Pimpl())
{
  _pimpl->fileName = fileName;

  readFile();
}

DcmtkUtil::~DcmtkUtil()
{
}

boost::shared_ptr<Image> DcmtkUtil::fetchImage() const {
  if (!_pimpl->pixelData16 || !_pimpl->pixelData8) {
    return boost::shared_ptr<Image>();
  }

  boost::shared_ptr<Image> image(new Image(_pimpl->pixelData16, _pimpl->pixelData8, _pimpl->nLength/2));
  image->setPosition(_pimpl->imagePosition);
  image->setSize(_pimpl->nCols, _pimpl->nRows);
  image->setOrientation(_pimpl->imageOrientation);
  image->setPixelSpacing(_pimpl->pixelSpacing);
  return image;
}

boost::shared_ptr<unsigned short> DcmtkUtil::pixel() {
  if (_pimpl->fileName.empty()) {
    return boost::shared_ptr<unsigned short>();
  }

  if (!_pimpl->pixelData16) {
    readFile();
  }

  return _pimpl->pixelData16;
}

int DcmtkUtil::pixelLength() const {
  return _pimpl->nLength;
}

int DcmtkUtil::imageHeight() const {
  return _pimpl->nRows;
}

int DcmtkUtil::imageWidth() const {
  return _pimpl->nCols;
}

bool DcmtkUtil::hasPixelData() const {
  return (_pimpl->pixelData.get() != 0);
}

void DcmtkUtil::readFile()
{
  DcmFileFormat dcmff;

  // Open DICOM file to read patient/study/series information from
  OFCondition cond = dcmff.loadFile(_pimpl->fileName.c_str());
  if (cond.bad()) 
    return;

  DcmDataset* srcDset = dcmff.getDataset();
  if (srcDset == NULL) 
    return;

  //Slice Thickness DS
  _pimpl->sliceThickness = readDicomValue<double>(DCM_SliceThickness, srcDset);

  //Image Position (Patient) DS
  _pimpl->imagePosition = readDicomValueArray(DCM_ImagePositionPatient, srcDset);

  //Image Orientation (Patient) DS
  _pimpl->imageOrientation = readDicomValueArray(DCM_ImageOrientationPatient, srcDset);

  //Number of frames IS
  try 
  {
    _pimpl->nNumFrames = readDicomValue<int>(DCM_NumberOfFrames, srcDset);
  }
  catch (const DcmtkUtilException&)
  {
    _pimpl->nNumFrames = 1;
  }

  //Rows US
  _pimpl->nRows = readDicomValue<unsigned short>(DCM_Rows, srcDset);

  //Columns US
  _pimpl->nCols = readDicomValue<unsigned short>(DCM_Columns, srcDset);

  //Pixel Spacing DS
  _pimpl->pixelSpacing = readDicomValueArray(DCM_PixelSpacing, srcDset);
  _pimpl->pixelSpacing.push_back(_pimpl->sliceThickness);

  //Bits Allocated US
  _pimpl->nBitsAllocated = readDicomValue<unsigned short>(DCM_BitsAllocated, srcDset);

  //High Bit US
  _pimpl->nHighBit = readDicomValue<unsigned short>(DCM_HighBit, srcDset);

  //Pixel Representation US
  _pimpl->bIsSigned = readDicomValue<bool>(DCM_PixelRepresentation, srcDset);

  //Window Center DS
  try
  {
    _pimpl->fWindowCenter = readDicomValue<double>(DCM_WindowCenter, srcDset);
  }
  catch (const DcmtkUtilException&)
  {
    _pimpl->fWindowCenter = -1.0;
  }

  //Window Width DS
  try 
  {
    _pimpl->fWindowWidth = readDicomValue<double>(DCM_WindowWidth, srcDset);
  }
  catch (const DcmtkUtilException&)
  {
    _pimpl->fWindowWidth = -1.0;
  }

  //Rescale Intercept DS
  _pimpl->fRescaleIntercept = readDicomValue<double>(DCM_RescaleIntercept, srcDset);

  //Rescale Slope DS
  _pimpl->fRescaleSlope = readDicomValue<double>(DCM_RescaleSlope, srcDset);

  const int nSamplesPerPixel = readDicomValue<int>(DCM_SamplesPerPixel, srcDset);

  _pimpl->nBytesP = nSamplesPerPixel * _pimpl->nBitsAllocated / 8;
  _pimpl->nFrameSize = _pimpl->nCols * _pimpl->nRows * _pimpl->nBytesP;
  _pimpl->nLength = _pimpl->nNumFrames * _pimpl->nFrameSize;

  _pimpl->pixelData.reset(new unsigned char[_pimpl->nLength + 16]);

  DcmElement* element = NULL;
  srcDset->chooseRepresentation(EXS_LittleEndianExplicit, NULL);
  cond = srcDset->findAndGetElement(DCM_PixelData, element);
  if (cond.bad() || element == NULL)
    return;

  unsigned char* pImage = NULL;
  element->getUint8Array(pImage);

  memcpy(_pimpl->pixelData.get(), pImage, _pimpl->nLength);

  if (_pimpl->pixelData) // Have we got the pixel data?
  {
    // Need to do byte swap?
    // if (nDataEndian == BIG_ENDIAN_DATA && _pimpl->nBitsAllocated > 8) {
    //   SwapWord((char *)_pimpl->pixelData.get(), _pimpl->nLength / 2);
    // }

    _pimpl->pixelData = UpSideDown(_pimpl->pixelData, _pimpl->nCols, _pimpl->nRows, _pimpl->nBytesP, _pimpl->nLength);

    _pimpl->imageAdjuestment();
  }
}
