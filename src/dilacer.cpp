#include "dilacer.h"
#include <stdexcept>
#include <cstring>
#include <vector>
#include <algorithm>
#include <iterator>
#include <future>

namespace ykozhoma
{

JpegDilacer::JpegDilacer()
{
    decompressedInfo_.err = jpeg_std_error(&jErr_);
    jpeg_create_decompress(&decompressedInfo_);

    compressedInfo_.err = jpeg_std_error(&jErr_);
    jpeg_create_compress(&compressedInfo_);

    runState_ = true;
    workerThread_ = std::thread(&JpegDilacer::WorkerReadAndDecompress, this);
}

JpegDilacer::~JpegDilacer()
{
    runState_ = false;

    if (workerThread_.joinable()) workerThread_.join();

    jpeg_destroy_compress(&compressedInfo_);
    jpeg_destroy_decompress(&decompressedInfo_);
}

void JpegDilacer::Deinterlace(std::string_view jpegInPath, std::string_view jpegOutPath)
{
    std::unique_lock<std::mutex> lock(pathMtx_);
    jpegPathList_.emplace_front(JpegDilacer::TImagePath{jpegInPath, jpegOutPath});
    lock.unlock();
    cv_.notify_one();

    {
        std::unique_lock<std::mutex> lock(pathMtx_);
        cv_.wait(lock, [this]{ return jpegPathList_.empty(); });
    }
}

void JpegDilacer::WorkerReadAndDecompress()
{
    while (runState_)
    {
        std::unique_lock<std::mutex> lock(pathMtx_);
        cv_.wait(lock, [this]{ return !jpegPathList_.empty(); });

        const auto filePaths = jpegPathList_.front();

        std::string inFilePath = std::get<0>(filePaths);
        std::string outFilePath = std::get<1>(filePaths);

        jpegPathList_.pop_front();
        lock.unlock();
        cv_.notify_one();

        FILE* jpegIn;

        if ((jpegIn = fopen(inFilePath.c_str(), "rb")) == NULL) {
            jpeg_destroy_decompress(&decompressedInfo_);
            throw std::runtime_error("Could not open input file!\n" );
        }
 
        jpeg_stdio_src(&decompressedInfo_, jpegIn);
        jpeg_read_header(&decompressedInfo_, TRUE);
        jpeg_calc_output_dimensions(&decompressedInfo_);
        decompressedInfo_.out_color_space = JCS_YCbCr;

        auto sampleArray = DecompressJpeg(jpegIn);
        fclose(jpegIn);

        auto compressed = std::async(&JpegDilacer::BlendAndCompress, this,
            JpegDilacer::TImageInfo
            {
                outFilePath,
                decompressedInfo_.image_width,
                decompressedInfo_.image_height,
                decompressedInfo_.output_components,
                decompressedInfo_.out_color_space,
                std::move(sampleArray)
            });

        compressed.wait();
    }
}

void JpegDilacer::BlendAndCompress(TImageInfo&& imageInfo)
{
        const auto& [outFilePath, imageWidth, imageHeight, outComponents, colorSpace, inputBuffer]
            = imageInfo;

        FILE* jpegOut;

        if ((jpegOut = fopen(outFilePath.c_str(), "wb")) == NULL) {
            jpeg_destroy_compress(&compressedInfo_);
            jpeg_destroy_decompress(&decompressedInfo_);
            throw std::runtime_error("Could not open output file!\n" );
        }

        int rowStride = imageWidth * outComponents;
 
        std::vector<JSAMPLE> blendedBuffer;
        blendedBuffer.reserve(rowStride * inputBuffer.size());

        for (size_t i = 0; i < inputBuffer.size(); ++i)
        {
            if (i % 2 == 0)
            {
                blendedBuffer.insert(blendedBuffer.end(), inputBuffer.at(i).begin(), inputBuffer.at(i).end());
            }
            else
            {
                std::transform(inputBuffer.at(i).begin(), inputBuffer.at(i).end(),
                   inputBuffer.at(i - 1).begin(),
                   std::back_inserter(blendedBuffer),
                   [](JSAMPLE c, JSAMPLE p) { return (c + p) >> 1; } );
            }
        }

        jpeg_stdio_dest(&compressedInfo_, jpegOut);

        compressedInfo_.image_width = imageWidth;
        compressedInfo_.image_height = imageHeight;
        compressedInfo_.input_components = outComponents;
        compressedInfo_.in_color_space = colorSpace;

        CompressJpeg(jpegOut, blendedBuffer);

        fclose(jpegOut);
}

void JpegDilacer::CompressJpeg(FILE* jpegOut, std::vector<JSAMPLE>& buffer)
{
    jpeg_set_defaults(&compressedInfo_);
    jpeg_set_quality(&compressedInfo_, 99, TRUE);
    jpeg_start_compress(&compressedInfo_, TRUE);
 
    JSAMPROW rowPtr = buffer.data();
    while (compressedInfo_.next_scanline < compressedInfo_.image_height)
    {
        jpeg_write_scanlines(&compressedInfo_, &rowPtr, 1);
        rowPtr += compressedInfo_.image_width * compressedInfo_.input_components;
    }

    jpeg_finish_compress(&compressedInfo_);
}

JpegDilacer::TSampleArray JpegDilacer::DecompressJpeg(FILE* jpegIn)
{
    int rowStride = decompressedInfo_.output_width * decompressedInfo_.output_components;

    JpegDilacer::TSampleArray inputBuffer;

    jpeg_start_decompress(&decompressedInfo_);
    while (decompressedInfo_.output_scanline < decompressedInfo_.output_height)
    {
        inputBuffer[decompressedInfo_.output_scanline] = std::vector<JSAMPLE>(rowStride);
        JSAMPROW row = inputBuffer[decompressedInfo_.output_scanline].data();

        jpeg_read_scanlines(&decompressedInfo_, &row, 1);
    }
 
    jpeg_finish_decompress(&decompressedInfo_);

    return inputBuffer;
}

} //namespace
