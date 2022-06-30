#include "bmp.h"
#include <cmath>
#include <cstring>

#include <bitset>
#include <iomanip>

void error(std::string msg)
{
    throw bmp_error(msg);
}


void bmp_base::readBMPFileHeader()
{
    pFile = fopen(file.c_str(), "rb");
        if(pFile == nullptr)
            error("Cannot open file: " + file);
    
    fread(&bfh, sizeof(BITMAPFILEHEADER), 1, pFile);

    //fclose(pFile);
}


void BMPImage<BITMAPCOREHEADER>::readBMPInfoHeader()
{
    if(pFile == nullptr) {
        pFile = fopen(file.c_str(), "rb");
        if(pFile == nullptr)
            error("Cannot open file: " + file);
    }

    fseek(pFile, sizeof(BITMAPFILEHEADER), SEEK_SET);
    fread(&bmpHeader, sizeof(BITMAPCOREHEADER), 1, pFile);

    if(!isSupportedBitCount(bmpHeader.bcBitCount))
    {
        fclose(pFile);
        error("Unsupported bit count!");
    }
}

void BMPImage<BITMAPCOREHEADER>::readPalette()
{
    if(pFile == nullptr) {
        pFile = fopen(file.c_str(), "rb");
        if(pFile == nullptr)
            error("Cannot open file: " + file);
    }

    paletteSize = pow(2,bmpHeader.bcBitCount);

    pixels = new BYTE[paletteSize * sizeof(RGBTRIPLE)];   

    fseek(pFile, sizeof(BITMAPFILEHEADER) + sizeof(BITMAPCOREHEADER), SEEK_SET);
    fread(pixels, sizeof(BYTE), paletteSize * sizeof(RGBTRIPLE), pFile);

    //fclose(pFile);
}

void BMPImage<BITMAPCOREHEADER>::readPixels()
{
    if(pFile == nullptr) {
        pFile = fopen(file.c_str(), "rb");
        if(pFile == nullptr)
            error("Cannot open file: " + file);
    }
    DWORD rowSize = ceil( ((float)bmpHeader.bcBitCount/8) * bmpHeader.bcWidth);
    pixels = new BYTE[rowSize * bmpHeader.bcHeight];

    fseek(pFile, bfh.bfOffBits, SEEK_SET);
    for(DWORD i=0;i<bmpHeader.bcHeight;i++)
    {
        fread(&pixels[i*(rowSize)], sizeof(BYTE), rowSize, pFile);
        fseek(pFile, getPid(bmpHeader.bcWidth * ceil(bmpHeader.bcBitCount/8)), SEEK_CUR);
    }

    fclose(pFile);
}

void BMPImage<BITMAPCOREHEADER>::writeInFile(std::string pathway)
{
    FILE* out_file = fopen(pathway.c_str(), "wb");
    if(out_file == nullptr)
        error("Cannot create file: " + pathway);


    DWORD rowSize = ceil( ((float)bmpHeader.bcBitCount/8) * bmpHeader.bcWidth);

    fwrite(&bfh, sizeof(BITMAPFILEHEADER), 1, out_file);
    fwrite(&bmpHeader, sizeof(BITMAPCOREHEADER), 1, out_file);
    if(palette != nullptr)
    {
        fwrite(palette, sizeof(BYTE), paletteSize*sizeof(RGBTRIPLE), out_file);
    }

    BYTE zero = 0;
    fseek(out_file, bfh.bfOffBits, SEEK_SET);
    for(DWORD i=0;i<bmpHeader.bcHeight;i++)
    {
        fwrite(&pixels[i*rowSize], sizeof(BYTE), rowSize, out_file);
        for(int j=0;j<getPid(bmpHeader.bcWidth * ceil(bmpHeader.bcBitCount/8));j++)
            fwrite(&zero, sizeof(BYTE), 1, out_file);
    }

    fclose(out_file);
}

