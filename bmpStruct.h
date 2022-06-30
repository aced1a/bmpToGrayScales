#include <cstdint>


//Маска для изображений с 16 битной глубиной 5-5-5-1
#define REDDEFAULTBITMASK_16BIT     0x7C00
#define GREENDEFAULTBITMASK_16BIT   0x3E0
#define BLUEDEFAULTBITMASK_16BIT    0x001F

typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef int32_t LONG;

/*  Имя констаны            BitCount    хранение пикселей                                   Знак Height

    BI_RGB              -     !=0       двумерный массив                                        +/-
    BI_RLE8             -      8        RLE-кодирование                                          +
    BI_RLE4             -      4        RLE-кодирование                                          +
    BI_BITFIELD         -   16 и 32     двумерный массив с масками цветовых каналов             +/-
    BI_JPG              -      0        во встроенном JPG-файле                                 ?/-
    BI_PNG              -      0        во встроенном PNG-файле                                 ?/-
    BI_ALPHABITFIELDS   -   16 и 32     двумерный массив с масками цветовых и альфа-канала      +/-
*/
enum Compression{
    BI_RGB              =   __MSABI_LONG(0),
    BI_RLE8             =   __MSABI_LONG(1),
    BI_RLE4             =   __MSABI_LONG(2),
    BI_BITFIELDS        =   __MSABI_LONG(3),
    BI_JPG              =   __MSABI_LONG(4),
    BI_PNG              =   __MSABI_LONG(5),
    BI_ALPHABITFIELDS   =   __MSABI_LONG(6)
};


enum class BMPVERSION{
    CORE    =    12,
    V3      =    40,
    V4      =   108,
    V5      =   124
};


#pragma pack(push, 1)
/*
    bfType          -       Сигнатура формата. (BM)
    bfSize          -       Размер файла в байтах
    bfRezerved1     -       Зарезервированно и содержит ноль
    bfRezerved2     -       Зарезервированно и содержит ноль
    bfOffBits       -       Положение массива пикселей относительно начала этой структуры
*/
typedef struct tagBITMAPFILEHEADER
{
    WORD    bfType;
    DWORD   bfSize;
    WORD    bfRezerved1;
    WORD    bfRezerved2;
    DWORD   bfOffBits;
} BITMAPFILEHEADER;



typedef struct tagBITMAPCOREHEADER
{
    DWORD   bcSize;
    WORD    bcWidth;
    WORD    bcHeight;
    WORD    bcPlanes;
    WORD    bcBitCount;
} BITMAPCOREHEADER;

/*
   biSize           -   Размер структуры
   biWidth          -   Ширина картинки в пикселях
   biHeight         -   Высота картинки в пикселях
   biPlanes         -   количество плоскостей
   biBitCount       -   Количество бит на один пиксель
   biCompression    -   Тип сжатия
   biSizeImage      -   размер пиксельных данных в байтах. Может быть обнулено, если не используется сжатие
   biXPelsPerMeter  -   кол-во пикселей на метр по горизонтали
   biYPelsPerMeter  -   кол-во пикселей на метр по вертикали
   biClrUsed        -   кол-во используемых цветов из таблицы. Может быть обнулено, если таблица не используется.
   biClrImportant   -   кол-во важных цветов (необходимых для изображения рисунка)
*/
 typedef struct tagBITMAPINFOHEADER
 {
   DWORD  biSize;
   LONG   biWidth;
   LONG   biHeight;
   WORD   biPlanes;
   WORD   biBitCount;
   DWORD  biCompression;
   DWORD  biSizeImage;
   LONG   biXPelsPerMeter;
   LONG   biYPelsPerMeter;
   DWORD  biClrUsed;
   DWORD  biClrImportant;
 } BITMAPINFOHEADER;




typedef struct tagBITMAPV4HEADER : tagBITMAPINFOHEADER
{
    DWORD        bV4RedMask;
    DWORD        bV4GreenMask;
    DWORD        bV4BlueMask;
    DWORD        bV4AlphaMask;
    DWORD        bV4CSType;
    
    LONG  RedX;          /* X coordinate of red endpoint */
    LONG  RedY;          /* Y coordinate of red endpoint */
    LONG  RedZ;          /* Z coordinate of red endpoint */
    LONG  GreenX;        /* X coordinate of green endpoint */
    LONG  GreenY;        /* Y coordinate of green endpoint */
    LONG  GreenZ;        /* Z coordinate of green endpoint */
    LONG  BlueX;         /* X coordinate of blue endpoint */
    LONG  BlueY;         /* Y coordinate of blue endpoint */
    LONG  BlueZ;         /* Z coordinate of blue endpoint */

    DWORD        bV4GammaRed;
    DWORD        bV4GammaGreen;
    DWORD        bV4GammaBlue;       
} BITMAPV4HEADER;

typedef struct tagBITMAPV5HEADER : tagBITMAPV4HEADER
{
    DWORD bV5Intent;
    DWORD bV5ProfileData;
    DWORD bV5ProfileSize;
    DWORD bV5Reserved;  
} BITMAPV5HEADER;


typedef struct tagRGBQUAD
{
    BYTE rgbBlue;
    BYTE rgbGreen;
    BYTE rgbRed;
    BYTE rgfReserved;
} RGBQUAD;

typedef struct tagRGBTRIPLE
{
    BYTE rgbBlue;
    BYTE rgbGreen;
    BYTE rgbRed;
} RGBTRIPLE;



typedef struct _int24_t
{
    int32_t data:24;
    _int24_t() {}
    _int24_t(int32_t integer) :data(integer) {}
    _int24_t(int8_t integer) :data(integer) {}
    _int24_t(uint32_t integer) :data(integer) {}


    _int24_t operator&(const _int24_t& t) const { return _int24_t(data & t.data); }
    _int24_t operator&(const _int24_t&& t) const { return _int24_t(data & t.data); }
    _int24_t operator|(const _int24_t& t) const { return _int24_t(data | t.data); }

    _int24_t operator>>(const int offset) const { return _int24_t(data >> offset); }

    bool operator>(const _int24_t& t) const { return (data>t.data); }

    _int24_t operator+(_int24_t& t) { return _int24_t(data+t.data); }

    explicit operator uint8_t() { return data; }
    explicit operator int32_t() { return data; }
    explicit operator uint32_t() { return data; }
} int24_t;

#pragma pack(pop)