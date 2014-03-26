#include "DicomUtil.h"

#include <fstream>
#include <iostream>
#include <memory.h>
#include <vector>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;

namespace {
  short int nCols = 0, nRows = 0;
  int nBytesP = 0, nFrameSize = 0;
  long int nLength = 0;

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
    short int nsLength;

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

  unsigned char* UpSideDown(unsigned char* _pData)
  {
    int nRowBytes=nCols*nBytesP;
    unsigned char* cc=_pData + (nRows - 1)*nRowBytes;
    unsigned char* pNewData=new unsigned char [nLength + 16];
    for(int i=0; i<nRows; i++)
    {
      for(int j=0; j<nRowBytes; j++)
      {
        *(pNewData++)=*(cc+j);
      }
      cc -= nRowBytes;
    }
    return pNewData-nLength;
  }

  std::vector<double> convertDicomValue(const std::string& dicomValue) {
    std::vector<std::string> splittedStrings;
    boost::split(splittedStrings, dicomValue, boost::is_any_of("\\"));

    std::vector<double> result;
    for (auto item : splittedStrings) {
      result.push_back(atof(item.c_str()));
      std::cerr<<result.back()<<std::endl;
    }

   return result;
 }

} // annonymouse namespace

class DicomUtil::Pimpl {
public:
  Pimpl() {}

  /* data */
  std::string fileName;
  std::vector<double> pixelSpacing;
  std::vector<double> imagePosition;
};

DicomUtil::DicomUtil()
  :_pimpl(new Pimpl()), _pData(0)
{
}

DicomUtil::~DicomUtil() {
  delete [] _pData;
}

void DicomUtil::setFileName(const string& filename) {
  _pimpl->fileName = filename;
}

boost::shared_ptr<unsigned char> DicomUtil::pixel() {
  if (_pimpl->fileName.empty()) {
    return boost::shared_ptr<unsigned char>();
  }

  if (!_pData) {
    readImage();
  }

  boost::shared_ptr<unsigned char> pixel(new unsigned char[nLength]);
  memcpy(pixel.get(), _pData, nLength);
  return pixel;
}

int DicomUtil::pixelLength() const {
  return nLength;
}

int DicomUtil::imageHeight() const {
  return nRows;
}

int DicomUtil::imageWidth() const {
  return nCols;
}

unsigned char*
    DicomUtil::convertTo8Bit(unsigned char* _pData, unsigned long nNumPixels,
                              bool bIsSigned, short nHighBit,
                              float fRescaleSlope, float fRescaleIntercept,
                              float fWindowCenter, float fWindowWidth)
{
  unsigned char* pNewData = 0;
  unsigned long nCount;
  short *pp;

  // 1. Clip the high bits.
  if (nHighBit < 15)
  {
    short nMask;
    short nSignBit;

    pp = (short *)_pData;
    nCount = nNumPixels;

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

    pp = (short *)_pData;
    nCount = nNumPixels;

    while( nCount-- > 0 )
    {
      fValue = (*pp) * fRescaleSlope + fRescaleIntercept;
      *pp ++ = (short)fValue;
    }

  }

  // 3. Window-level or rescale to 8-bit
  if ((fWindowCenter != 0) || (fWindowWidth != 0))
  {
    float fSlope;
    float fShift;
    float fValue;
    unsigned char* np = new unsigned char [nNumPixels+4];

    pNewData = np;

    // Since we have window level info, we will only map what are
    // within the Window.

    fShift = fWindowCenter - fWindowWidth / 2.0f;
    fSlope = 255.0f / fWindowWidth;

    nCount = nNumPixels;
    pp = (short *)_pData;

    while (nCount-- > 0)
    {
      fValue = ((*pp ++) - fShift) * fSlope;
      if (fValue < 0)
        fValue = 0;
      else if (fValue > 255)
        fValue = 255;

      *np ++ = (unsigned char) fValue;
    }
  }
  else
  {
    // We will map the whole dynamic range.
    float fSlope;
    float fValue;
    int nMin, nMax;
    unsigned char* np = new unsigned char [nNumPixels+4];

    pNewData = np;

    // First compute the min and max.
    nCount = nNumPixels;
    pp = (short *)_pData;
    nMin = nMax = *pp;
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

    nCount = nNumPixels;
    pp = (short *)_pData;

    // int pixelData = 0;
    // int pixel0024 = 0;
    // int pixel2549 = 0;
    // int pixel5074 = 0;
    // int pixel7599 = 0;
    // int pixel100124 = 0;
    // int pixel125149 = 0;
    // int pixel150174 = 0;
    // int pixel175199 = 0;
    // int pixel200224 = 0;
    // int pixel225255 = 0;

    while (nCount-- > 0)
    {
      fValue = ((*pp ++) - nMin) * fSlope;
      if (fValue < 0)
        fValue = 0;
      else if (fValue > 255)
        fValue = 255;

      *np ++ = (unsigned char) fValue;
#if 0
      pixelData = (int)fValue;

      if (pixelData < 25) {
        pixel0024++;
      }
      else if (pixelData >= 25 && pixelData < 50) {
        pixel2549++;
      }
      else if (pixelData >= 50 && pixelData < 75) {
        pixel5074++;
      }
      else if (pixelData >= 75 && pixelData < 100) {
        pixel7599++;
      }
      else if (pixelData >= 100 && pixelData < 125) {
        pixel100124++;
      }
      else if (pixelData >= 125 && pixelData < 150) {
        pixel125149++;
      }
      else if (pixelData >= 150 && pixelData < 175) {
        pixel150174++;
      }
      else if (pixelData >= 175 && pixelData < 200) {
        pixel175199++;
      }
      else if (pixelData >= 200 && pixelData < 225) {
        pixel200224++;
      }
      else {
        pixel225255++;
      }
      #endif
    }

#if 0
    cout<<"Pixel data between   0 ~  24: "<<pixel0024<<endl;
    cout<<"Pixel data between  25 ~  49: "<<pixel2549<<endl;
    cout<<"Pixel data between  50 ~  74: "<<pixel5074<<endl;
    cout<<"Pixel data between  75 ~  99: "<<pixel7599<<endl;
    cout<<"Pixel data between 100 ~ 124: "<<pixel100124<<endl;
    cout<<"Pixel data between 125 ~ 149: "<<pixel125149<<endl;
    cout<<"Pixel data between 150 ~ 174: "<<pixel150174<<endl;
    cout<<"Pixel data between 175 ~ 199: "<<pixel175199<<endl;
    cout<<"Pixel data between 200 ~ 224: "<<pixel200224<<endl;
    cout<<"Pixel data between 225 ~ 255: "<<pixel225255<<endl;
    #endif
  }

  return (unsigned char* )pNewData;
}

