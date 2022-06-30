#include "bmpStruct.h"
#include <stdio.h>
#include <string>

#include <iostream>


extern void error(std::string msg);
extern BYTE min(BYTE r, BYTE g, BYTE b);
extern BYTE getMaskSize(DWORD mask, BYTE offset);
extern void offsetAdjustment(BYTE& rOffset, BYTE& gOffset, BYTE& bOffset, DWORD r, DWORD g, DWORD b);
extern int getPid(WORD width);
extern bool isSupportedBitCount(DWORD bit);

template<typename> extern BYTE getMediumColor(void*, DWORD, DWORD, DWORD, BYTE, BYTE, BYTE);
extern BYTE getOffsetForMask(DWORD mask);


extern void pixelIntoGrayScale(void* pixel);
template<typename T> extern void pixelIntoGrayScale_mask(void* pixel, DWORD, DWORD, DWORD, BYTE bOffset, BYTE gOffset, BYTE rOffset);

struct bmp_error
{
    std::string name;
    bmp_error(std::string n) :name{n} {}
    bmp_error(const char* n) :name{n} {}
};


class bmp_base
{
protected:

    std::string file;
    FILE* pFile;
    DWORD paletteSize;

    //BMPVERSION version;
    BITMAPFILEHEADER bfh;

    BYTE* pixels;
    BYTE* palette;
    
public:

    bmp_base()  :file{""}, bfh{}, pFile{nullptr}, pixels{nullptr}, palette{nullptr}, paletteSize{0} {}
    bmp_base(std::string pathway)   :file{pathway}, pFile{nullptr}, pixels{nullptr}, palette{nullptr}, paletteSize{0} { readBMPFileHeader(); }
    bmp_base(BITMAPFILEHEADER h, std::string pathway, FILE* f) :bfh{h}, file{pathway}, pFile{f}, palette{nullptr}, pixels{nullptr}, paletteSize{0} {}

   // virtual ~bmp_base(){
    ~bmp_base(){
        std::cout << "Destructor called\n";
        if(pixels != nullptr)
            delete[] pixels;

        if(palette != nullptr)
            delete[] palette;

        if(pFile != nullptr)
            fclose(pFile);
    }

    void readBMPFileHeader();

    virtual void readBMPInfoHeader()=0;
    virtual void readPalette()=0;
    virtual void readPixels()=0;
    virtual void writeInFile(std::string pathway)=0;
    virtual void toGrayScales()=0;
};


class BMP
{
    bmp_base* bmp;
public:
    BMP()   :bmp{nullptr} {}
    BMP(std::string pathway) :bmp{nullptr} { getBMPFile(pathway); }

    ~BMP()
    {
        if(bmp != nullptr)
            delete bmp;
    }
    void getBMPFile(std::string);
    void intoGrayScales() { bmp->toGrayScales(); }
    void writeBMPFile(std::string f) { bmp->writeInFile(f); }
};



template<class T> class BMPImage : public bmp_base {};

template<> class BMPImage<BITMAPCOREHEADER> : public bmp_base
{
    BITMAPCOREHEADER bmpHeader;
    void loadImage() {
        readBMPInfoHeader();
        if(bmpHeader.bcBitCount<=8)
            readPalette();
        readPixels();
    }
    void to_grayScale();
    void to_grayScale_mask();
    void to_grayScale_palette();

    void readBMPInfoHeader()  override;
    void readPalette() override;
    void readPixels() override;
public:
    BMPImage() :bmp_base(), bmpHeader() {}
    BMPImage(std::string pathway) :bmp_base(pathway) { loadImage(); }   
    BMPImage(BITMAPFILEHEADER h, FILE* f, std::string pathway)  :bmp_base(h,pathway, f) { loadImage(); }

    ~BMPImage() { }

    void toGrayScales() override;

    void writeInFile(std::string pathway) override;
};


template<> class BMPImage<BITMAPINFOHEADER> : public bmp_base
{
    BITMAPINFOHEADER bmpHeader;
    DWORD masks[4];
    int masksBytes;
    void loadImage() {
        readBMPInfoHeader();
        if(bmpHeader.biBitCount<=8 || bmpHeader.biClrUsed != 0)
            readPalette();
        readPixels();
    }
    void to_grayScale();
    void to_grayScale_mask();
    void to_grayScale_palette();

    void readBMPInfoHeader()  override;
    void readPalette() override;
    void readPixels() override;
public:
    BMPImage() :bmp_base(), bmpHeader{}, masksBytes{0} {}
    BMPImage(std::string pathway) :bmp_base(pathway), masksBytes{0} { loadImage(); }   
    BMPImage(BITMAPFILEHEADER h, FILE* f, std::string pathway)  :bmp_base(h,pathway, f), masksBytes{0} { std::cout<<"create INFOHEADER\n"; loadImage(); }

    ~BMPImage() { }

    void toGrayScales() override;
    
    void writeInFile(std::string pathway) override;
};

template<> class BMPImage<BITMAPV4HEADER> : public bmp_base
{
    BITMAPV4HEADER bmpHeader;

    void loadImage() {
        readBMPInfoHeader();
        if(bmpHeader.biBitCount<=8 || bmpHeader.biClrUsed != 0)
            readPalette();
        readPixels();
    }
    void to_grayScale();
    void to_grayScale_mask();
    void to_grayScale_palette();

    void readBMPInfoHeader()  override;
    void readPalette() override;
    void readPixels() override;
public:
    BMPImage() :bmp_base(), bmpHeader() {}
    BMPImage(std::string pathway) :bmp_base(pathway) { loadImage(); }   
    BMPImage(BITMAPFILEHEADER h, FILE* f, std::string pathway)  :bmp_base(h,pathway, f) { loadImage(); }

    ~BMPImage() { }


    void toGrayScales() override;

    void writeInFile(std::string pathway) override;
};


template<> class BMPImage<BITMAPV5HEADER> : public bmp_base
{
    BITMAPV5HEADER bmpHeader;

    void loadImage() {
        readBMPInfoHeader();
        if(bmpHeader.biBitCount<=8 || bmpHeader.biClrUsed != 0)
            readPalette();
        readPixels();
    }
    void to_grayScale();
    void to_grayScale_mask();
    void to_grayScale_palette();

    
    void readBMPInfoHeader()  override;
    void readPalette() override;
    void readPixels() override;

public:
    BMPImage() :bmp_base(), bmpHeader() {}
    BMPImage(std::string pathway) :bmp_base(pathway) { loadImage(); }   
    BMPImage(BITMAPFILEHEADER h, FILE* f, std::string pathway)  :bmp_base(h,pathway, f) { loadImage(); }

    ~BMPImage() { }

    void toGrayScales() override;

    void writeInFile(std::string pathway) override;
};