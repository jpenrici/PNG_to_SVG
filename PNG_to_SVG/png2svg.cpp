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

auto ImgTool::load(std::string path) -> bool
{
    const std::filesystem::path p(path);
    if (!std::filesystem::exists(p)) {
        std::cerr << "File not found!\n";
        return false;
    }

    std::string extension{p.extension()};
    std::transform(extension.begin(), extension.end(), extension.begin(), ::toupper);
    if (!extension.ends_with(".PNG")) {
        std::cerr << "Invalid file!\n";
        return false;
    }

    // Process PNG.
    char *memblock = nullptr;
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
                                IDAT_data.emplace_back(int((unsigned char)valuesOut[j]));
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
                        }
                        else if (pb <= pc) {
                            pr = b;
                        }
                        else {
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
                            }
                            else if (filter_type == 1) {
                                // Sub.
                                Recon_x = Filt_x + Recon_a(row, col);
                            }
                            else if (filter_type == 2) {
                                // Up.
                                Recon_x = Filt_x + Recon_b(row, col);
                            }
                            else if (filter_type == 3) {
                                // Average.
                                Recon_x = Filt_x + (Recon_a(row, col) + Recon_b(row, col)) / 2;
                            }
                            else if (filter_type == 4) {
                                // Paeth.
                                Recon_x = Filt_x + paethPredictor(Recon_a(row, col),
                                                                  Recon_b(row, col),
                                                                  Recon_c(row, col));
                            }
                            else {
                                std::cerr << "Unknown filter type: " << filter_type << '\n';
                                break;
                            }
                            Recon.emplace_back(Recon_x & 0xff);
                        }
                    }

                    // Update data.
                    currentImage.status = (!Recon.empty() && Recon.size() % 4 == 0);
                    if (currentImage.status) {
                        for (unsigned i = 0; i < Recon.size(); i += 4) {
                            currentImage.image.emplace_back(RGBA(Recon[i],
                                                                 Recon[i + 1],
                                                                 Recon[i + 2],
                                                                 Recon[i + 3]));
                        }
                    }
                }
            }

            // Exit.
            delete[] memblock;
        }
    }
    catch (...) {
        std::cerr << "Error processing image!\n";
        return false;
    }

    return currentImage.status;
}

void ImgTool::summary(bool imageData)
{
    std::cout << currentImage.toStr(imageData);
}

auto ImgTool::exportSVG(std::string path, unsigned outputType) -> bool
{
    if (!currentImage.status) {
        std::cerr << "There was something wrong!\n";
        return false;
    }

    // Alert.
    if ((currentImage.height * currentImage.width) > limit) {
        std::cerr << "Algorithm Limit: " << limit << " px\n"
                  << "Image          : " << currentImage.filename
                  << " (" << currentImage.width *currentImage.height << " px). "
                  << "Converting to SVG is not recommended!\n";
        return false;
    }

    std::string svg{};
    switch (outputType) {
    case PIXEL:
        std::cout << "Method: Pixel to Pixel.\n";
        svg = svgPixel();
        break;
    case GROUP:
        std::cout << "Method: Pixel grouped.\n";
        svg = svgGroupPixel();
        break;
    case REGIONS1:
        std::cout << "Method: Regions 1.\n"
                  << "This processing can take a long time!\n";
        svg = svgRegions();
        break;
    case REGIONS2:
        std::cout << "Method: Regions 2.\n"
                  << "This processing can take a long time!\n";
        svg = svgRegions(true);
        break;
    default:
        break;
    }

    if (!svg.empty()) {
        std::cout << "Conversion from PNG to SVG finished!\n"
                  << "Result: " << path << '\n';
    }

    return IO::save(svg, path);
}

auto ImgTool::svgPixel() -> std::string
{
    // Square dimensions.
    int width  = 1;
    int height = 1;
    // Construct SVG.
    std::string figure{};
    auto image = currentImage.image;
    auto rows  = currentImage.height;
    auto cols  = currentImage.width;
    for (unsigned row = 0; row < rows; row++) {
        for (unsigned col = 0; col < cols; col++) {
            auto index = row * cols + col;
            auto pixel = image[index];
            if (pixel.A > 0) {
                figure += draw("p" + std::to_string(index), pixel,
                               rect(Point(col, row), width, height));
            }
        }
    }

    return SVG::svg(cols * width, rows * height, figure, SVG::Metadata());
}

