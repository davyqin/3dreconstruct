#include "DcmtkUtil.h"

//#include "model/Image.h"

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
//  boost::shared_ptr<unsigned char> pixelData;
//  boost::shared_ptr<unsigned short> pixelData16;
//  boost::shared_ptr<unsigned char> pixelData8;
  std::string fileName;
  double sliceThickness;


  //void imageAdjuestment() {

  //  // 1. Clip the high bits.
  //  if (nHighBit < 15)
  //  {
  //    short* pp = (short *)pixelData.get();
  //    short nMask;
  //    short nSignBit;

  //    int nCount = nLength / 2;

  //    if (bIsSigned == 0) // Unsigned integer
  //    {
  //      nMask = 0xffff << (nHighBit + 1);
  //      while (nCount-- > 0)
  //        *(pp++) &= ~nMask;
  //    }
  //    else
  //    {
  //      // 1's complement representation
  //      nSignBit = 1 << nHighBit;
  //      nMask = 0xffff << (nHighBit + 1);
  //      while (nCount-- > 0)
  //      {
  //        if ((*pp & nSignBit) != 0)
  //          *(pp++) |= nMask;
  //        else
  //          *(pp++) &= ~nMask;
  //      }
  //    }
  //  }

  //  // 2. Rescale if needed (especially for CT)
  //  if ((fRescaleSlope != 1.0f) || (fRescaleIntercept != 0.0f))
  //  {
  //    float fValue;

  //    short* pp = (short*)pixelData.get();
  //    int nCount = nLength / 2;

  //    while (nCount-- > 0)
  //    {
  //      fValue = (*pp) * fRescaleSlope + fRescaleIntercept;
  //      *pp++ = (short)fValue;
  //    }
  //  }

  //  // 3. Apply window/level
  //  float fShift;
  //  float fSlope;
  //  if ((fWindowCenter > 0) || (fWindowWidth > 0))
  //  {
  //    // Since we have window level info, we will only map what are
  //    // within the Window.
  //    fShift = fWindowCenter - fWindowWidth / 2.0;
  //    fSlope = 255.0 / double(fWindowWidth);
  //  }
  //  else {
  //    int minValue = std::numeric_limits<short>::max();
  //    int maxValue = std::numeric_limits<short>::min();
  //    short* p = (short*)pixelData.get();

  //    const int nCount = nLength / 2;
  //    for (int i = 0; i < nCount; ++i, ++p)
  //    {
  //      if (*p < minValue) minValue = *p;
  //      if (*p > maxValue) maxValue = *p;
  //    }

  //    if (maxValue != minValue)
  //      fSlope = 255.0f / (maxValue - minValue);
  //    else
  //      fSlope = 1.0f;

  //    fShift = minValue;
  //  }

  //  {
  //    pixelData8.reset(new unsigned char[nLength / 2 + 8]);
  //    unsigned char* data8 = pixelData8.get();
  //    int nCount = nLength / 2;
  //    short* pp = (short*)pixelData.get();

  //    while (nCount-- > 0)
  //    {
  //      float fValue = ((*pp++) - fShift) * fSlope;
  //      if (fValue < 0)
  //        fValue = 0;
  //      else if (fValue > 255)
  //        fValue = 255;

  //      *data8++ = (unsigned char)fValue;
  //    }
  //  }

  //  // 3. Convert to unsigned short(16bit)
  //  unsigned long mask, signMask, makePos;
  //  mask = 0xffff;
  //  if (bIsSigned == 1)
  //  {
  //    /* in this case, Signed values must be taken in account */
  //    signMask = 1 << nHighBit; /* Mask to find sign */
  //    makePos = 1 << (nBitsAllocated - 1); /* Addition to make value positive */
  //  }
  //  else
  //  { /* Unsigned values */
  //    signMask = 0;
  //    makePos = 0;
  //  }

  //  unsigned short pixelSize = 0;
  //  if (nBitsAllocated <= 8)
  //    pixelSize = 1;
  //  else if (nBitsAllocated <= 16)
  //    pixelSize = 2;

  //  pixelData16.reset(new unsigned short[nLength / 2 + 16]);
  //  unsigned short* data16 = pixelData16.get();
  //  unsigned char* data8 = pixelData8.get();
  //  const unsigned char* p = (unsigned char*)pixelData.get();
  //  int nCount = nLength / 2;

  //  switch (nBitsAllocated)
  //  {
  //  case 8: // 8 bit images
  //  {
  //    for (int i = 0; i < nCount; ++i)
  //    {
  //      unsigned char value = p[0];
  //      p += pixelSize;
  //      data16[i] = convertToUnsigned(value, mask, signMask, makePos);
  //      data8[i] = value;
  //    }
  //    break;
  //  }
  //  case 16: // 16 bit images
  //  {
  //    for (int i = 0; i < nCount; ++i)
  //    {
  //      unsigned short value = (p[0] | p[1] << 8);
  //      p += pixelSize;
  //      data16[i] = convertToUnsigned(value, mask, signMask, makePos);
  //    }
  //    break;
  //  }
  //  default:
  //  {
  //    //throw InternalFailure("Invalid bits allocated value");
  //    break;
  //  }
  //  }
  //}
};


DcmtkUtil::DcmtkUtil()
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

void DcmtkUtil::readFile()
{
  DcmFileFormat dcmff;
  OFCondition cond;

  // Open DICOM file to read patient/study/series information from
  cond = dcmff.loadFile(_pimpl->fileName.c_str());
  if (cond.bad()) return;

  DcmDataset* srcDset = dcmff.getDataset();
  if (srcDset == NULL) return;

  OFString value;
  //cond =  srcDset->findAndGetOFString(DCM_PatientName, value);
  //if (cond.bad()) return;
  //value.clear();

  //Slice Thickness DS
  cond = srcDset->findAndGetOFString(DCM_SliceThickness, value);
  if (cond.bad()) return;
  _pimpl->sliceThickness = boost::lexical_cast<double>(value.data(), value.size());
  value.clear();

  //Image Position (Patient) DS
  cond = srcDset->findAndGetOFString(DCM_ImagePositionPatient, value);
  if (cond.bad()) return;
  _pimpl->imagePosition = convertDicomValue(std::string(value.data()));
  value.clear();
}