void BMPImage<BITMAPCOREHEADER>::to_grayScale_mask()
{
    BYTE blueMaskOffset = getOffsetForMask(BLUEDEFAULTBITMASK_16BIT);
	BYTE greenMaskOffset = getOffsetForMask(GREENDEFAULTBITMASK_16BIT);
	BYTE redMaskOffset = getOffsetForMask(REDDEFAULTBITMASK_16BIT);

    for(DWORD i=0;i<(bmpHeader.bcHeight*bmpHeader.bcWidth);i++)
    {
        pixelIntoGrayScale_mask<WORD>(
            &pixels[i * (bmpHeader.bcBitCount/8)],
            BLUEDEFAULTBITMASK_16BIT,
            GREENDEFAULTBITMASK_16BIT,
            REDDEFAULTBITMASK_16BIT,
            blueMaskOffset,
            greenMaskOffset,
            redMaskOffset
        );
    }
}

void BMPImage<BITMAPCOREHEADER>::to_grayScale_palette()
{
    RGBTRIPLE* p = nullptr;
    for(DWORD i=0;i<paletteSize;i++)
    {
        pixelIntoGrayScale(&palette[i * 4]);
    }
}

void BMPImage<BITMAPCOREHEADER>::to_grayScale()
{
    for(DWORD i=0;i<(bmpHeader.bcHeight*bmpHeader.bcWidth);i++)
    {
        pixelIntoGrayScale(&pixels[i * (bmpHeader.bcBitCount/8)]);
    }
}


void BMPImage<BITMAPCOREHEADER>::toGrayScales()
{
    if(palette != nullptr)
    {
        to_grayScale_palette();
    } else if(bmpHeader.bcBitCount == 16)
    {
        to_grayScale_mask();
    }
    else{
        to_grayScale();
    }
}






void BMPImage<BITMAPINFOHEADER>::readBMPInfoHeader()
{
    if(pFile == nullptr) {
        pFile = fopen(file.c_str(), "rb");
        if(pFile == nullptr)
            error("Cannot open file: " + file);
    }


    fseek(pFile, sizeof(BITMAPFILEHEADER), SEEK_SET);
    fread(&bmpHeader, sizeof(BITMAPINFOHEADER), 1, pFile);

    if(!isSupportedBitCount(bmpHeader.biBitCount))
    {
        fclose(pFile);
        error("Unsupported bit count!");
    }

    if(bmpHeader.biCompression != BI_RGB)
    {
        if(bmpHeader.biCompression == BI_BITFIELDS)
        {
            masksBytes = 3;
            fread(masks, sizeof(DWORD), masksBytes, pFile);
        }
        else if(bmpHeader.biCompression == BI_ALPHABITFIELDS)
        {   
            masksBytes = 4;
            fread(masks, sizeof(DWORD), masksBytes, pFile);
        }
        else{
            error("This .bmp file not supported");
        }
    } else if(bmpHeader.biBitCount == 16){
        masks[0] = REDDEFAULTBITMASK_16BIT;
        masks[1] = GREENDEFAULTBITMASK_16BIT;
        masks[2] = BLUEDEFAULTBITMASK_16BIT;
    }
    //fclose(pFile);
}

void BMPImage<BITMAPINFOHEADER>::readPalette()
{
    if(pFile == nullptr) {
        pFile = fopen(file.c_str(), "rb");
        if(pFile == nullptr)
            error("Cannot open file: " + file);
    }

    std::cout << "READ PALETTE\n";

    
    if(bmpHeader.biBitCount <= 8)
        paletteSize = pow(2, bmpHeader.biBitCount);
    
    if(bmpHeader.biClrUsed != 0)
        paletteSize = ( (bmpHeader.biClrUsed > paletteSize) && (paletteSize!=0) ) ? paletteSize : bmpHeader.biClrUsed;

    palette = new BYTE[paletteSize * sizeof(RGBQUAD)];


    fseek(pFile, sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + (masksBytes * sizeof(DWORD)), SEEK_SET);

    fread(palette, sizeof(BYTE), sizeof(RGBQUAD) * paletteSize, pFile);


}

