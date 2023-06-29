#include "png2svg.h"

#include <zlib.h>

ImgTool::ImgTool()
{
    std::cout << "Image Tool running!\n";
}

ImgTool::~ImgTool()
{
    std::cout << "Image Tool stoped!\n";
}

bool ImgTool::load(std::string path)
{
    const std::filesystem::path p(path);
    if (!std::filesystem::exists(p)) {
        std::cerr << "File not found!\n";
        return false;
    }

    std::string extension = p.extension();
    std::transform(extension.begin(), extension.end(), extension.begin(), ::toupper);
    if (!extension.ends_with(".PNG")) {
        std::cerr << "Invalid file!\n";
        return false;
    }

    // Process PNG.
    char* memblock = nullptr;
    std::ifstream::pos_type size{0};
    try {
        std::cout << "Load: " << path << '\n';
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        // Open PNG file.
        if (file.is_open()) {
            size = file.tellg();
            memblock = new char[size];
            file.seekg(0, std::ios::beg);
            file.read(memblock, size);
            file.close();
            // Update data.
            currentImage.path = p.parent_path();
            currentImage.filename = p.filename();
            currentImage.bytes = size;
            // Check PNG signature.
            if (std::string(memblock, memblock + 8) == std::string("\x89PNG\r\n\x1a\n")) {
                // Find chunk IHDR.
                for (unsigned i = 0; i < size; i++) {
                    currentImage.status = std::string(memblock + i, memblock + i + 4) == std::string("IHDR");
                    if (currentImage.status) {
                        currentImage.width = int((unsigned char)(memblock[i + 4]) << 24 |
                                                 (unsigned char)(memblock[i + 5]) << 16 |
                                                 (unsigned char)(memblock[i + 6]) << 8  |
                                                 (unsigned char)(memblock[i + 7]));
                        currentImage.height = int((unsigned char)(memblock[i + 8]) << 24 |
                                                  (unsigned char)(memblock[i + 9]) << 16 |
                                                  (unsigned char)(memblock[i + 10]) << 8 |
                                                  (unsigned char)(memblock[i + 11]));
                        currentImage.bitDepth = int((unsigned char)memblock[i + 12]);
                        currentImage.colorType = int((unsigned char)memblock[i + 13]);
                        currentImage.compression =  int((unsigned char)memblock[i + 14]);
                        currentImage.filter = int((unsigned char)memblock[i + 15]);
                        currentImage.interlace = int((unsigned char)memblock[i + 16]);
                        break;
                    }
                }
                // Validate for this algorithm.
                currentImage.status = (currentImage.width > 0) && (currentImage.height > 0) &&
                                      (currentImage.bitDepth == 8) &&       // 8 bits.
                                      (currentImage.colorType == 6) &&      // Truecolour with alpha.
                                      (currentImage.compression == 0) &&    // Standard.
                                      (currentImage.filter == 0) &&         // Filter method 0.
                                      (currentImage.interlace == 0);        // No interlacing.
                int bytesPerPixel = 4;
                int expectedLength = currentImage.height * (1 + currentImage.width * bytesPerPixel);
                // Find chunk IDAT.
                std::vector<int> IDAT_data;
                if (currentImage.status) {
                    for (unsigned i = 0; i < size; i++) {
                        if (std::string(memblock + i, memblock + i + 4) == std::string("IDAT")) {
                            // Data length.
                            unsigned int length = int((unsigned char)(memblock[i - 4]) << 24 |
                                                      (unsigned char)(memblock[i - 3]) << 16 |
                                                      (unsigned char)(memblock[i - 2]) << 8  |
                                                      (unsigned char)(memblock[i - 1]));
                            // Prepare.
                            char *valuesIn, *valuesOut;
                            valuesIn = new char[length + 1];
                            valuesOut = new char[expectedLength + 1];
                            for (unsigned j = 0; j < length; j++) {
                                valuesIn[j] = memblock[i + 4 + j];
                            }
                            // Decompress.
                            z_stream infstream;
                            infstream.zalloc = Z_NULL;
                            infstream.zfree  = Z_NULL;
                            infstream.opaque = Z_NULL;
                            infstream.avail_in  = length;
                            infstream.next_in   = (Bytef *) valuesIn;
                            infstream.avail_out = expectedLength;
                            infstream.next_out  = (Bytef *) valuesOut;
                            inflateInit(&infstream);
                            inflate(&infstream, Z_NO_FLUSH);
                            inflateEnd(&infstream);
                            // Store.
                            for (unsigned j = 0; j < expectedLength; j++) {
                                IDAT_data.push_back(int((unsigned char)valuesOut[j]));
                            }
                        }
                        if (std::string(memblock + i, memblock + i + 4) == std::string("IEND")) {
                            break;
                        }
                    }

                    // Decode.
                    std::vector<int> Recon; // Storage of reconstructed values.
                    int stride = currentImage.width * bytesPerPixel;

                    auto paethPredictor = [](int a, int b, int c) {
                        int p = a + b - c;
                        int pa = abs(p - a);
                        int pb = abs(p - b);
                        int pc = abs(p - c);
                        int pr = 0;
                        if (pa <= pb && pa <= pc) {
                            pr = a;
                        } else if (pb <= pc) {
                            pr = b;
                        } else {
                            pr = c;
                        }
                        return pr;
                    };

                    auto Recon_a = [&](int r, int c) {
                        return c >= bytesPerPixel ? Recon[r * stride + c - bytesPerPixel] : 0;
                    };

                    auto Recon_b = [&](int r, int c) {
                        return r > 0 ? Recon[(r - 1) * stride + c] : 0;
                    };

                    auto Recon_c = [&](int r, int c) {
                        return r > 0 && c >= bytesPerPixel ? Recon[(r - 1) * stride + c - bytesPerPixel] : 0;
                    };

                    int i = 0;
                    int Recon_x = 0;
                    for (unsigned row = 0; row < currentImage.height; row++) {
                        int filter_type = IDAT_data[i++];
                        for (unsigned col = 0; col < stride; col++) {
                            int Filt_x = IDAT_data[i++];
                            if (filter_type == 0) {
                                // None.
                                Recon_x = Filt_x;
                            } else if (filter_type == 1) {
                                // Sub.
                                Recon_x = Filt_x + Recon_a(row, col);
                            } else if (filter_type == 2) {
                                // Up.
                                Recon_x = Filt_x + Recon_b(row, col);
                            } else if (filter_type == 3) {
                                // Average.
                                Recon_x = Filt_x + (Recon_a(row, col) + Recon_b(row, col)) / 2;
                            } else if (filter_type == 4) {
                                // Paeth.
                                Recon_x = Filt_x + paethPredictor(Recon_a(row, col), Recon_b(row, col), Recon_c(row, col));
                            } else {
                                std::cerr << "Unknown filter type: " << filter_type << '\n';
                                break;
                            }
                            Recon.push_back(Recon_x & 0xff);
                        }
                    }

                    // Update data.
                    currentImage.status = (!Recon.empty() && Recon.size() % 4 == 0);
                    if (currentImage.status) {
                        for (unsigned i = 0; i < Recon.size(); i += 4) {
                            currentImage.image.push_back(RGBA(Recon[i], Recon[i + 1], Recon[i + 2], Recon[i + 3]));
                        }
                    }
                }
            }

            // Exit.
            delete[] memblock;
        }
    } catch (...) {
        std::cerr << "Error processing image!\n";
        return false;
    }

    return currentImage.status;
}

