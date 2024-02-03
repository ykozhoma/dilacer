#ifndef DILACER
#define DILACER

#include <stdio.h>
#include <memory>
#include <thread>
#include <map>
#include <forward_list>
#include <string_view>
#include <tuple>
#include <mutex>
#include <condition_variable>

#include "jpeglib.h"

namespace ykozhoma
{
class JpegDilacer
{
public:
    using TSampleArray = std::map<int, std::vector<JSAMPLE>>;
    using TImageInfo = std::tuple<std::string,
                                  int, int, int, J_COLOR_SPACE,
                                  TSampleArray>;
    using TImagePath = std::tuple<std::string, std::string>;

    ~JpegDilacer();

    JpegDilacer();
    JpegDilacer(const JpegDilacer& other) = delete;

public:
    void Deinterlace(std::string_view jpegInPath, std::string_view jpegOutPath);

private:
    void WorkerReadAndDecompress();
    void BlendAndCompress(TImageInfo&& imageInfo);

    void CompressJpeg(FILE* jpegOut, std::vector<JSAMPLE>& buffer);
    TSampleArray DecompressJpeg(FILE* jpegIn);
private:
    std::thread workerThread_;
    bool runState_ = false;

    std::mutex pathMtx_;
    std::condition_variable cv_;
    std::forward_list<TImagePath> jpegPathList_;

    jpeg_decompress_struct decompressedInfo_;
    jpeg_compress_struct compressedInfo_;
    jpeg_error_mgr jErr_;
};
}
#endif
