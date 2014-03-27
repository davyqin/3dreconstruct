#include "DicomUtil.h"

#include <fstream>
#include <iostream>
#include <vector>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;

namespace {

  enum DATA_ENDIAN
  {
    LITTLE_ENDIAN_DATA,
    BIG_ENDIAN_DATA
  };

  enum COMPRESSION_MODE
  {
    COMPRESS_NONE = 0,
    COMPRESS_RLE,
    COMPRESS_JPEGLOSSY,
    COMPRESS_JPEGLOSSY12BIT,
    COMPRESS_JPEGLOSSLESS,
    COMPRESS_JPEGLOSSLESS2
  };

  void SwapWord(char *pArray, int nWords)
  {
    char *cc = pArray, c0;
    int i;

    // Swap every two bytes
    for (i = 0; i < nWords; i ++)
    {
      c0 = *cc;
      *cc = *(cc + 1);
      *(cc + 1)  = c0;

      cc += 2;
    }
  }

  void SwapDWord(char *pArray, int nDWords)
  {
    char *cc = pArray, c0;
    int i;

    // Rotate every four bytes
    for (i = 0; i < nDWords; i ++)
    {

      // Swap first and last bytes
      c0 = *cc;
      *cc = *(cc + 3);
      *(cc + 3)  = c0;

      // Swap middle two bytes
      c0 = *(cc + 2);
      *(cc + 2) = *(cc + 1);
      *(cc + 1)  = c0;

      cc += 4;
    }
  }

  int ReadUL(fstream& pcf, DATA_ENDIAN nDataEndian, bool /*bImplicitVR*/)
  {
    int lVal = 0;
    pcf.seekg(4, ios::cur);
    pcf.read((char *)&lVal, sizeof(int));
    if(nDataEndian == BIG_ENDIAN_DATA) {
      SwapDWord((char *) &lVal, 1);
    }
    return lVal;
  }

  long ReadLength(fstream& pcf, DATA_ENDIAN nDataEndian, bool bImplicitVR)
  {
    long int nValLength = 0;
    short int nsLength = 0;

    if (bImplicitVR)
    {
      pcf.read((char *)&nValLength, sizeof(int));
      if (nDataEndian == BIG_ENDIAN_DATA)
        SwapDWord((char *) &nValLength, 1);
    }
    else
    {
      pcf.seekg(2,ios::cur);
      pcf.read((char*)&nsLength, sizeof(short));
      if (nDataEndian == BIG_ENDIAN_DATA)
        SwapWord((char *) &nsLength, 1);
      nValLength = nsLength;
    }
    return nValLength;
  }

  int ReadString(fstream& pcf, char *pszStr, DATA_ENDIAN nDataEndian, bool bImplicitVR)
  {
    long int nValLength = 0;

    nValLength = ReadLength(pcf, nDataEndian, bImplicitVR);

    if ((nValLength > 80) || (nValLength < 0))
      return -1;

    pcf.read(pszStr, nValLength);
    pszStr[nValLength] = '\0';

    return 0;
  }

  std::string WriteToString(fstream& pcf, string* Text, string pszTitle, DATA_ENDIAN nDataEndian, bool bImplicitVR)
  {
    char szTemp[80];
    memset(szTemp, '0', 80);
    ReadString(pcf,szTemp,nDataEndian,bImplicitVR);
    *Text += pszTitle;
    *Text += szTemp;
    *Text += "\r\n";
    return string(szTemp);
  }

  float ReadDS(fstream& pcf, DATA_ENDIAN nDataEndian, bool bImplicitVR)
  {
    char szTemp[80]="";
    float fVal = 0;

    if (ReadString(pcf, szTemp, nDataEndian, bImplicitVR) == 0)
      sscanf(szTemp, "%f", &fVal);

    return fVal;
  }

  int ReadIS(fstream& pcf, DATA_ENDIAN nDataEndian, bool bImplicitVR)
  {
    char szTemp[80]="";
    int nVal = 0;
    if (ReadString(pcf, szTemp, nDataEndian, bImplicitVR) == 0)
      sscanf(szTemp, "%d", &nVal);

    return nVal;
  }

  int ReadUS(fstream& pcf, DATA_ENDIAN nDataEndian)
  {
    unsigned short nVal;
    pcf.seekg(4,ios::cur);
    pcf.read((char*)&nVal, sizeof(short)); // read the unsigned short value
    if (nDataEndian == BIG_ENDIAN_DATA)
      SwapWord((char *) &nVal, 1);

    return (int) nVal;
  }

