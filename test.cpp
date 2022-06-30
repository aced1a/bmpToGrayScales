#include <iostream>
#include <iomanip>
#include <bitset>
#include <cmath>

using namespace std;


char getOffsetForMask(unsigned int mask)
{
    char count = 0;
    while((mask & 1) == 0)
    {
        count++;
        mask = (mask >> 1);
    }
    return count;
}

int main()
{
    unsigned int b = 0x1f, g = 0x7e0, r = 0xf800;
    unsigned int c = (255 << 8) + 63, y = 116;
    unsigned int mask = (255 << 8);
    char bO = 0, gO = 0, rO = 0;
    unsigned int a = 16;

    unsigned short limit = ( ((unsigned short)-1) >> (16 - 5) );

    cout << bitset<sizeof(b) * 8>(b) << '\t' << (int) (bO = getOffsetForMask(b)) << endl
         << bitset<sizeof(g) * 8>(g) << '\t' << (int) (getOffsetForMask(g)) << endl
         << bitset<sizeof(r) * 8>(r) << '\t' << (int) (getOffsetForMask(r)) << endl << endl;
    
    cout << bitset<sizeof(c) * 8>(c) << '\t' << bitset<sizeof(mask) * 8>(mask) << endl;
    cout << bitset<sizeof(y) * 8>(y) << endl;

    cout << bitset<16>(limit) << endl;

    c = (y << 8) | (c & ~mask);

    cout << ceil(a / 8) << endl;
    cout << bitset<sizeof(c) * 8>(c) << '\t' << bitset<sizeof(mask) * 8>(mask) << endl;

    return 0;
}