void DicomUtil::readImage()
{
  short int nBitsAllocated, nSamplesPerPixel = 1;
  short int nHighBit = 0;
  char szPhotometric[32] = "", szTransferSyntaxUID[80] = "";
  float fWindowWidth = 0, fWindowCenter = 0, fRescaleSlope = 1, fRescaleIntercept = 0;
  bool bIsSigned = false;
  bool bImplicitVR = true;
  COMPRESSION_MODE nCompressionMode = COMPRESS_NONE;
  DATA_ENDIAN nDataEndian = LITTLE_ENDIAN_DATA;
  //double nThickness = 0.0;
  short int gTag, eTag;
  int nNumFrames = 1;
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
            //                        WriteToString(&sHeader,"0028,0002 Samples Per Pixel: ",nSamplesPerPixel);
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
            nNumFrames=ReadIS(fp,nDataEndian,bImplicitVR);
            WriteToString(&sHeader,"0028,0008 Number of frames: ",nNumFrames);
            break;
          }
        case 0x0010: //Rows US
          {
            nRows=ReadUS(fp, nDataEndian);
            WriteToString(&sHeader,"0028,0010 Rows: ",nRows);
            break;
          }
        case 0x0011: //Columns US
          {
            nCols=ReadUS(fp, nDataEndian);
            WriteToString(&sHeader,"0028,0011 Columns: ",nCols);
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
            nBitsAllocated=ReadUS(fp, nDataEndian);
            WriteToString(&sHeader,"0028,0100 Bits Allocated: ",nBitsAllocated);
            break;
          }
        case 0x0101: //Bits Stored US
          {
            ReadUS(fp, nDataEndian);
            break;
          }
        case 0x0102: //High Bit US
          {
            nHighBit=ReadUS(fp, nDataEndian);
            WriteToString(&sHeader,"0028,0102 High Bit: ",nHighBit);
            break;
          }
        case 0x0103: //Pixel Representation US
          {
            bIsSigned=ReadUS(fp, nDataEndian);
            WriteToString(&sHeader,"0028,0103 Pixel Representation: ",bIsSigned);
            break;
          }
        case 0x0120: //Pixel Padding Value US or SS
          {
            ReadUS(fp, nDataEndian);
            break;
          }
        case 0x1050: //Window Center DS
          {
            fWindowCenter=ReadDS(fp, nDataEndian, bImplicitVR);
            WriteToString(&sHeader,"0028,1050 Window Center: ",(int)fWindowCenter);
            break;
          }
        case 0x1051: //Window Width DS
          {
            fWindowWidth=ReadDS(fp, nDataEndian, bImplicitVR);
            WriteToString(&sHeader,"0028,1051 Window Width: ",(int)fWindowWidth);
            break;
          }
        case 0x1052: //Rescale Intercept DS
          {
            fRescaleIntercept=ReadDS(fp, nDataEndian, bImplicitVR);
            break;
          }
        case 0x1053: //Rescale Slope DS
          {
            fRescaleSlope=ReadDS(fp, nDataEndian, bImplicitVR);
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
        case 0x1000:
          {
            //WriteToString(fp,&sHeader,"160d,1000 : ", nDataEndian, bImplicitVR);
            WriteToString(&sHeader,"160d,1000 : ", 0);
            fp.seekg(538, ios::cur);
            break;
          }
        default:
          {
            int nVal = ReadLength(fp,nDataEndian,bImplicitVR);
            if (nVal == 0) {std::cout<<"160d default"<<std::endl;}
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
            nBytesP = nSamplesPerPixel*nBitsAllocated/8;
            nFrameSize = nCols * nRows * nBytesP;
            nLength = nNumFrames * nFrameSize;

            // Parse pixel data
            switch(nCompressionMode)
            {
            case COMPRESS_NONE:
              {
                _pData = new unsigned char[nLength + 16];
                fp.seekg(4,ios::cur);
                fp.read((char*)_pData, nLength);
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

    if (_pData) // Have we got the pixel data?
    {
        // Need to do byte swap?
        if (nDataEndian == BIG_ENDIAN_DATA)
        {
            if (nBitsAllocated > 8)
                SwapWord((char *)_pData, nLength/2);
        }

        unsigned char* pNewData=UpSideDown(_pData);
#if 1
        if (nBitsAllocated > 8)
        {
            // We need to convert it to 8-bit.
            pNewData = convertTo8Bit(pNewData, nLength/2, bIsSigned, nHighBit,
                                     fRescaleSlope, fRescaleIntercept,
                                     fWindowCenter, fWindowWidth);
            // Use the new 8-bit data.
            if (pNewData)
            {
                delete [] _pData;
                _pData = pNewData;
                nBytesP = 1;
                nFrameSize /= 2;
                nLength /= 2;
            }
        }
#endif
    }
}
