/*
 * PNG:
 *      Only supports 8-bit Truecolor with Alpha (RGBA).
 *      Not support interlacing.
 *
 * References:
 *
 *      https://www.w3.org/TR/png/
 *
 */

#pragma once

#include "svg.h"

#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

class ImgTool
{
public:
    ImgTool();
    ~ImgTool();

    enum Type {
        PIXEL, // Converts pixel to small squares.
        GROUP  // Converts regions with equal and close pixels into group of small squares.
    };

    // Export processed data in SVG.
    bool exportSVG(std::string path, unsigned outputType = PIXEL);

    // Load file and process binary data from PNG file.
    bool load(std::string path);

    // View details of the processed image in the terminal. If imageData true displays RGBA data.
    void summary(bool imageData = false);

private:

    struct RGBA {
        int R{0}, G{0}, B{0}, A{0};

        RGBA(){};
        RGBA(int r, int g, int b, int a);

        std::string toStr();
    };

    struct IMG {
        bool status{false};                 // state of image preparation (RGBA vector).
        std::string path{};                 // path of input file.
        std::string filename{};             // input file name.
        unsigned long long bytes{0};        // file size.
        unsigned int width{0}, height{0};   // image dimension in pixel.
        unsigned int bitDepth{0};           // number of bits per sample or per palette index (not per pixel).
        unsigned int colorType{0};          // image type (0:Grayscale, 2:Truecolor, 3:Indexed-color, 4:Grayscale Alpha, 6:Truecolor Alpha).
        unsigned int compression{0};        // compression method.
        unsigned int filter{0};             // filter method..
        unsigned int interlace{0};          // interlace method.
        std::vector<RGBA> image{};          // image pixel translated into RGBA.

        std::string toStr(bool imageData);  // text data.
    };

    IMG currentImage;

    // Temporary storage.
    std::vector<bool> visited{};
    std::vector<SVG::Point> connected{};

    // Process separate pixels.
    std::string svgPixel();

    // Process pixel in groups.
    std::string svgRegions();

    // Recursive function.
    void connect(unsigned row, unsigned col, std::string rgbaHex);
};