void ImgTool::summary(bool imageData)
{
    std::cout << currentImage.toStr(imageData);
}

bool ImgTool::exportSVG(std::string path, unsigned outputType)
{
    if (!currentImage.status) {
        std::cerr << "There was something wrong!\n";
        return false;
    }

    std::string svg{};
    switch (outputType) {
    case PIXEL:
        svg = svgPixel();
        break;
    case GROUP:
        svg = svgGroupPixel();
        break;
    default:
        break;
    }

    return SVG::save(svg, path);
}

std::string ImgTool::svgPixel()
{
    // Alert.
    int limit = 256 * 256;
    if ((currentImage.height * currentImage.width) > limit) {
        std::cerr << "Function       : svgPixel()\n"
                  << "Algorithm Limit: " << limit << " px\n"
                  << "Image          : " << currentImage.filename
                  << " (" << currentImage.width * currentImage.height << " px). "
                  << "Converting to SVG is not recommended!\n";
        return {};
    }

    // Square dimensions.
    int width  = 1;
    int height = 1;
    // Construct SVG.
    std::string figure = "";
    for (unsigned row = 0; row < currentImage.height; row++) {
        for (unsigned col = 0; col < currentImage.width; col++) {
            int index = row * currentImage.width + col;
            auto pixel = currentImage.image[index];
            auto color = SVG::RGB2HEX(pixel.R, pixel.G, pixel.B);
            figure += SVG::polyline(SVG::Shape(
                "PX_" + std::to_string(index),      // name.
                color,                              // fill color.
                BLACK,                              // stroke color.
                pixel.A,                            // fill opacity.
                0.0,                                // stroke width
                255,                                // stroke opacity
                std::vector<SVG::Point>{            // points.
                    SVG::Point(col * width, row * height),
                    SVG::Point(col * width + width, row * height),
                    SVG::Point(col * width + width, row * height + height),
                    SVG::Point(col * width, row * height + height),
                    SVG::Point(col * width, row * height)
                }));
        }
    }

    return SVG::svg(currentImage.width * width, currentImage.height * height, figure,
                    SVG::Metadata());
}

