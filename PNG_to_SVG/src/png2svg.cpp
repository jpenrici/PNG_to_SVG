#include "png2svg.h"

#include <filesystem>
#include <print>
#include <span>
#include <zlib.h>

ImgTool::ImgTool() { std::println("Image Tool running!"); }
ImgTool::~ImgTool() { std::println("Image Tool stopped!"); }

// PNG chunk structure
struct Chunk {
    std::string type;
    std::vector<std::byte> data;
};

static auto readUint32(std::span<const std::byte> buffer, size_t offset)
    -> uint32_t
{
    return (std::to_integer<uint32_t>(buffer[offset]) << 24) | (std::to_integer<uint32_t>(buffer[offset + 1]) << 16) | (std::to_integer<uint32_t>(buffer[offset + 2]) << 8) | std::to_integer<uint32_t>(buffer[offset + 3]);
}

static auto readChunks(std::span<const std::byte> buffer) -> std::vector<Chunk>
{
    std::vector<Chunk> chunks;
    size_t offset = 8; // skip PNG signature

    while (offset + 8 <= buffer.size()) {
        uint32_t length = readUint32(buffer, offset);
        std::string type(4, '\0');
        for (int i = 0; i < 4; ++i) {
            type[i] = std::to_integer<char>(buffer[offset + 4 + i]);
        }

        offset += 8; // length + type

        if (offset + length > buffer.size())
            break;

        std::vector<std::byte> data(buffer.begin() + offset,
            buffer.begin() + offset + length);
        chunks.push_back({ type, std::move(data) });

        offset += length + 4; // data + CRC
    }

    return chunks;
}

