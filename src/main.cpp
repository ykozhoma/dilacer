#include <iostream>
#include <stdexcept>
#include "dilacer.h"

int main(int argc, char* argv[])
{
    if (argc < 3) {
        std::cerr << argv[0] << " : not enough arguments!\n";
        std::cerr << "Usage: " << argv[0] << " <input file> <output file> \n";
        return EXIT_FAILURE;
    }

    try
    {
        ykozhoma::JpegDilacer d;
        d.Deinterlace(argv[1], argv[2]);

        //multiple out files test
        //for(size_t i = 0; i < 10; ++i)
        //{
        //    d.Deinterlace(argv[1], std::string_view("out" + std::to_string(i + 1) + ".jpg"));
        //}
    }
    catch(const std::exception& e)
    {
        std::cerr << argv[0] << " failed with: " << e.what();
    }

    return 0;
}