std::string ImgTool::svgGroupPixel()
{
    // Alert.
    int limit = 256 * 256;
    if ((currentImage.height * currentImage.width) > limit) {
        std::cerr << "Function       : svgGroupPixel()\n"
                  << "Algorithm Limit: " << limit << " px\n"
                  << "Image          : " << currentImage.filename
                  << " (" << currentImage.width * currentImage.height << " px). "
                  << "Converting to SVG is not recommended!\n";
        return {};
    }

    // Dimensions.
    int width  = 1;
    int height = 1;
    // Construct SVG.
    int count = 0;
    std::string figure = "";
    visited = std::vector<bool>(currentImage.height * currentImage.width, false);
    for (unsigned row = 0; row < currentImage.height; row++) {
        for (unsigned col = 0; col < currentImage.width; col++) {
            if (!visited[row * currentImage.width + col]) {
                connected.clear();
                auto pixel = currentImage.image[row * currentImage.width + col];
                connect(row, col, SVG::RGBA2HEX(pixel.R, pixel.G, pixel.B, pixel.A));
                std::string elements{};
                for (auto& item : connected) {
                    auto index = item.y * currentImage.width + item.x;
                    auto pixel = currentImage.image[index];
                    auto color = SVG::RGB2HEX(pixel.R, pixel.G, pixel.B);
                    elements += SVG::polyline(SVG::Shape(
                        "PX_" + std::to_string((int)index), // name.
                        color,                              // fill color.
                        BLACK,                              // stroke color.
                        pixel.A,                            // fill opacity.
                        0.0,                                // stroke width.
                        255,                                // stroke opacity.
                        std::vector<SVG::Point>{            // points.
                            SVG::Point(item.x * width, item.y * height),
                            SVG::Point(item.x * width + width, item.y * height),
                            SVG::Point(item.x * width + width, item.y * height + height),
                            SVG::Point(item.x * width, item.y * height + height),
                            SVG::Point(item.x * width, item.y * height)
                        }));
                }
                if (!elements.empty()) {
                    figure += SVG::group("Group_" + std::to_string(count++), elements);
                }
            }
        }
    }

    return SVG::svg(currentImage.width * width, currentImage.height * height, figure,
                    SVG::Metadata());
}

void ImgTool::connect(unsigned row, unsigned col, std::string rgbaHex)
{
    if (row < 0 || row > currentImage.height || col < 0 || col > currentImage.width) {
        return;
    }

    if (visited[row * currentImage.width + col]) {
        return;
    }

    auto pixel = currentImage.image[row * currentImage.width + col];
    if (rgbaHex != SVG::RGBA2HEX(pixel.R, pixel.G, pixel.B, pixel.A)) {
        return;
    }

    // Update data
    connected.push_back(SVG::Point(col, row));
    visited[row * currentImage.width + col] = true;

    // Recursion
    connect(row, col + 1, rgbaHex);      // right
    connect(row + 1, col, rgbaHex);      // down
    connect(row, col - 1, rgbaHex);      // left
    connect(row - 1, col, rgbaHex);      // up
    connect(row + 1, col + 1, rgbaHex);  // right, down
    connect(row + 1, col - 1, rgbaHex);  // left, down
    connect(row - 1, col + 1, rgbaHex);  // right, up
    connect(row - 1, col - 1, rgbaHex);  // left, up
}

ImgTool::RGBA::RGBA(int r, int g, int b, int a)
{
    R = r < 0 ? 0 : r % 256;
    G = g < 0 ? 0 : g % 256;
    B = b < 0 ? 0 : b % 256;
    A = a < 0 ? 0 : a % 256;
}

std::string ImgTool::RGBA::toStr()
{
    return {
        std::to_string(R) + "," +
        std::to_string(G) + "," +
        std::to_string(B) + "," +
        std::to_string(A)
    };
}

std::string ImgTool::IMG::toStr(bool imageData = true)
{
    std::vector<std::string> colourType {
        "Greyscale",            // 0
        "",                     // Empty
        "Truecolour",           // 2
        "Indexed-colour",       // 3
        "Greyscale Alpha",      // 4
        "",                     // Empty
        "Truecolour Alpha"      // 6
    };

    std::string data{};
    if (imageData) {
        for (auto &v : image) {
            data += v.toStr() + " ";
        }
        data = "\nData       : " + data;
    }

    std::string text{};
    text += "* Details  *";
    text += "\nSource     : " + path;
    text += "\nFilename   : " + filename;
    text += "\nSize       : " + std::to_string(bytes) + " bytes";
    text += "\nWidth      : " + std::to_string(width) + " px";
    text += "\nHeight     : " + std::to_string(height) + " px";
    text += "\nBit depth  : " + std::to_string(bitDepth);
    text += "\nType       : [" + std::to_string(colorType) + "] " + colourType[colorType < 0 || colorType > colourType.size() ? 2 : colorType];
    text += "\nCompression: " + std::to_string(compression);
    text += "\nFilter     : " + std::to_string(filter);
    text += "\nInterlace  : " + std::to_string(interlace);
    text += "\nStatus     : " + std::string(status ? "Ok" : "This algorithm does not resolve this image!");
    text += data;
    text += "\n************\n";

    return  text;
}
