#ifndef DILACER
#define DILACER

#include <stdio.h>
#include <memory>
#include "jpeglib.h"

namespace ykozhoma
{
class JpegDilacer
{
public:
    static JpegDilacer& GetInstance(const char* jpegInPath, const char* jpegOutPath) {
        static auto sInstance = std::unique_ptr<JpegDilacer>
        (new JpegDilacer(jpegInPath, jpegOutPath));
        return *sInstance;
    }

    ~JpegDilacer();

private:
    JpegDilacer() = delete;
    JpegDilacer(const JpegDilacer& other) = delete;
    JpegDilacer(const char* jpegInPath, const char* jpegOutPath);

public:
    void DeInterlace();

private:
    void BlendLines(JSAMPROW currentLine, JSAMPROW prevLine, int stride);

private:
    FILE* jpegIn_;
    FILE* jpegOut_;

    jpeg_decompress_struct dInfo_;
    jpeg_compress_struct cInfo_;
    jpeg_error_mgr jErr_;
};
}
#endif
