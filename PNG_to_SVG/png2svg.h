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

#include "svgToolBox.hpp"

#define RGBA smalltoolbox::Color::RGBA

using smalltoolbox::Point;
using smalltoolbox::SVG;
using smalltoolbox::IO;

class ImgTool {
public:
    ImgTool();
    ~ImgTool();

    enum Type {
        PIXEL,      // Converts pixel to small squares.
        GROUP,      // Converts regions with equal and close pixels into group of small squares.
        REGIONS1,   // Experimental. Simple edge detection. Generated SVG contains border pixels.
        REGIONS2    // Experimental. Simple edge detection. SVG contains pixels grouped into vertices.
    };

    // Export processed data in SVG.
    bool exportSVG(std::string path, unsigned outputType = PIXEL);

    // Load file and process binary data from PNG file.
    bool load(std::string path);

    // View details of the processed image in the terminal. If imageData true displays RGBA data.
    void summary(bool imageData = false);

private:

    struct IMG {
        bool status{false};                 // state of image preparation (RGBA vector).
        std::string path{};                 // path of input file.
        std::string filename{};             // input file name.
        unsigned long long bytes{0};        // file size.
        unsigned int width{0}, height{0};   // image dimension in pixel.
        unsigned int bitDepth{0};           // number of bits per sample or per palette index (not per pixel).
        unsigned int colorType{0};          // image type (0:Grayscale, 2:Truecolor, 3:Indexed-color, 4:Grayscale Alpha, 6:Truecolor Alpha).
        unsigned int compression{0};        // compression method.
        unsigned int filter{0};             // filter method.
        unsigned int interlace{0};          // interlace method.
        std::vector<RGBA> image{};          // image pixel translated into RGBA.

        std::string toStr(bool imageData);  // text data.
    };

    IMG currentImage;
    long long int limit = 256 * 256;

    // Points to draw a pixel.
    Points rect(Point origin, unsigned width, unsigned height);

    // SVG element.
    std::string draw(std::string label, RGBA pixel, Points points);

    // Process separate pixels.
    // Each pixel is converted to a square in SVG.
    std::string svgPixel();

    // Process pixel in groups.
    // Searches for identical pixels to group using recursive algorithm.
    // Each pixel is converted to a square in SVG.
    std::string svgGroupPixel();

    // Process pixel in regions.
    // Searches for identical pixels to group using recursive algorithm.
    // Experimental. Simple algorithm for finding edges.
    std::string svgRegions(bool onlyVertices = false);

    // Recursive function.
    void connect(Points &connected,                 // Temporary storage of nearby points.
                 std::vector<RGBA> &image,          // Temporary matrix or image copy.
                 unsigned rows, unsigned cols,      // Image vector size.
                 unsigned row, unsigned col,        // Starting point.
                 RGBA rgba,                         // Searched color.
                 bool eightDirectional = false);    // Search movement in the matrix.
};