void BMPImage<BITMAPINFOHEADER>::readPixels()
{
    if(pFile == nullptr) {
        pFile = fopen(file.c_str(), "rb");
        if(pFile == nullptr)
            error("Cannot open file: " + file);
    }

    DWORD rowSize = ceil( ((float)bmpHeader.biBitCount/8) * bmpHeader.biWidth);
    pixels = new BYTE[rowSize * bmpHeader.biHeight];

    fseek(pFile, bfh.bfOffBits, SEEK_SET);
    for(DWORD i=0;i<bmpHeader.biHeight;i++)
    {
        fread(&pixels[i*(rowSize)], sizeof(BYTE), rowSize, pFile);
        fseek(pFile, getPid(rowSize), SEEK_CUR);
    }

    fclose(pFile);
}

void BMPImage<BITMAPINFOHEADER>::writeInFile(std::string pathway)
{
    FILE* out_file = fopen(pathway.c_str(), "wb");
    if(out_file == nullptr)
        error("Cannot create file: " + pathway);

    DWORD rowSize = ceil( ((float)bmpHeader.biBitCount/8) * bmpHeader.biWidth);

    fwrite(&bfh, sizeof(BITMAPFILEHEADER), 1, out_file); 
    fwrite(&bmpHeader, sizeof(BITMAPINFOHEADER), 1, out_file);


    if(masksBytes != 0){
        fwrite(masks, sizeof(DWORD), masksBytes, out_file);
    }

    if(palette != nullptr){
        fwrite(palette, sizeof(BYTE), sizeof(RGBQUAD) * paletteSize, out_file);
    }
    
    BYTE zero = 0;
    fseek(out_file, bfh.bfOffBits, SEEK_SET);

    for(DWORD i=0;i<bmpHeader.biHeight;i++)
    {
        fwrite(&pixels[i*rowSize], sizeof(BYTE), rowSize, out_file);
        for(int j=0;j<getPid(rowSize);j++)
        {
            fwrite(&zero, sizeof(BYTE), 1, out_file);
        }
    }

    fclose(out_file);
}




void BMPImage<BITMAPINFOHEADER>::to_grayScale_palette()
{
    RGBTRIPLE* p = nullptr;
    for(DWORD i=0;i<paletteSize;i++)
    {
        pixelIntoGrayScale(&palette[i * 4]);
    }
}

void BMPImage<BITMAPINFOHEADER>::to_grayScale()
{
    RGBTRIPLE* p = nullptr;
    for(DWORD i=0;i<(bmpHeader.biHeight*bmpHeader.biWidth);i++)
    {
        pixelIntoGrayScale(&pixels[i * (bmpHeader.biBitCount/8)]);
    }
}

void BMPImage<BITMAPINFOHEADER>::to_grayScale_mask()
{
    BYTE blueMaskOffset = getOffsetForMask(masks[2]);
	BYTE greenMaskOffset = getOffsetForMask(masks[1]);
	BYTE redMaskOffset = getOffsetForMask(masks[0]);

    offsetAdjustment(redMaskOffset, greenMaskOffset, blueMaskOffset, masks[0], masks[1], masks[2]);

    if(bmpHeader.biBitCount == 16){

        for(DWORD i=0;i<(bmpHeader.biHeight*bmpHeader.biWidth);i++)
        {
            pixelIntoGrayScale_mask<WORD>(
                &pixels[i * (bmpHeader.biBitCount/8)],
                masks[2],
                masks[1],
                masks[0],
                blueMaskOffset,
                greenMaskOffset,
                redMaskOffset
            );
        }
    }
    else{
        for(DWORD i=0;i<(bmpHeader.biHeight*bmpHeader.biWidth);i++)
        {
            pixelIntoGrayScale_mask<int24_t>(
                &pixels[i * (bmpHeader.biBitCount/8)],
                masks[2],
                masks[1],
                masks[0],
                blueMaskOffset,
                greenMaskOffset,
                redMaskOffset
            );
        }
    }
    
}


