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
        ykozhoma::JpegDilacer::GetInstance(argv[1], argv[2])
        .DeInterlace();
    }
    catch(const std::exception& e)
    {
        std::cerr << argv[0] << " failed with: " << e.what();
    }

    return 0;
}
