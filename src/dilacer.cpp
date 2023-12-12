#include "dilacer.h"
#include <stdexcept>
#include <cstring>

namespace ykozhoma
{

JpegDilacer::JpegDilacer(const char* jpegInPath, const char* jpegOutPath)
{
    dInfo_.err = jpeg_std_error(&jErr_);
    jpeg_create_decompress(&dInfo_);

    if ((jpegIn_ = fopen(jpegInPath, "rb")) == NULL) {
        jpeg_destroy_decompress(&dInfo_);
        throw std::runtime_error("Could not open input file!\n" ); 
    }

    jpeg_stdio_src(&dInfo_, jpegIn_);
    jpeg_read_header(&dInfo_, TRUE);
    jpeg_calc_output_dimensions(&dInfo_);
    dInfo_.out_color_space = JCS_YCbCr;

    cInfo_.err = jpeg_std_error(&jErr_);
    jpeg_create_compress(&cInfo_);

    if ((jpegOut_ = fopen(jpegOutPath, "wb")) == NULL) {
        jpeg_destroy_compress(&cInfo_);
        jpeg_destroy_decompress(&dInfo_);
        throw std::runtime_error("Could not open output file!\n" );
    }

    jpeg_stdio_dest(&cInfo_, jpegOut_);

    cInfo_.image_width = dInfo_.image_width;
    cInfo_.image_height = dInfo_.image_height;
    cInfo_.input_components = dInfo_.output_components;
    cInfo_.in_color_space = dInfo_.out_color_space;

    jpeg_set_defaults(&cInfo_);
    jpeg_set_quality(&cInfo_, 99, TRUE);
}

JpegDilacer::~JpegDilacer()
{
    jpeg_destroy_compress(&cInfo_);
    jpeg_destroy_decompress(&dInfo_);

    fclose(jpegOut_); fclose(jpegIn_);
}

void JpegDilacer::DeInterlace()
{
    int rowStride = dInfo_.output_width * dInfo_.output_components;
    JSAMPARRAY buffer = (*dInfo_.mem->alloc_sarray)
            ((j_common_ptr) &dInfo_, JPOOL_IMAGE, rowStride, 2);

    std::memset(buffer[1], 0, rowStride);

    jpeg_start_decompress(&dInfo_);
    jpeg_start_compress(&cInfo_, TRUE);

    jpeg_read_scanlines(&dInfo_, &buffer[0], 1);
    jpeg_write_scanlines(&cInfo_, &buffer[0], 1);
    std::memcpy(buffer[1], buffer[0], (size_t)rowStride);

    while (dInfo_.output_scanline < dInfo_.output_height)
    {
        jpeg_read_scanlines(&dInfo_, &buffer[0], 1);

        BlendLines(buffer[0], buffer[1], rowStride);
        jpeg_write_scanlines(&cInfo_, &buffer[0], 1);

        std::memcpy(buffer[1], buffer[0], (size_t)rowStride);
    }

    jpeg_finish_compress(&cInfo_);
    jpeg_finish_decompress(&dInfo_);
}

void JpegDilacer::BlendLines(JSAMPROW currentLine, JSAMPROW prevLine, int stride)
{
    for (int i = 0; i < stride; ++i)
    {
        currentLine[i] = (currentLine[i] + prevLine[i]) >> 1;
    }
}

} //namespace