void BMPImage<BITMAPINFOHEADER>::toGrayScales()
{
    std::cout << "INFO HEADER toGrayScales() called\n";
    if(palette != nullptr)
    {
        std::cout << "\tPalette \n";
        to_grayScale_palette();
    } else if(masksBytes != 0 || bmpHeader.biBitCount == 16){
        std::cout << "\tMask \n";
        to_grayScale_mask();
    }
    else{
        std::cout << "\tto_grayScale()\n";
        to_grayScale();
    }
}





void BMPImage<BITMAPV4HEADER>::readBMPInfoHeader()
{
    if(pFile == nullptr) {
        pFile = fopen(file.c_str(), "rb");
        if(pFile == nullptr)
            error("Cannot open file: " + file);
    }

    fseek(pFile, sizeof(BITMAPFILEHEADER), SEEK_SET);
    fread(&bmpHeader, sizeof(BITMAPV4HEADER), 1, pFile);

    if(bmpHeader.biCompression != BI_RGB &&
        (bmpHeader.biCompression != BI_BITFIELDS && bmpHeader.biCompression != BI_ALPHABITFIELDS)){
            std::cout << "biCompression == " << bmpHeader.biCompression << std::endl;
            error("This .bmp file not supported");
    }
    if(bmpHeader.biBitCount == 16 && (bmpHeader.biCompression != BI_BITFIELDS || bmpHeader.biCompression != BI_ALPHABITFIELDS))
    {
        bmpHeader.bV4RedMask = REDDEFAULTBITMASK_16BIT;
        bmpHeader.bV4GreenMask = GREENDEFAULTBITMASK_16BIT;
        bmpHeader.bV4BlueMask = BLUEDEFAULTBITMASK_16BIT;
    }

    if(!isSupportedBitCount(bmpHeader.biBitCount))
    {
        fclose(pFile);
        error("Unsupported bit count!");
    }
}

void BMPImage<BITMAPV4HEADER>::readPalette()
{
    if(pFile == nullptr) {
        pFile = fopen(file.c_str(), "rb");
        if(pFile == nullptr)
            error("Cannot open file: " + file);
    }
   
    if(bmpHeader.biBitCount <= 8)
        paletteSize = pow(2, bmpHeader.biBitCount);
    
    if(bmpHeader.biClrUsed != 0)
        paletteSize = ( (bmpHeader.biClrUsed > paletteSize) && (paletteSize!=0) ) ? paletteSize : bmpHeader.biClrUsed;

    pixels = new BYTE[paletteSize * sizeof(RGBQUAD)];   

    fseek(pFile, sizeof(BITMAPFILEHEADER) + sizeof(BITMAPV4HEADER), SEEK_SET);
    fread(pixels, sizeof(BYTE), paletteSize * sizeof(RGBQUAD), pFile);

    //fclose(pFile);
}

void BMPImage<BITMAPV4HEADER>::readPixels()
{
    if(pFile == nullptr) {
        pFile = fopen(file.c_str(), "rb");
        if(pFile == nullptr)
            error("Cannot open file: " + file);
    }
    DWORD rowSize = ceil( ((float)bmpHeader.biBitCount/8) * bmpHeader.biWidth);
    pixels = new BYTE[rowSize * bmpHeader.biHeight];

    fseek(pFile, bfh.bfOffBits, SEEK_SET);
    for(DWORD i=0;i<bmpHeader.biHeight;i++)
    {
        fread(&pixels[i*(rowSize)], sizeof(BYTE), rowSize, pFile);
        fseek(pFile, getPid(bmpHeader.biWidth * ceil(bmpHeader.biBitCount/8)), SEEK_CUR);
    }

    fclose(pFile);
}