auto ImgTool::load(std::string_view png_path) -> bool
{
    const std::filesystem::path p(png_path);

    if (!std::filesystem::exists(p)) {
        std::println(stderr, "Error loading PNG image file. File not found!");
        return false;
    }

    auto extension = p.extension().string();
    std::ranges::transform(extension, extension.begin(), ::toupper);
    if (extension != ".PNG") {
        std::println(stderr,
            "Error loading PNG image file. Invalid file extension!");
        return false;
    }

    // Read file into buffer
    std::ifstream file(p, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::println(stderr, "Error opening file: {}", png_path);
        return false;
    }

    const auto size = static_cast<size_t>(file.tellg());
    std::vector<std::byte> memblock(size);
    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char*>(memblock.data()), size);
    file.close();

    std::println("Load: {}", png_path);

    // Update data
    currentImage.path = p.parent_path().string();
    currentImage.filename = p.filename().string();
    currentImage.bytes = size;

    // Validate PNG signature
    constexpr std::array<std::byte, 8> PNG_SIGNATURE {
        std::byte { 0x89 }, std::byte { 0x50 }, std::byte { 0x4E }, std::byte { 0x47 },
        std::byte { 0x0D }, std::byte { 0x0A }, std::byte { 0x1A }, std::byte { 0x0A }
    };

    if (!std::equal(PNG_SIGNATURE.begin(), PNG_SIGNATURE.end(),
            memblock.begin())) {
        std::println(stderr, "Invalid PNG signature!");
        return false;
    }

    // Parse chunks
    const auto chunks = readChunks(memblock);

    // Read IHDR
    auto ihdr = std::ranges::find_if(
        chunks, [](const Chunk& c) { return c.type == "IHDR"; });

    if (ihdr == chunks.end() || ihdr->data.size() < 13) {
        std::println(stderr, "IHDR chunk not found or invalid!");
        return false;
    }

    auto span = std::span(ihdr->data);
    currentImage.width = readUint32(span, 0);
    currentImage.height = readUint32(span, 4);
    currentImage.bitDepth = std::to_integer<unsigned>(span[8]);
    currentImage.colorType = std::to_integer<unsigned>(span[9]);
    currentImage.compression = std::to_integer<unsigned>(span[10]);
    currentImage.filter = std::to_integer<unsigned>(span[11]);
    currentImage.interlace = std::to_integer<unsigned>(span[12]);

    // Validate for this algorithm
    currentImage.status = (currentImage.width > 0) && (currentImage.height > 0) && (currentImage.bitDepth == 8) && // 8 bits
        (currentImage.colorType == 6) && // Truecolour with alpha
        (currentImage.compression == 0) && // Standard
        (currentImage.filter == 0) && // Filter method 0
        (currentImage.interlace == 0); // No interlacing

    if (!currentImage.status) {
        std::println(stderr, "PNG format not supported by this algorithm!");
        return false;
    }

    // Collect and decompress IDAT chunks
    constexpr int bytesPerPixel = 4;
    const int expectedLength = currentImage.height * (1 + currentImage.width * bytesPerPixel);

    std::vector<std::byte> compressed;
    for (const auto& chunk : chunks) {
        if (chunk.type == "IDAT") {
            compressed.insert(compressed.end(), chunk.data.begin(), chunk.data.end());
        }
        if (chunk.type == "IEND")
            break;
    }

    std::vector<std::byte> decompressed(expectedLength);
    z_stream infstream {};
    infstream.avail_in = static_cast<uInt>(compressed.size());
    infstream.next_in = reinterpret_cast<Bytef*>(compressed.data());
    infstream.avail_out = static_cast<uInt>(expectedLength);
    infstream.next_out = reinterpret_cast<Bytef*>(decompressed.data());

    if (inflateInit(&infstream) != Z_OK) {
        std::println(stderr, "Failed to initialize zlib!");
        return false;
    }
    inflate(&infstream, Z_NO_FLUSH);
    inflateEnd(&infstream);

    // Decode filters
    std::vector<int> Recon;
    Recon.reserve(currentImage.height * currentImage.width * bytesPerPixel);
    const int stride = currentImage.width * bytesPerPixel;

    auto paethPredictor = [](int a, int b, int c) -> int {
        int p = a + b - c;
        int pa = std::abs(p - a);
        int pb = std::abs(p - b);
        int pc = std::abs(p - c);
        if (pa <= pb && pa <= pc)
            return a;
        if (pb <= pc)
            return b;
        return c;
    };

    auto Recon_a = [&](int r, int c) -> int {
        return c >= bytesPerPixel ? Recon[r * stride + c - bytesPerPixel] : 0;
    };
    auto Recon_b = [&](int r, int c) -> int {
        return r > 0 ? Recon[(r - 1) * stride + c] : 0;
    };
    auto Recon_c = [&](int r, int c) -> int {
        return (r > 0 && c >= bytesPerPixel)
            ? Recon[(r - 1) * stride + c - bytesPerPixel]
            : 0;
    };

    size_t i = 0;
    for (unsigned row = 0; row < currentImage.height; ++row) {
        const int filter_type = std::to_integer<int>(decompressed[i++]);
        for (int col = 0; col < stride; ++col) {
            const int Filt_x = std::to_integer<int>(decompressed[i++]);
            int Recon_x = 0;

            switch (filter_type) {
            case 0:
                Recon_x = Filt_x;
                break;
            case 1:
                Recon_x = Filt_x + Recon_a(row, col);
                break;
            case 2:
                Recon_x = Filt_x + Recon_b(row, col);
                break;
            case 3:
                Recon_x = Filt_x + (Recon_a(row, col) + Recon_b(row, col)) / 2;
                break;
            case 4:
                Recon_x = Filt_x + paethPredictor(Recon_a(row, col), Recon_b(row, col), Recon_c(row, col));
                break;
            [[unlikely]] default:
                std::println(stderr, "Unknown filter type: {}", filter_type);
                return false;
            }
            Recon.push_back(Recon_x & 0xff);
        }
    }

    // Build RGBA image
    currentImage.status = (!Recon.empty() && Recon.size() % 4 == 0);
    if (currentImage.status) {
        currentImage.image.reserve(Recon.size() / 4);
        for (size_t j = 0; j < Recon.size(); j += 4) {
            currentImage.image.emplace_back(
                Color::RGBA(Recon[j], Recon[j + 1], Recon[j + 2], Recon[j + 3]));
        }
    }

    return currentImage.status;
}

void ImgTool::summary(bool imageData)
{
    std::println("{}", currentImage.toStr(imageData));
}