  int WriteToString(string* Text, string pszTitle, string pszContent)
  {
    *Text += pszTitle;
    *Text += pszContent;
    *Text += "\r\n";
    return 1;
  }

  int WriteToString(string* Text, string pszTitle, int nContent)
  {
    *Text += pszTitle;
    *Text += boost::lexical_cast<string>(nContent);
    *Text += "\r\n";
    return 1;
  }

  boost::shared_ptr<unsigned char> UpSideDown(boost::shared_ptr<unsigned char> pixelData,
                                              int nCols, int nRows, int nBytesP, int pixelLength)
  {
    const int nRowBytes = nCols * nBytesP;
    unsigned char* cc= pixelData.get() + (nRows - 1) * nRowBytes;
    boost::shared_ptr<unsigned char> pNewData(new unsigned char [pixelLength + 16]);
    unsigned char* dd = pNewData.get();
    for (int i = 0; i < nRows; i++)
    {
      for(int j = 0; j < nRowBytes; j++)
      {
        *(dd++)=*(cc+j);
      }
      cc -= nRowBytes;
    }
    return pNewData;
  }

  std::vector<double> convertDicomValue(const std::string& dicomValue) {
    std::vector<std::string> splittedStrings;
    boost::split(splittedStrings, dicomValue, boost::is_any_of("\\"));

    std::vector<double> result;
    for (auto item : splittedStrings) {
      result.push_back(atof(item.c_str()));
    }

   return result;
 }

} // annonymouse namespace

class DicomUtil::Pimpl {
public:
  Pimpl()
  :nCols(0),nRows(0),nBytesP(0),nFrameSize(0),nNumFrames(1),nHighBit(0),bIsSigned(false),nBitsAllocated(0)
  ,fWindowCenter(0.0), fWindowWidth(0.0), fRescaleSlope(1.0), fRescaleIntercept(0),nLength(0) {}

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
  int nLength;
  std::vector<double> pixelSpacing;
  std::vector<double> imagePosition;
  boost::shared_ptr<unsigned char> pixelData;
  std::string fileName;

  void imageAdjuestment() {
    if (nBitsAllocated == 8) 
      return;

    short* pp = (short *)pixelData.get();

    // 1. Clip the high bits.
    if (nHighBit < 15)
    {
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

      pp = (short *)pixelData.get();
      int nCount = nLength / 2;

      while( nCount-- > 0 )
      {
        fValue = (*pp) * fRescaleSlope + fRescaleIntercept;
        *pp ++ = (short)fValue;
      }
    }
    
    // 3. Convert to 8bit image
    pp = (short *)pixelData.get();
    boost::shared_ptr<unsigned char> pNewData(new unsigned char[nLength/2 + 8]);
    unsigned char* np = pNewData.get();

    if ((fWindowCenter != 0) || (fWindowWidth != 0))
    {
      // Since we have window level info, we will only map what are within the Window.
      const float fShift = fWindowCenter - fWindowWidth / 2.0f;
      const float fSlope = 255.0f / fWindowWidth;

      int nCount = nLength / 2;

      while (nCount-- > 0)
      {
        short fValue = ((*pp ++) - fShift) * fSlope;
        if (fValue < 0) {
          fValue = 0;
        }
        else if (fValue > 255) {
          fValue = 255;
        }

        *np ++ = (unsigned char)fValue;
      }
    }
    else
    {
      // We will map the whole dynamic range.
      float fSlope = 1;;

      // First compute the min and max.
      int nCount = nLength / 2;
      int nMin = *pp;
      int nMax = *pp;
      while (nCount-- > 0)
      {
        if (*pp < nMin)
          nMin = *pp;

        if (*pp > nMax)
          nMax = *pp;

        pp ++;
      }

      // Calculate the scaling factor.
      if (nMax != nMin)
        fSlope = 255.0f / (nMax - nMin);
      else
        fSlope = 1.0f;

      nCount = nLength / 2;
      pp = (short *)pixelData.get();

      while (nCount-- > 0)
      {
        float fValue = ((*pp ++) - nMin) * fSlope;
        if (fValue < 0)
          fValue = 0;
        else if (fValue > 255)
          fValue = 255;

        *np ++ = (unsigned char) fValue;
      }
    }

    pixelData = pNewData;

    nBytesP = 1;
    nFrameSize /= 2;
    nLength /= 2;
  }
};

