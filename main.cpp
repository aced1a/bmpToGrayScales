#include "bmp.h"

using namespace std;

char* in_file;
char* out_file;

void usage()
{
    cout << "Usage: program input_file_path  output_file_path\n";   
}

int main(int argc, char* argv[])
{
    try
    {
        if(argc < 3)
        {
            usage();
            return 1;
        }

        in_file = argv[1];
        out_file = argv[2];

        BMP image(in_file);
        image.intoGrayScales();
        image.writeBMPFile(string{out_file});
       
        return 0;
    }
    catch(exception& c)
    {
        cerr << c.what() << endl;
        return 2;
    }
    catch(bmp_error c)
    {
        cerr << "BMP ERROR: "<< c.name << endl;
        return 3;
    }
}