void BMPImage<BITMAPV4HEADER>::writeInFile(std::string pathway)
{
    FILE* out_file = fopen(pathway.c_str(), "wb");
    if(out_file == nullptr)
        error("Cannot create file: " + pathway);

    DWORD rowSize = ceil( ((float)bmpHeader.biBitCount/8) * bmpHeader.biWidth);

    fwrite(&bfh, sizeof(BITMAPFILEHEADER), 1, out_file);
    fwrite(&bmpHeader, sizeof(BITMAPV4HEADER), 1, out_file);

    if(palette != nullptr)
    {
        fwrite(palette, sizeof(BYTE), paletteSize * sizeof(RGBQUAD), out_file);
    }
    
    BYTE zero = 0;
    fseek(out_file, bfh.bfOffBits, SEEK_SET);
    for(DWORD i=0;i<bmpHeader.biHeight;i++)
    {
        fwrite(&pixels[i*rowSize], sizeof(BYTE), rowSize, out_file);
        for(int j=0;j<getPid(bmpHeader.biWidth * ceil(bmpHeader.biBitCount/8));j++)
            fwrite(&zero, sizeof(BYTE), 1, out_file);
    }

    fclose(out_file);
}




void BMPImage<BITMAPV4HEADER>::to_grayScale_palette()
{
    RGBTRIPLE* p = nullptr;
    for(DWORD i=0;i<paletteSize;i++)
    {
        pixelIntoGrayScale(&palette[i * 4]);
    }
}

void BMPImage<BITMAPV4HEADER>::to_grayScale()
{
    for(DWORD i=0;i<(bmpHeader.biHeight*bmpHeader.biWidth);i++)
    {
        pixelIntoGrayScale(&pixels[i * (bmpHeader.biBitCount/8)]);
    }
}

void BMPImage<BITMAPV4HEADER>::to_grayScale_mask()
{
    BYTE blueMaskOffset = getOffsetForMask(bmpHeader.bV4BlueMask);
	BYTE greenMaskOffset = getOffsetForMask(bmpHeader.bV4GreenMask);
	BYTE redMaskOffset = getOffsetForMask(bmpHeader.bV4RedMask);

    offsetAdjustment(redMaskOffset, greenMaskOffset, blueMaskOffset, bmpHeader.bV4RedMask, bmpHeader.bV4GreenMask, bmpHeader.bV4BlueMask);

    if(bmpHeader.biBitCount == 16){
        for(DWORD i=0;i<(bmpHeader.biHeight*bmpHeader.biWidth);i++)
        {
            pixelIntoGrayScale_mask<WORD>(
                &pixels[i * (bmpHeader.biBitCount/8)],
                bmpHeader.bV4BlueMask,
                bmpHeader.bV4GreenMask,
                bmpHeader.bV4RedMask,
                blueMaskOffset,
                greenMaskOffset,
                redMaskOffset
            );
        }
    }
    else{
        for(DWORD i=0;i<(bmpHeader.biHeight*bmpHeader.biWidth);i++)
        {
            pixelIntoGrayScale_mask<int24_t>(
                &pixels[i * (bmpHeader.biBitCount/8)],
                bmpHeader.bV4BlueMask,
                bmpHeader.bV4GreenMask,
                bmpHeader.bV4RedMask,
                blueMaskOffset,
                greenMaskOffset,
                redMaskOffset
            );
        }
    }
}


void BMPImage<BITMAPV4HEADER>::toGrayScales()
{
    if(palette != nullptr)
    {
        to_grayScale_palette();
    } else if(bmpHeader.biClrUsed != 0 || bmpHeader.biBitCount == 16){
        to_grayScale_mask();
    }
    else{
        to_grayScale();
    }
}






void BMPImage<BITMAPV5HEADER>::readBMPInfoHeader()
{
    if(pFile == nullptr) {
        pFile = fopen(file.c_str(), "rb");
        if(pFile == nullptr)
            error("Cannot open file: " + file);
    }

    fseek(pFile, sizeof(BITMAPFILEHEADER), SEEK_SET);
    fread(&bmpHeader, sizeof(BITMAPV5HEADER), 1, pFile);

    if(bmpHeader.biCompression != BI_RGB &&
        (bmpHeader.biCompression != BI_BITFIELDS && bmpHeader.biCompression != BI_ALPHABITFIELDS)){
            error("This .bmp file not supported");
    }
    
    if(bmpHeader.biBitCount == 16 && (bmpHeader.biCompression != BI_BITFIELDS || bmpHeader.biCompression != BI_ALPHABITFIELDS))
    {
        bmpHeader.bV4RedMask = REDDEFAULTBITMASK_16BIT;
        bmpHeader.bV4GreenMask = GREENDEFAULTBITMASK_16BIT;
        bmpHeader.bV4BlueMask = BLUEDEFAULTBITMASK_16BIT;
    }

    if(!isSupportedBitCount(bmpHeader.biBitCount))
    {
        fclose(pFile);
        error("Unsupported bit count!");
    }
}