auto ImgTool::svgGroupPixel() -> std::string
{
    // Dimensions.
    int width  = 1;
    int height = 1;
    // Construct SVG.
    int count = 0;
    std::string figure{};
    auto image = currentImage.image;
    auto rows  = currentImage.height;
    auto cols  = currentImage.width;
    for (unsigned row = 0; row < rows; row++) {
        for (unsigned col = 0; col < cols; col++) {
            auto index = row * cols + col;
            auto pixel = image[index];
            if (!pixel.empty()) {
                std::vector<Point> connected;
                connect(connected, image, rows, cols, row, col, pixel, true);
                if (!connected.empty()) {
                    std::string elements{};
                    for (auto &item : connected) {
                        elements += draw("p" + item.toStr(true), pixel,
                                         rect(item, width, height));
                    }
                    if (!elements.empty()) {
                        figure += SVG::group("g" + std::to_string(count++), elements);
                    }
                }
            }
        }
    }

    return SVG::svg(cols * width, rows * height, figure, SVG::Metadata());
}

auto ImgTool::svgRegions(bool onlyVertices) -> std::string
{
    // Dimensions.
    int width  = 1;
    int height = 1;
    // Construct SVG.
    int count = 0;
    std::string figure{};
    auto original = currentImage.image;
    auto copy     = currentImage.image;
    auto rows = currentImage.height;
    auto cols = currentImage.width;
    for (unsigned row = 0; row < rows; row++) {
        for (unsigned col = 0; col < cols; col++) {
            auto index = row * cols + col;
            auto pixel = original[index];
            if (!original[index].empty()) {
                Points connected;
                connect(connected, original, rows, cols, row, col, pixel);
                if (!connected.empty()) {
                    // Analyze Connected Points.
                    Point first = connected.front();
                    std::vector<int> matrix(rows * cols, 0);
                    for (auto item : connected) {
                        matrix[item.Y.value * cols + item.X.value] = 1;
                    }
                    // Find possible edges and gaps.
                    for (unsigned r = 0; r < rows; r++) {
                        bool flag = false;
                        for (unsigned c = 0; c < cols; c++) {
                            int index = r * cols + c;
                            if (matrix[index] == 0 && flag) {
                                flag = false;
                            }
                            if (matrix[index] == 1 && !flag) {
                                matrix[index] = 2;
                                flag = true;
                            }
                        }
                    }
                    for (unsigned r = 0; r < rows; r++) {
                        bool flag = false;
                        for (unsigned c = 0; c < cols; c++) {
                            int index = r * cols + (cols - c - 1);
                            if (matrix[index] == 0 && flag) {
                                flag = false;
                            }
                            if (matrix[index] == 1 && !flag) {
                                matrix[index] = 2;
                                flag = true;
                            }
                        }
                    }
                    for (unsigned c = 0; c < cols; c++) {
                        bool flag = false;
                        for (unsigned r = 0; r < rows; r++) {
                            int index = r * cols + c;
                            if (matrix[index] == 0 && flag) {
                                flag = false;
                            }
                            if (matrix[index] == 1 && !flag) {
                                matrix[index] = 2;
                                flag = true;
                            }
                        }
                    }
                    for (unsigned c = 0; c < cols; c++) {
                        bool flag = false;
                        for (unsigned r = 0; r < rows; r++) {
                            int index = (rows - r - 1) * cols + c;
                            if (matrix[index] == 0 && flag) {
                                flag = false;
                            }
                            if (matrix[index] == 1 && !flag) {
                                matrix[index] = 2;
                                flag = true;
                            }
                        }
                    }
                    // Find possible edges.
                    for (unsigned c = 0; c < cols; c++) {
                        for (unsigned r = 0; r < rows; r++) {
                            int index = r * cols + c;
                            if (matrix[index] > 0) {
                                matrix[index] = 2;
                                break;
                            }
                        }
                    }
                    for (unsigned r = 0; r < rows; r++) {
                        for (unsigned c = 0; c < cols; c++) {
                            int index = r * cols + (cols - c - 1);
                            if (matrix[index] > 0) {
                                matrix[index] = 2;
                                break;
                            }
                        }
                    }
                    for (unsigned c = 0; c < cols; c++) {
                        for (unsigned r = 0; r < rows; r++) {
                            int index = (rows - r - 1) * cols + (cols - c - 1);
                            if (matrix[index] > 0) {
                                matrix[index] = 2;
                                break;
                            }
                        }
                    }
                    for (unsigned r = 0; r < rows; r++) {
                        for (unsigned c = 0; c < cols; c++) {
                            int index = (rows - r - 1) * cols + c;
                            if (matrix[index] > 0) {
                                matrix[index] = 2;
                                break;
                            }
                        }
                    }
                    // Reprocess.
                    auto image = copy;
                    for (unsigned r = 0; r < rows; r++) {
                        for (unsigned c = 0; c < cols; c++) {
                            int index = r * cols + c;
                            if (matrix[index] != 2) {
                                image[index] = RGBA();
                            }
                        }
                    }
                    // SVG.
                    for (unsigned r = 0; r < rows; r++) {
                        for (unsigned c = 0; c < cols; c++) {
                            auto index = r * cols + c;
                            auto pixel = image[index];
                            if (!image[index].empty()) {
                                connected.clear();
                                connect(connected, image, rows, cols, r, c, pixel);
                                if (!connected.empty()) {
                                    std::string elements{};
                                    if (!onlyVertices) {
                                        // Method: Regions 1
                                        for (auto &item : connected) {
                                            elements += draw("p" + item.toStr(true), pixel,
                                                             rect(item, width, height));
                                        }
                                        if (!elements.empty()) {
                                            figure += SVG::group("g" + std::to_string(count++), elements);
                                        }
                                    }
                                    else {
                                        // Method: Regions 2
                                        // Ignore connections smaller than three points.
                                        if (connected.size() == 3) {
                                            figure += draw("p" + connected.front().toStr(true), pixel,
                                                           connected);
                                        }
                                        if (connected.size() > 3) {
                                            Point center; // Midpoint of the region.
                                            Point::average(connected, center);
                                            for (auto &item : connected) {
                                                // Shift right.
                                                if (item.X.value > center.X.value &&
                                                        item.Y.value < center.Y.value) {
                                                    item += Point(width, 0);
                                                }
                                                // Shift down.
                                                else if (item.X.value < center.X.value &&
                                                         item.Y.value > center.Y.value) {
                                                    item += Point(0, height);
                                                }
                                                // Shift right and down.
                                                else if (item.X.value > center.X.value &&
                                                         item.Y.value > center.Y.value) {
                                                    item += Point(width, height);
                                                }
                                                // if item.X.value < center.X.value &&
                                                //    item.Y.value < center.Y.value
                                                // Do nothing!
                                            }
                                            auto vertices = Point::organize(center, connected);
                                            figure += draw("p" + std::to_string(count++), pixel, vertices);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return SVG::svg(cols * width, rows * height, figure, SVG::Metadata());
}

void ImgTool::connect(Points &connected,
                      std::vector<RGBA> &image,
                      unsigned int rows, unsigned int cols,
                      unsigned int row, unsigned int col,
                      RGBA rgba,
                      bool eightDirectional)
{
    if (row < 0 || row > rows || col < 0 || col > cols) {
        return;
    }

    if (image[row * cols + col].empty()) {
        return;
    }

    auto pixel = image[row * cols + col];
    if (pixel.A == 0) {
        return;
    }
    if (!rgba.equal(pixel)) {
        return;
    }

    // Update data
    connected.emplace_back(Point(col, row));
    image[row * cols + col] = RGBA();

    // Recursion
    connect(connected, image, rows, cols, row, col + 1, rgba, eightDirectional);          // right
    connect(connected, image, rows, cols, row + 1, col, rgba, eightDirectional);          // down
    connect(connected, image, rows, cols, row, col - 1, rgba, eightDirectional);          // left
    connect(connected, image, rows, cols, row - 1, col, rgba, eightDirectional);          // up
    if (eightDirectional) {
        connect(connected, image, rows, cols, row + 1, col + 1, rgba, eightDirectional);  // right, down
        connect(connected, image, rows, cols, row + 1, col - 1, rgba, eightDirectional);  // left, down
        connect(connected, image, rows, cols, row - 1, col + 1, rgba, eightDirectional);  // right, up
        connect(connected, image, rows, cols, row - 1, col - 1, rgba, eightDirectional);  // left, up
    }
}

auto ImgTool::rect(Point origin, unsigned int width, unsigned int height) -> Points
{
    return Points {Point(origin.X.value * width, origin.Y.value * height),
                   Point(origin.X.value * width + width, origin.Y.value * height),
                   Point(origin.X.value * width + width, origin.Y.value * height + height),
                   Point(origin.X.value * width, origin.Y.value * height + height),
                   Point(origin.X.value * width, origin.Y.value * height)
                  };
}

auto ImgTool::draw(std::string label, RGBA pixel, Points points) -> std::string
{
    return SVG::polyline(SVG::NormalShape(
                             label,                                     // name.
                             SVG::RGB2HEX(pixel.R, pixel.G, pixel.B),   // fill color.
                             SVG::BLACK,                                // stroke color.
                             0.0,                                       // stroke width
                             pixel.A,                                   // fill opacity.
                             255,                                       // stroke opacity
                             points));                                  // Points
}

auto ImgTool::IMG::toStr(bool imageData = true) -> std::string
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