auto ImgTool::exportSVG(std::string_view svg_path, ImgTool::Type outputType)
    -> bool
{
    if (!currentImage.status) {
        std::println(stderr, "No image loaded!");
        return false;
    }

    if (static_cast<long long>(currentImage.image.size()) > limit) {
        std::println(stderr, "Image too large! Limit: {} pixels", limit);
        return false;
    }

    std::string figure;
    switch (outputType) {
    case Type::PIXEL:
        figure = svgPixel();
        break;
    case Type::GROUP:
        figure = svgGroupPixel();
        break;
    case Type::REGIONS1:
        figure = svgRegions();
        break;
    case Type::REGIONS2:
        figure = svgRegions(true);
        break;
    }

    const bool saved = IO::save(figure, std::string(svg_path));
    if (saved) {
        std::println("Exported: {}", svg_path);
    }

    return saved;
}

auto ImgTool::svgPixel() -> std::string
{
    // Square dimensions.
    constexpr int width = 1;
    constexpr int height = 1;

    // Construct SVG.
    const auto rows = currentImage.height;
    const auto cols = currentImage.width;
    const auto& image = currentImage.image;

    std::string figure {};
    figure.reserve(image.size() * 64); // estimated 64 bytes per SVG element

    for (unsigned row = 0; row < rows; ++row) {
        for (unsigned col = 0; col < cols; ++col) {
            const auto index = row * cols + col;
            const auto& pixel = image[index];
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
    // Square dimensions.
    constexpr int width = 1;
    constexpr int height = 1;

    // Construct SVG.
    const auto rows = currentImage.height;
    const auto cols = currentImage.width;
    auto image = currentImage.image; // intentional copy

    int count = 0;
    std::string figure {};
    figure.reserve(image.size() * 64); // estimated 64 bytes per SVG element

    for (unsigned row = 0; row < rows; ++row) {
        for (unsigned col = 0; col < cols; ++col) {
            const auto index = row * cols + col;
            const auto& pixel = image[index];
            if (pixel.empty())
                continue;

            std::vector<Point> connected;
            connect(connected, image, rows, cols, row, col, pixel, true);
            if (connected.empty())
                continue;

            std::string elements;
            elements.reserve(connected.size() * 64); // estimated 64 bytes per SVG element
            for (const auto& item : connected) {
                elements += draw("p" + item.toStr(true), pixel, rect(item, width, height));
            }

            if (!elements.empty()) {
                figure += SVG::group("g" + std::to_string(count++), elements);
            }
        }
    }

    return SVG::svg(cols * width, rows * height, figure, SVG::Metadata());
}

auto ImgTool::svgRegions(bool onlyVertices) -> std::string
{
    // Dimensions.
    constexpr int width = 1;
    constexpr int height = 1;

    // Construct SVG.
    const auto rows = currentImage.height;
    const auto cols = currentImage.width;
    auto original = currentImage.image; // intentional copy — connect modifies
    const auto copy = currentImage.image; // reference for reprocessing

    int count = 0;
    std::string figure;
    figure.reserve(original.size() * 64);

    auto markEdges = [&](std::vector<int>& matrix) {
        // Horizontal scan — left to right
        for (unsigned r = 0; r < rows; ++r) {
            for (unsigned c = 0; c < cols; ++c) {
                if (matrix[r * cols + c] == 1) {
                    matrix[r * cols + c] = 2;
                    break;
                }
            }
        }
        // Horizontal scan — right to left
        for (unsigned r = 0; r < rows; ++r) {
            for (unsigned c = 0; c < cols; ++c) {
                if (matrix[r * cols + (cols - c - 1)] == 1) {
                    matrix[r * cols + (cols - c - 1)] = 2;
                    break;
                }
            }
        }
        // Vertical scan — top to bottom
        for (unsigned c = 0; c < cols; ++c) {
            for (unsigned r = 0; r < rows; ++r) {
                if (matrix[r * cols + c] == 1) {
                    matrix[r * cols + c] = 2;
                    break;
                }
            }
        }
        // Vertical scan — bottom to top
        for (unsigned c = 0; c < cols; ++c) {
            for (unsigned r = 0; r < rows; ++r) {
                if (matrix[(rows - r - 1) * cols + c] == 1) {
                    matrix[(rows - r - 1) * cols + c] = 2;
                    break;
                }
            }
        }
    };

    auto renderRegion1 = [&](const std::vector<Point>& connected,
                             const Color::RGBA& pixel) {
        std::string elements;
        elements.reserve(connected.size() * 64);
        for (const auto& item : connected) {
            elements += draw("p" + item.toStr(true), pixel,
                rect(item, width, height));
        }
        if (!elements.empty()) {
            figure += SVG::group("g" + std::to_string(count++), elements);
        }
    };

    auto renderRegion2 = [&](std::vector<Point> connected,
                             const Color::RGBA& pixel) {
        if (connected.size() < 3)
            return;

        if (connected.size() == 3) {
            figure += draw("p" + connected.front().toStr(true), pixel, connected);
            return;
        }

        Point center;
        Point::average(connected, center);

        for (auto& item : connected) {
            const bool shiftRight = item.X.value > center.X.value;
            const bool shiftDown = item.Y.value > center.Y.value;
            if (shiftRight && !shiftDown)
                item += Point(width, 0);
            else if (!shiftRight && shiftDown)
                item += Point(0, height);
            else if (shiftRight && shiftDown)
                item += Point(width, height);
            // upper left quadrant — no displacement
        }

        auto vertices = Point::organize(center, connected);
        figure += draw("p" + std::to_string(count++), pixel, vertices);
    };

    for (unsigned row = 0; row < rows; ++row) {
        for (unsigned col = 0; col < cols; ++col) {
            const auto& pixel = original[row * cols + col];
            if (pixel.empty())
                continue;

            std::vector<Point> connected;
            connect(connected, original, rows, cols, row, col, pixel);
            if (connected.empty())
                continue;

            std::vector<int> matrix(rows * cols, 0);
            for (const auto& item : connected) {
                matrix[static_cast<int>(item.Y.value) * cols + static_cast<int>(item.X.value)] = 1;
            }

            // Mark edges with gaps and outer edges.
            for (unsigned r = 0; r < rows; ++r) {
                bool flag = false;
                for (unsigned c = 0; c < cols; ++c) {
                    auto& cell = matrix[r * cols + c];
                    if (cell == 0 && flag) {
                        flag = false;
                    }
                    if (cell == 1 && !flag) {
                        cell = 2;
                        flag = true;
                    }
                }
            }
            for (unsigned r = 0; r < rows; ++r) {
                bool flag = false;
                for (unsigned c = 0; c < cols; ++c) {
                    auto& cell = matrix[r * cols + (cols - c - 1)];
                    if (cell == 0 && flag) {
                        flag = false;
                    }
                    if (cell == 1 && !flag) {
                        cell = 2;
                        flag = true;
                    }
                }
            }
            for (unsigned c = 0; c < cols; ++c) {
                bool flag = false;
                for (unsigned r = 0; r < rows; ++r) {
                    auto& cell = matrix[r * cols + c];
                    if (cell == 0 && flag) {
                        flag = false;
                    }
                    if (cell == 1 && !flag) {
                        cell = 2;
                        flag = true;
                    }
                }
            }
            for (unsigned c = 0; c < cols; ++c) {
                bool flag = false;
                for (unsigned r = 0; r < rows; ++r) {
                    auto& cell = matrix[(rows - r - 1) * cols + c];
                    if (cell == 0 && flag) {
                        flag = false;
                    }
                    if (cell == 1 && !flag) {
                        cell = 2;
                        flag = true;
                    }
                }
            }

            markEdges(matrix);

            // Reprocesses with only edge pixels.
            auto image = copy;
            for (unsigned r = 0; r < rows; ++r) {
                for (unsigned c = 0; c < cols; ++c) {
                    if (matrix[r * cols + c] != 2) {
                        image[r * cols + c] = Color::RGBA();
                    }
                }
            }

            // Generate SVG
            for (unsigned r = 0; r < rows; ++r) {
                for (unsigned c = 0; c < cols; ++c) {
                    const auto& px = image[r * cols + c];
                    if (px.empty())
                        continue;

                    connected.clear();
                    connect(connected, image, rows, cols, r, c, px);
                    if (connected.empty())
                        continue;

                    if (!onlyVertices)
                        renderRegion1(connected, px);
                    else
                        renderRegion2(connected, px);
                }
            }
        }
    }

    return SVG::svg(cols * width, rows * height, figure, SVG::Metadata());
}

void ImgTool::connect(std::vector<smalltoolbox::Point>& connected,
    std::vector<Color::RGBA>& image, unsigned rows,
    unsigned cols, unsigned row, unsigned col,
    const Color::RGBA rgba, bool eightDirectional)
{
    // Iterative algorithm for connecting pixels using stack.
    std::vector<std::pair<unsigned, unsigned>> stack;
    stack.reserve(rows * cols);
    stack.emplace_back(row, col);

    while (!stack.empty()) {
        auto [r, c] = stack.back();
        stack.pop_back();

        if (r >= rows || c >= cols)
            continue;

        auto& cell = image[r * cols + c];
        if (cell.empty() || cell.A == 0 || !rgba.equal(cell))
            continue;

        connected.emplace_back(Point(c, r));
        cell = Color::RGBA();

        // Add neighbors
        stack.push_back({ r, c + 1 }); // right
        stack.push_back({ r + 1, c }); // down
        stack.push_back({ r, c - 1 }); // left
        stack.push_back({ r - 1, c }); // up

        if (eightDirectional) {
            stack.push_back({ r + 1, c + 1 }); // right, down
            stack.push_back({ r + 1, c - 1 }); // left, down
            stack.push_back({ r - 1, c + 1 }); // right, up
            stack.push_back({ r - 1, c - 1 }); // left, up
        }
    }
}

auto ImgTool::rect(Point origin, unsigned int width, unsigned int height)
    -> std::vector<Point>
{
    const double x = origin.X.value * width;
    const double y = origin.Y.value * height;
    return { { x, y },
        { x + width, y },
        { x + width, y + height },
        { x, y + height },
        { x, y } };
}

auto ImgTool::draw(std::string label, const Color::RGBA& pixel,
    std::vector<Point> points) -> std::string
{
    return SVG::polyline(
        SVG::NormalShape(label, // name.
            SVG::RGB2HEX(pixel.R, pixel.G, pixel.B), // fill color.
            SVG::BLACK, // stroke color.
            0.0, // stroke width
            pixel.A, // fill opacity.
            255, // stroke opacity
            std::move(points))); // Points
}

auto ImgTool::IMG::toStr(bool imageData = true) -> std::string
{
    static const std::map<unsigned, std::string_view> colourType {
        { 0, "Greyscale" },
        { 2, "Truecolour" },
        { 3, "Indexed-colour" },
        { 4, "Greyscale Alpha" },
        { 6, "Truecolour Alpha" }
    };

    auto it = colourType.find(colorType);
    std::string_view typeName = (it != colourType.end()) ? it->second : "Unknown";

    std::string data {};
    if (imageData) {
        for (auto& v : image) {
            data += v.toStr() + " ";
        }
        data = "\nData       : " + data;
    }

    std::string text {};
    text += "* Details  *";
    text += "\nSource     : " + path;
    text += "\nFilename   : " + filename;
    text += "\nSize       : " + std::to_string(bytes) + " bytes";
    text += "\nWidth      : " + std::to_string(width) + " px";
    text += "\nHeight     : " + std::to_string(height) + " px";
    text += "\nBit depth  : " + std::to_string(bitDepth);
    text += "\nType       : [" + std::to_string(colorType) + "] " + typeName;
    text += "\nCompression: " + std::to_string(compression);
    text += "\nFilter     : " + std::to_string(filter);
    text += "\nInterlace  : " + std::to_string(interlace);
    text += "\nStatus     : " + std::string(status ? "Ok" : "This algorithm does not resolve this image!");
    text += data;
    text += "\n************\n";

    return text;
}