void BMPImage<BITMAPV5HEADER>::readPalette()
{
    if(pFile == nullptr) {
        pFile = fopen(file.c_str(), "rb");
        if(pFile == nullptr)
            error("Cannot open file: " + file);
    }

    if(bmpHeader.biBitCount <= 8)
        paletteSize = pow(2, bmpHeader.biBitCount);
    
    if(bmpHeader.biClrUsed != 0)
        paletteSize = ( (bmpHeader.biClrUsed > paletteSize) && (paletteSize!=0) ) ? paletteSize : bmpHeader.biClrUsed;

    pixels = new BYTE[paletteSize * sizeof(RGBQUAD)];   

    fseek(pFile, sizeof(BITMAPFILEHEADER) + sizeof(BITMAPV5HEADER), SEEK_SET);
    fread(pixels, sizeof(BYTE), paletteSize * sizeof(RGBQUAD), pFile);

    //fclose(pFile);
}

void BMPImage<BITMAPV5HEADER>::readPixels()
{
    if(pFile == nullptr) {
        pFile = fopen(file.c_str(), "rb");
        if(pFile == nullptr)
            error("Cannot open file: " + file);
    }
    DWORD rowSize = ceil( ((float)bmpHeader.biBitCount/8) * bmpHeader.biWidth);
    pixels = new BYTE[rowSize * bmpHeader.biHeight];

    fseek(pFile, bfh.bfOffBits, SEEK_SET);
    for(DWORD i=0;i<bmpHeader.biHeight;i++)
    {
        fread(&pixels[i*(rowSize)], sizeof(BYTE), rowSize, pFile);
        fseek(pFile, getPid(bmpHeader.biWidth * ceil(bmpHeader.biBitCount/8)), SEEK_CUR);
    }

    fclose(pFile);
}

void BMPImage<BITMAPV5HEADER>::writeInFile(std::string pathway)
{
    FILE* out_file = fopen(pathway.c_str(), "wb");
    if(out_file == nullptr)
        error("Cannot create file: " + pathway);


    DWORD rowSize = ceil( ((float)bmpHeader.biBitCount/8) * bmpHeader.biWidth);

    fwrite(&bfh, sizeof(BITMAPFILEHEADER), 1, out_file);
    fwrite(&bmpHeader, sizeof(BITMAPV5HEADER), 1, out_file);

    if(palette != nullptr)
    {
        fwrite(palette, sizeof(BYTE), paletteSize * sizeof(RGBQUAD), out_file);
    }
    
    BYTE zero = 0;
    fseek(out_file, bfh.bfOffBits, SEEK_SET);
    for(DWORD i=0;i<bmpHeader.biHeight;i++)
    {
        fwrite(&pixels[i*rowSize], sizeof(BYTE), rowSize, out_file);
        for(int j=0;j<getPid(bmpHeader.biWidth * ceil(bmpHeader.biBitCount/8));j++)
            fwrite(&zero, sizeof(BYTE), 1, out_file);
    }

    fclose(out_file);
}


void BMPImage<BITMAPV5HEADER>::to_grayScale_palette()
{
    RGBTRIPLE* p = nullptr;
    for(DWORD i=0;i<paletteSize;i++)
    {
        pixelIntoGrayScale(&palette[i * 4]);
    }
}