DicomUtil::DicomUtil()
  :_pimpl(new Pimpl())
{
}

DicomUtil::~DicomUtil() {}

void DicomUtil::setFileName(const string& filename) {
  _pimpl->fileName = filename;
}

boost::shared_ptr<unsigned char> DicomUtil::pixel() {
  if (_pimpl->fileName.empty()) {
    return boost::shared_ptr<unsigned char>();
  }

  if (!_pimpl->pixelData) {
    readImage();
  }

  return _pimpl->pixelData;
}

int DicomUtil::pixelLength() const {
  return _pimpl->nLength;
}

int DicomUtil::imageHeight() const {
  return _pimpl->nRows;
}

int DicomUtil::imageWidth() const {
  return _pimpl->nCols;
}

void DicomUtil::readImage()
{
  short int nSamplesPerPixel = 1;
  char szPhotometric[32] = "", szTransferSyntaxUID[80] = "";
  bool bImplicitVR = true;
  COMPRESSION_MODE nCompressionMode = COMPRESS_NONE;
  DATA_ENDIAN nDataEndian = LITTLE_ENDIAN_DATA;
  short int gTag, eTag;
  bool bPixelData = false;

  fstream fp;
  fp.open(_pimpl->fileName.c_str(), ios::in|ios::binary);
  if (fp.bad()) {
    return;
  }

  fp.seekg(132,ios_base::beg); //Skip dcm header
  string sHeader;

  while(!fp.eof()) {

    if (bPixelData) {
      break;
    }

    fp.read((char *)&gTag, sizeof(short));

    if (nDataEndian == BIG_ENDIAN_DATA) {
      SwapWord((char *) &gTag, 1);
    }

    fp.read((char *)&eTag, sizeof(short));
    if (nDataEndian == BIG_ENDIAN_DATA) {
      SwapWord((char *)&eTag, 1);
    }

    switch(gTag)
    {
    case 0x0002:
      {
        switch(eTag)
        {
        case 0x0000: //File Meta Elements Group Length UL
          {
            ReadUL(fp, nDataEndian, false);
            break;
          }
        case 0x0001: //File Meta Info Version OB
          {
            fp.seekg(10,ios::cur); //2 bytes reserved, 8 bytes unsigned int
            break;
          }
        case 0x0002: //Media Storage SOP Class UID UI
          {
            WriteToString(fp,&sHeader,"0002,0002 Media Storage SOP Class UID: ",nDataEndian,false);
            break;
          }
        case 0x0003: //Media Storage SOP Inst UID UI
          {
            WriteToString(fp,&sHeader,"0002,0003 Media Storage SOP Inst UID: ",nDataEndian,false);
            break;
          }
        case 0x0010: //Transfer Syntax UID UI
          {
            if (ReadString(fp, szTransferSyntaxUID, LITTLE_ENDIAN_DATA, false) != 0)
              break;

            if (string(szTransferSyntaxUID) == "1.2.840.10008.1.2.2") // Explicit VR Big Endian
              nDataEndian = BIG_ENDIAN_DATA; // Big Endian
            else 
              nDataEndian = LITTLE_ENDIAN_DATA; // Little Endian

            // Check if it is implicit VR or Explicit VR
            if (string(szTransferSyntaxUID) == "1.2.840.10008.1.2") // Implicit VR Little Endian
              bImplicitVR = true; // Implicit VR
            else
              bImplicitVR = false; // Explicit VR

            // Parse the encapsulation/compression
            if (string(szTransferSyntaxUID) == "1.2.840.10008.1.2.4.50") // JPEG lossy
              nCompressionMode = COMPRESS_JPEGLOSSY;
            else if (string(szTransferSyntaxUID) == "1.2.840.10008.1.2.4.51") // JPEG lossy 12bit
              nCompressionMode = COMPRESS_JPEGLOSSY12BIT;
            else if (string(szTransferSyntaxUID) == "1.2.840.10008.1.2.4.70") // JPEG lossless first order prediction
              nCompressionMode = COMPRESS_JPEGLOSSLESS;
            else if (string(szTransferSyntaxUID) == "1.2.840.10008.1.2.4.57") // JPEG lossless process 14
              nCompressionMode = COMPRESS_JPEGLOSSLESS2;
            else if (string(szTransferSyntaxUID) == "1.2.840.10008.1.2.5") // RLE
              nCompressionMode = COMPRESS_RLE;

            break;
          }
        case 0x0012: //Implementation Class UID UI
          {
            WriteToString(fp,&sHeader,"0002,0012 Implementation Class UID: ",nDataEndian,false);
            break;
          }
        case 0x0013: //Implementation Version Name SH
          {
            WriteToString(fp,&sHeader,"0002,0013 Implementation Version Name: ",nDataEndian,false);
            break;
          }
        case 0x0016: //Source App Entity Title AE
          {
            WriteToString(fp,&sHeader,"0002,0016 Source App Entity Title: ",nDataEndian,false);
            break;
          }
        default:
          {
            int nVal = ReadLength(fp,nDataEndian,false);
            fp.seekg(nVal,ios::cur);
            break;
          }
        }
        break;
      }
    case 0x0008:
      {
        switch(eTag)
        {
        case 0x0005: //Specific Character Set CS
          {
            WriteToString(fp,&sHeader,"0008,0005 Specific Character Set: ",nDataEndian,bImplicitVR);
            break;
          }
        case 0x0008: //Image Type CS
          {
            WriteToString(fp,&sHeader,"0008,0008 Image Type: ",nDataEndian,bImplicitVR);
            break;
          }
        case 0x0016: //SOP Class UID UI
          {
            WriteToString(fp,&sHeader,"0008,0016 SOP Class UID: ",nDataEndian,bImplicitVR);
            break;
          }
        case 0x0018: //SOP Instance UID UI
          {
            WriteToString(fp,&sHeader,"0008,0018 SOP Instance UID: ",nDataEndian,bImplicitVR);
            break;
          }
        case 0x0020: //Study Date DA
          {
            WriteToString(fp,&sHeader,"0008,0020 Study Date: ",nDataEndian,bImplicitVR);
            break;
          }
        case 0x0021: //Series Date DA
          {
            WriteToString(fp,&sHeader,"0008,0021 Series Date: ",nDataEndian,bImplicitVR);
            break;
          }
        case 0x0022: //Acquisition Date DA
          {
            WriteToString(fp,&sHeader,"0008,0022 Acquisition Date: ",nDataEndian,bImplicitVR);
            break;
          }
        case 0x0023: //Image Date DA
          {
            WriteToString(fp,&sHeader,"0008,0023 Image Date: ",nDataEndian,bImplicitVR);
            break;
          }
        case 0x0030: //Study Time TM
          {
            WriteToString(fp,&sHeader,"0008,0030 Study Time: ",nDataEndian,bImplicitVR);
            break;
          }
        case 0x0031: //Series Time TM
          {
            WriteToString(fp,&sHeader,"0008,0031 Series Time: ",nDataEndian,bImplicitVR);
            break;
          }
        case 0x0032: //Acquisition Time TM
          {
            WriteToString(fp,&sHeader,"0008,0032 Acquisition Time: ",nDataEndian,bImplicitVR);
            break;
          }
        case 0x0033: //Image Time TM
          {
            WriteToString(fp,&sHeader,"0008,0033 Image Time: ",nDataEndian,bImplicitVR);
            break;
          }
        case 0x0050: //Accession Number SH
          {
            WriteToString(fp,&sHeader,"0008,0050 Accession Number: ",nDataEndian,bImplicitVR);
            break;
          }
        case 0x0060: //Modality CS
          {
            WriteToString(fp,&sHeader,"0008,0060 Modality: ",nDataEndian,bImplicitVR);
            break;
          }
        case 0x0070: //Manufacturer LO
          {
            WriteToString(fp,&sHeader,"0008,0070 Manufacturer: ",nDataEndian,bImplicitVR);
            break;
          }
        default:
          {
            const int nVal = ReadLength(fp,nDataEndian,bImplicitVR);
            fp.seekg(nVal,ios::cur);
            break;
          }
        }
        break;
      }
    case 0x0010:
      {
        switch(eTag)
        {
        case 0x0010: //Patient's Name PN
          {
            WriteToString(fp,&sHeader,"0010,0010 Patient's Name: ",nDataEndian,bImplicitVR);
            break;
          }
        case 0x0020: //Patient ID PN
          {
            WriteToString(fp,&sHeader,"0010,0020 Patient ID: ",nDataEndian,bImplicitVR);
            break;
          }
        case 0x0030: //Patient Date of Birth DA
          {
            WriteToString(fp,&sHeader,"0010,0030 Patient Date of Birth: ",nDataEndian,bImplicitVR);
            break;
          }
        case 0x0040: //Patient Sex CS
          {
            WriteToString(fp,&sHeader,"0010,0040 Patient Sex: ",nDataEndian,bImplicitVR);
            break;
          }
        case 0x1010: //Patient Age AS
          {
            WriteToString(fp,&sHeader,"0010,1010 Patient Age: ",nDataEndian,bImplicitVR);
            break;
          }
        default:
          {
            int nVal = ReadLength(fp,nDataEndian,bImplicitVR);
            fp.seekg(nVal,ios::cur);
            break;
          }
        }
        break;
      }
    case 0x0018:
      {
        switch(eTag)
        {
        case 0x0015: //Body Part Examined CS
          {
            WriteToString(fp,&sHeader,"0018,0015 Body Part Examined: ",nDataEndian,bImplicitVR);
            break;
          }
        case 0x0050: //Slice Thickness DS
          {
            //                        string sTemp;
            //double nThickness =
            ReadDS(fp, nDataEndian, bImplicitVR);
            break;
          }
        case 0x0060: //KVP [Peak KV] DS
          {
            ReadDS(fp, nDataEndian, bImplicitVR);
            break;
          }
        case 0x0090: //Data collection diameter DS
          {
            ReadDS(fp, nDataEndian, bImplicitVR);
            break;
          }
        case 0x1000: //Device Serial Number LO
          {
            WriteToString(fp,&sHeader,"0018,1000 Device Serial Number: ",nDataEndian,bImplicitVR);
            break;
          }
        case 0x1020: //Software Version LO
          {
            WriteToString(fp,&sHeader,"0018,1020 Software Version: ",nDataEndian,bImplicitVR);
            break;
          }
        case 0x1030: //Protocol Name LO
          {
            WriteToString(fp,&sHeader,"0018,1030 Protocol Name: ",nDataEndian,bImplicitVR);
            break;
          }
        case 0x1050: //Spatial Resolution DS
          {
            ReadDS(fp, nDataEndian, bImplicitVR);
            break;
          }
        case 0x1110: //Distance Source to Detector [mm] DS
          {
            ReadDS(fp, nDataEndian, bImplicitVR);
            break;
          }
        case 0x1111: //Distance Source to Patient [mm] DS
          {
            ReadDS(fp, nDataEndian, bImplicitVR);
            break;
          }
        case 0x1120: //Gantry/Detector Tilt DS
          {
            ReadDS(fp, nDataEndian, bImplicitVR);
            break;
          }
        case 0x1130: //Table Heigh DS
          {
            ReadDS(fp, nDataEndian, bImplicitVR);
            break;
          }
        case 0x1140: //Rotation Direction CS
          {
            WriteToString(fp,&sHeader,"0018,1140 Rotation Direction: ",nDataEndian,bImplicitVR);
            break;
          }
        case 0x1151: //X-ray Tube Current [mA] IS
          {
            ReadIS(fp, nDataEndian, bImplicitVR);
            break;
          }
        case 0x1170: //Generator Power IS
          {
            ReadIS(fp, nDataEndian, bImplicitVR);
            break;
          }
        case 0x1190: //Focal Spot[s] DS
          {
            ReadDS(fp, nDataEndian, bImplicitVR);
            break;
          }
        case 0x5100: //Patient Position CS
          {
            WriteToString(fp,&sHeader,"0018,5100 Patient Position: ",nDataEndian,bImplicitVR);
            break;
          }
        default:
          {
            int nVal = ReadLength(fp,nDataEndian,bImplicitVR);
            fp.seekg(nVal,ios::cur);
            break;
          }
        }
        break;
      }
    case 0x0020:
      {
        switch(eTag)
        {
        case 0x000D: //Study Instance UID UI
          {
            WriteToString(fp,&sHeader,"0020,000D Study Instance UID: ",nDataEndian,bImplicitVR);
            break;
          }
        case 0x000E: //Series Instance UID UI
          {
            WriteToString(fp,&sHeader,"0020,000E Series Instance UID: ",nDataEndian,bImplicitVR);
            break;
          }
        case 0x0010: //Study ID SH
          {
            WriteToString(fp,&sHeader,"0020,0010 Study ID: ",nDataEndian,bImplicitVR);
            break;
          }
        case 0x0011: //Series Number IS
          {
            ReadIS(fp, nDataEndian, bImplicitVR);
            break;
          }
        case 0x0012: //Acquisition Number IS
          {
            ReadIS(fp, nDataEndian, bImplicitVR);
            break;
          }
        case 0x0013: //Image Number IS
          {
            ReadIS(fp, nDataEndian, bImplicitVR);
            break;
          }
        case 0x0032: //Image Position (Patient) DS
          {
            //????Dicom2006????
            const std::string imagePos = WriteToString(fp,&sHeader,"0020,0032 Image Position (Patient): ",nDataEndian,bImplicitVR);
            _pimpl->imagePosition = convertDicomValue(imagePos);
            break;
          }
        case 0x0037: //Image Orientation (Patient) DS
          {
            //????Dicom2006????
            WriteToString(fp,&sHeader,"0020,0037 Image Orientation (Patient): ",nDataEndian,bImplicitVR);
            break;
          }
        case 0x0052: //Frame of Reference UID UI
          {
            WriteToString(fp,&sHeader,"0020,0052 Frame of Reference UID: ",nDataEndian,bImplicitVR);
            break;
          }
        case 0x1040: //Position Reference LO
          {
            WriteToString(fp,&sHeader,"0020,1040 Position Reference: ",nDataEndian,bImplicitVR);
            break;
          }
        default:
          {
            int nVal = ReadLength(fp,nDataEndian,bImplicitVR);
            fp.seekg(nVal,ios::cur);
            break;
          }
        }
        break;
      }
    case 0x0028:
      {
        switch(eTag)
        {
        case 0x0002: //Samples Per Pixel US
          {
            nSamplesPerPixel = ReadUS(fp, nDataEndian);
            WriteToString(&sHeader,"0028,0002 Samples Per Pixel: ",nSamplesPerPixel);
            break;
          }
        case 0x0004: //Photometric Interpretation CS
          {
            ReadString(fp,szPhotometric,nDataEndian,bImplicitVR);
            //                        WriteToString(&sHeader,"0028,0004 Photometric Interpretation: ",szPhotometric);
            break;
          }
        case 0x0008: //Number of frames IS
          {
            _pimpl->nNumFrames=ReadIS(fp,nDataEndian,bImplicitVR);
            WriteToString(&sHeader,"0028,0008 Number of frames: ", _pimpl->nNumFrames);
            break;
          }
        case 0x0010: //Rows US
          {
            _pimpl->nRows=ReadUS(fp, nDataEndian);
            WriteToString(&sHeader,"0028,0010 Rows: ", _pimpl->nRows);
            break;
          }
        case 0x0011: //Columns US
          {
            _pimpl->nCols=ReadUS(fp, nDataEndian);
            WriteToString(&sHeader,"0028,0011 Columns: ", _pimpl->nCols);
            break;
          }
        case 0x0030: //Pixel Spacing DS
          {
            const std::string pixelSpacing = WriteToString(fp,&sHeader,"0028,0030 Pixel Spacing: ", nDataEndian, bImplicitVR);
             _pimpl->pixelSpacing = convertDicomValue(pixelSpacing);
            // break;
            break;
          }
        case 0x0100: //Bits Allocated US
          {
            _pimpl->nBitsAllocated=ReadUS(fp, nDataEndian);
            WriteToString(&sHeader,"0028,0100 Bits Allocated: ", _pimpl->nBitsAllocated);
            break;
          }
        case 0x0101: //Bits Stored US
          {
            ReadUS(fp, nDataEndian);
            break;
          }
        case 0x0102: //High Bit US
          {
            _pimpl->nHighBit=ReadUS(fp, nDataEndian);
            WriteToString(&sHeader,"0028,0102 High Bit: ", _pimpl->nHighBit);
            break;
          }
        case 0x0103: //Pixel Representation US
          {
            _pimpl->bIsSigned=ReadUS(fp, nDataEndian);
            WriteToString(&sHeader,"0028,0103 Pixel Representation: ", _pimpl->bIsSigned);
            break;
          }
        case 0x0120: //Pixel Padding Value US or SS
          {
            ReadUS(fp, nDataEndian);
            break;
          }
        case 0x1050: //Window Center DS
          {
            _pimpl->fWindowCenter=ReadDS(fp, nDataEndian, bImplicitVR);
            WriteToString(&sHeader,"0028,1050 Window Center: ", (int)_pimpl->fWindowCenter);
            break;
          }
        case 0x1051: //Window Width DS
          {
            _pimpl->fWindowWidth=ReadDS(fp, nDataEndian, bImplicitVR);
            WriteToString(&sHeader,"0028,1051 Window Width: ", (int)_pimpl->fWindowWidth);
            break;
          }
        case 0x1052: //Rescale Intercept DS
          {
            _pimpl->fRescaleIntercept=ReadDS(fp, nDataEndian, bImplicitVR);
            WriteToString(&sHeader,"0028,1052 Rescale Intercept: ",(int)_pimpl->fRescaleIntercept);
            break;
          }
        case 0x1053: //Rescale Slope DS
          {
            _pimpl->fRescaleSlope=ReadDS(fp, nDataEndian, bImplicitVR);
            WriteToString(&sHeader,"0028,1053 Rescale Slope: ",(int)_pimpl->fRescaleSlope);
            break;
          }
        default:
          {
            int nVal = ReadLength(fp,nDataEndian,bImplicitVR);
            fp.seekg(nVal,ios::cur);
            break;
          }
        }
        break;
      }
      case 0x140d:
      {
        switch(eTag)
        {
        case 0x1000:
          {
            WriteToString(&sHeader,"140d,1000 : ", 0);
            fp.seekg(24, ios::cur);
            break;
          }
        default:
          {
            int nVal = ReadLength(fp,nDataEndian,bImplicitVR);
            fp.seekg(nVal,ios::cur);
            break;
          }
        }
        break;
      }
      case 0x160d:
      {
        switch(eTag)
        {
        default:
          {
            int nVal = 0;
            fp.seekg(4, ios::cur);
            fp.read((char*)&nVal, 4);
            fp.seekg(nVal,ios::cur);
            break;
          }
        }
        break;
      }
      case 0x170d:
      {
        switch(eTag)
        {
        default:
          {
            int nVal = 0;
            fp.seekg(4, ios::cur);
            fp.read((char*)&nVal, 4);
            fp.seekg(nVal,ios::cur);
            break;
          }
        }
        break;
      }
      case 0x7005:
      {
        switch(eTag)
        {
        default:
          {
            int nVal = 0;
            fp.seekg(4, ios::cur);
            fp.read((char*)&nVal, 4);
            fp.seekg(nVal,ios::cur);
            break;
          }
        }
        break;
      }
       case 0x7FE0:
      {
        switch(eTag)
        {
        case 0x0010: //Pixel Data OW or OB
          {
            _pimpl->nBytesP = nSamplesPerPixel * _pimpl->nBitsAllocated / 8;
            _pimpl->nFrameSize = _pimpl->nCols * _pimpl->nRows * _pimpl->nBytesP;
            _pimpl->nLength = _pimpl->nNumFrames * _pimpl->nFrameSize;

            // Parse pixel data
            switch(nCompressionMode)
            {
            case COMPRESS_NONE:
              {
                _pimpl->pixelData.reset(new unsigned char[_pimpl->nLength + 16]);
                fp.seekg(4,ios::cur);
                fp.read((char*)_pimpl->pixelData.get(), _pimpl->nLength);
                bPixelData = true;
                break;
              }
            case COMPRESS_RLE:
            case COMPRESS_JPEGLOSSY:
            case COMPRESS_JPEGLOSSY12BIT:
            case COMPRESS_JPEGLOSSLESS:
            case COMPRESS_JPEGLOSSLESS2:
            default:
              break;
            }
            break;
          }
        default:
          {
            int nVal = ReadLength(fp,nDataEndian,bImplicitVR);
            fp.seekg(nVal,ios::cur);
            break;
          }
        }
        break;
      }
   default:
      {
        int nVal = ReadLength(fp,nDataEndian,bImplicitVR);
        fp.seekg(nVal,ios::cur);
        break;
      }
    }
  }

    fp.close();

//    std::cout<<std::endl<<sHeader<<std::endl;

    if (_pimpl->pixelData) // Have we got the pixel data?
    {
        // Need to do byte swap?
      if (nDataEndian == BIG_ENDIAN_DATA && _pimpl->nBitsAllocated > 8) {
        SwapWord((char *)_pimpl->pixelData.get(), _pimpl->nLength / 2);
      }

      _pimpl->pixelData = UpSideDown(_pimpl->pixelData, _pimpl->nCols, _pimpl->nRows, _pimpl->nBytesP, _pimpl->nLength);

      _pimpl->imageAdjuestment();          
    }
}