void BMPImage<BITMAPV5HEADER>::to_grayScale()
{
    for(DWORD i=0;i<(bmpHeader.biHeight*bmpHeader.biWidth);i++)
    {
        pixelIntoGrayScale(&pixels[i * (bmpHeader.biBitCount/8)]);
    }
}

void BMPImage<BITMAPV5HEADER>::to_grayScale_mask()
{
    BYTE blueMaskOffset = getOffsetForMask(bmpHeader.bV4BlueMask);
	BYTE greenMaskOffset = getOffsetForMask(bmpHeader.bV4GreenMask);
	BYTE redMaskOffset = getOffsetForMask(bmpHeader.bV4RedMask);
    
    offsetAdjustment(redMaskOffset, greenMaskOffset, blueMaskOffset, bmpHeader.bV4RedMask, bmpHeader.bV4GreenMask, bmpHeader.bV4BlueMask);

    if(bmpHeader.biBitCount == 16){

        for(DWORD i=0;i<(bmpHeader.biHeight*bmpHeader.biWidth);i++)
        {
            pixelIntoGrayScale_mask<WORD>(
                &pixels[i * (bmpHeader.biBitCount/8)],
                bmpHeader.bV4BlueMask,
                bmpHeader.bV4GreenMask,
                bmpHeader.bV4RedMask,
                blueMaskOffset,
                greenMaskOffset,
                redMaskOffset
            );
        }
    }
    else{
        for(DWORD i=0;i<(bmpHeader.biHeight*bmpHeader.biWidth);i++)
        {
            pixelIntoGrayScale_mask<int24_t>(
                &pixels[i * (bmpHeader.biBitCount/8)],
                bmpHeader.bV4BlueMask,
                bmpHeader.bV4GreenMask,
                bmpHeader.bV4RedMask,
                blueMaskOffset,
                greenMaskOffset,
                redMaskOffset
            );
        }
    }
}


void BMPImage<BITMAPV5HEADER>::toGrayScales()
{
    if(palette != nullptr)
    {
        to_grayScale_palette();
    } else if(bmpHeader.biClrUsed != 0 || bmpHeader.biBitCount == 16){
        to_grayScale_mask();
    }
    else{
        to_grayScale();
    }
}





int getPid(WORD widthInBytes)
{
    return widthInBytes%4 ? 4 - (widthInBytes % 4) : 0;
}


void BMP::getBMPFile(std::string filePathway)
{
    FILE* file = fopen(filePathway.c_str(), "rb");
    if(file == nullptr)
        error("Cannot open file: " + filePathway);

    BITMAPFILEHEADER bfh;
    fread(&bfh, sizeof(BITMAPFILEHEADER), 1, file);

    if(bfh.bfType != 19778) // != BMP
    {
        fclose(file);
        error("Error! File signature does not match BMP file signature.");
    }

    DWORD size;
    fread(&size, sizeof(DWORD), 1, file);


    std::cout << "SIZE = " << size << std::endl;
    switch (size)
    {
        case DWORD(BMPVERSION::CORE):
            std::cout<<"it's CORE HEADER\n";
            bmp = new BMPImage<BITMAPCOREHEADER>{bfh, file, filePathway};
            break;
        case DWORD(BMPVERSION::V3):
            std::cout<<"it's V3 HEADER\n";
            bmp = new BMPImage<BITMAPINFOHEADER>{bfh, file, filePathway};
            break;
        case DWORD(BMPVERSION::V4):
            std::cout<<"it's V4 HEADER\n";
            bmp = new BMPImage<BITMAPV4HEADER>{bfh, file, filePathway};
            break;
        case DWORD(BMPVERSION::V5):
            std::cout<<"it's V5 HEADER\n";
            bmp = new BMPImage<BITMAPV5HEADER>{bfh, file, filePathway};
            break;
        default:
            error("Unkown version of BMP");
            break;
    }
}


bool isSupportedBitCount(DWORD bit){
   switch (bit)
   {
        case 1: case 2: case 4: case 8:
        case 16: case 24: case 32:
            return true;

        default:
            return false;
   }
   
}

BYTE min(BYTE r, BYTE g, BYTE b){
    if( b < g)
        return b < r ? b : r;
    else
        return g < r ? g : r;
}

void offsetAdjustment(BYTE& rOffset, BYTE& gOffset, BYTE& bOffset, DWORD r, DWORD g, DWORD b)
{
    BYTE minMaskSize = min(getMaskSize(b, bOffset), getMaskSize(g, gOffset), getMaskSize(b, bOffset));
    bOffset += getMaskSize(b, bOffset) - minMaskSize;
    gOffset += getMaskSize(g, gOffset) - minMaskSize;
    rOffset += getMaskSize(r, rOffset) - minMaskSize;
}


BYTE getOffsetForMask(DWORD mask)
{
    BYTE count = 0;
    while((mask & 1) == 0)
    {
        count++;
        mask = (mask >> 1);
    }
    return count;
}

BYTE getMaskSize(DWORD mask, BYTE offset)
{
    mask = (mask >> offset);
    BYTE count = 0;

    while((mask & 1) == 1)
    {
        count++;
        mask = (mask >> 1);
    }
    return count;
}

BYTE getOffsetForLimit(DWORD blueMask, DWORD greenMask, DWORD redMask, 
    BYTE blueMaskOffset, BYTE greenMaskOffset, BYTE redMaskOffset)
{
    BYTE b = getMaskSize(blueMask, blueMaskOffset);
    BYTE g = getMaskSize(greenMask, greenMaskOffset);
    BYTE r = getMaskSize(redMask, redMaskOffset);
    

    return min(b, g, r);
}

template<typename T>
BYTE getMediumColor(void* pixel, DWORD blueMask, DWORD greenMask, DWORD redMask,
    BYTE blueMaskOffset, BYTE greenMaskOffset, BYTE redMaskOffset)
{
    T* p = (T*) pixel;
    T limit = ( ((T) -1) >> ( (sizeof(T)*8) - getOffsetForLimit(blueMask, greenMask, redMask, blueMaskOffset, greenMaskOffset, redMaskOffset)) ) ;
  
    BYTE y = 
        ( ( ( (DWORD)*p & (blueMask))  >> blueMaskOffset ) * 0.0721 //* 0.1
        + ( ( (DWORD)*p & (greenMask)) >> greenMaskOffset) * 0.7154//* 0.59
        + ( ( (DWORD)*p & (redMask))   >> redMaskOffset  ) * 0.2125); //* 0.3 );
    y = (BYTE)( ( ( (T) y) > limit) ? limit : y );
    return y;
}






//For 24 and 32 bit (ignore Alpha-channel), without masks
void pixelIntoGrayScale(void* pixel)
{
    RGBTRIPLE* p = (RGBTRIPLE*) pixel;
    BYTE y = (p->rgbBlue*0.0721 + p->rgbGreen*0.7154 + p->rgbRed*0.2125); ///3;
    p->rgbBlue = y;
    p->rgbGreen = y;
    p->rgbRed = y;
}


//For 16 bit (5-6-5)(5-5-5(-1)), 24 bit, 32 bit(ignore alpha-channel)
template<typename T>
void pixelIntoGrayScale_mask(void* pixel, DWORD blueMask, DWORD greenMask, DWORD redMask,
    BYTE blueMaskOffset, BYTE greenMaskOffset, BYTE redMaskOffset)
{
    T* p = (T*) pixel;
    BYTE y = getMediumColor<T>(pixel,blueMask, greenMask, redMask, blueMaskOffset, greenMaskOffset, redMaskOffset);

    //alpha-channel will be ignored
   // *p = (T) ( (T)(y << blueMaskOffset) | (T)(y << greenMaskOffset) | (T)(y << redMaskOffset) );
    *p =  ((T)(y << blueMaskOffset)  | ( ((T) *p) & ((T)~blueMask)));
	*p =  ((T)(y << greenMaskOffset) | ( ((T) *p) & ((T)~greenMask)));
	*p =  ((T)(y << redMaskOffset)   | ( ((T) *p) & ((T)~redMask)));
}