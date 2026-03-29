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
    -> uint32_t {
  return (std::to_integer<uint32_t>(buffer[offset]) << 24) |
         (std::to_integer<uint32_t>(buffer[offset + 1]) << 16) |
         (std::to_integer<uint32_t>(buffer[offset + 2]) << 8) |
         std::to_integer<uint32_t>(buffer[offset + 3]);
}

static auto readChunks(std::span<const std::byte> buffer)
    -> std::vector<Chunk> {
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
    chunks.push_back({type, std::move(data)});

    offset += length + 4; // data + CRC
  }

  return chunks;
}

auto ImgTool::load(std::string_view png_path) -> bool {
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
  file.read(reinterpret_cast<char *>(memblock.data()), size);
  file.close();

  std::println("Load: {}", png_path);

  // Update data
  currentImage.path = p.parent_path().string();
  currentImage.filename = p.filename().string();
  currentImage.bytes = size;

  // Validate PNG signature
  constexpr std::array<std::byte, 8> PNG_SIGNATURE{
      std::byte{0x89}, std::byte{0x50}, std::byte{0x4E}, std::byte{0x47},
      std::byte{0x0D}, std::byte{0x0A}, std::byte{0x1A}, std::byte{0x0A}};

  if (!std::equal(PNG_SIGNATURE.begin(), PNG_SIGNATURE.end(),
                  memblock.begin())) {
    std::println(stderr, "Invalid PNG signature!");
    return false;
  }

  // Parse chunks
  const auto chunks = readChunks(memblock);

  // Read IHDR
  auto ihdr = std::ranges::find_if(
      chunks, [](const Chunk &c) { return c.type == "IHDR"; });

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
  currentImage.status =
      (currentImage.width > 0) && (currentImage.height > 0) &&
      (currentImage.bitDepth == 8) &&    // 8 bits
      (currentImage.colorType == 6) &&   // Truecolour with alpha
      (currentImage.compression == 0) && // Standard
      (currentImage.filter == 0) &&      // Filter method 0
      (currentImage.interlace == 0);     // No interlacing

  if (!currentImage.status) {
    std::println(stderr, "PNG format not supported by this algorithm!");
    return false;
  }

  // Collect and decompress IDAT chunks
  constexpr int bytesPerPixel = 4;
  const int expectedLength =
      currentImage.height * (1 + currentImage.width * bytesPerPixel);

  std::vector<std::byte> compressed;
  for (const auto &chunk : chunks) {
    if (chunk.type == "IDAT") {
      compressed.insert(compressed.end(), chunk.data.begin(), chunk.data.end());
    }
    if (chunk.type == "IEND")
      break;
  }

  std::vector<std::byte> decompressed(expectedLength);
  z_stream infstream{};
  infstream.avail_in = static_cast<uInt>(compressed.size());
  infstream.next_in = reinterpret_cast<Bytef *>(compressed.data());
  infstream.avail_out = static_cast<uInt>(expectedLength);
  infstream.next_out = reinterpret_cast<Bytef *>(decompressed.data());

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
        Recon_x = Filt_x + paethPredictor(Recon_a(row, col), Recon_b(row, col),
                                          Recon_c(row, col));
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

void ImgTool::summary(bool imageData) {
  std::println("{}", currentImage.toStr(imageData));
}

auto ImgTool::exportSVG(std::string_view svg_path, ImgTool::Type outputType)
    -> bool {
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

auto ImgTool::svgPixel() -> std::string {
  // Square dimensions.
  int width = 1;
  int height = 1;
  // Construct SVG.
  std::string figure{};
  auto image = currentImage.image;
  auto rows = currentImage.height;
  auto cols = currentImage.width;
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

auto ImgTool::svgGroupPixel() -> std::string {
  // Dimensions.
  int width = 1;
  int height = 1;
  // Construct SVG.
  int count = 0;
  std::string figure{};
  auto image = currentImage.image;
  auto rows = currentImage.height;
  auto cols = currentImage.width;
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
            elements +=
                draw("p" + item.toStr(true), pixel, rect(item, width, height));
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

auto ImgTool::svgRegions(bool onlyVertices) -> std::string {
  // Dimensions.
  int width = 1;
  int height = 1;
  // Construct SVG.
  int count = 0;
  std::string figure{};
  auto original = currentImage.image;
  auto copy = currentImage.image;
  auto rows = currentImage.height;
  auto cols = currentImage.width;
  for (unsigned row = 0; row < rows; row++) {
    for (unsigned col = 0; col < cols; col++) {
      auto index = row * cols + col;
      auto pixel = original[index];
      if (!original[index].empty()) {
        std::vector<Point> connected;
        connect(connected, original, rows, cols, row, col, pixel);
        if (!connected.empty()) {
          // Analyze Connected Points.
          Point first = connected.front();
          std::vector<int> matrix(rows * cols, 0);
          for (const auto &item : connected) {
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
                image[index] = Color::RGBA();
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
                      figure +=
                          SVG::group("g" + std::to_string(count++), elements);
                    }
                  } else {
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
                      figure +=
                          draw("p" + std::to_string(count++), pixel, vertices);
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

void ImgTool::connect(std::vector<smalltoolbox::Point> &connected,
                      std::vector<Color::RGBA> &image, unsigned rows,
                      unsigned cols, unsigned row, unsigned col,
                      Color::RGBA rgba, bool eightDirectional) {
  // Iterative algorithm for connecting pixels using stack.
  std::vector<std::pair<unsigned, unsigned>> stack;
  stack.push_back({row, col});

  while (!stack.empty()) {
    auto [r, c] = stack.back();
    stack.pop_back();

    if (r >= rows || c >= cols || image[r * cols + c].empty() ||
        image[r * cols + c].A == 0 || !rgba.equal(image[r * cols + c])) {
      continue;
    }

    connected.emplace_back(Point(c, r));
    image[r * cols + c] = Color::RGBA();

    // Add neighbors
    stack.push_back({r, c + 1}); // right
    stack.push_back({r + 1, c}); // down
    stack.push_back({r, c - 1}); // left
    stack.push_back({r - 1, c}); // up

    if (eightDirectional) {
      stack.push_back({r + 1, c + 1}); // right, down
      stack.push_back({r + 1, c - 1}); // left, down
      stack.push_back({r - 1, c + 1}); // right, up
      stack.push_back({r - 1, c - 1}); // left, up
    }
  }
}

auto ImgTool::rect(Point origin, unsigned int width, unsigned int height)
    -> std::vector<Point> {
  return std::vector<Point>{
      Point(origin.X.value * width, origin.Y.value * height),
      Point(origin.X.value * width + width, origin.Y.value * height),
      Point(origin.X.value * width + width, origin.Y.value * height + height),
      Point(origin.X.value * width, origin.Y.value * height + height),
      Point(origin.X.value * width, origin.Y.value * height)};
}

auto ImgTool::draw(std::string label, Color::RGBA pixel,
                   std::vector<Point> points) -> std::string {
  return SVG::polyline(
      SVG::NormalShape(label,                                   // name.
                       SVG::RGB2HEX(pixel.R, pixel.G, pixel.B), // fill color.
                       SVG::BLACK,                              // stroke color.
                       0.0,                                     // stroke width
                       pixel.A,                                 // fill opacity.
                       255,      // stroke opacity
                       points)); // Points
}

auto ImgTool::IMG::toStr(bool imageData = true) -> std::string {
  std::vector<std::string> colourType{
      "Greyscale",       // 0
      "",                // Empty
      "Truecolour",      // 2
      "Indexed-colour",  // 3
      "Greyscale Alpha", // 4
      "",                // Empty
      "Truecolour Alpha" // 6
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
  text +=
      "\nType       : [" + std::to_string(colorType) + "] " +
      colourType[colorType < 0 || colorType > colourType.size() ? 2
                                                                : colorType];
  text += "\nCompression: " + std::to_string(compression);
  text += "\nFilter     : " + std::to_string(filter);
  text += "\nInterlace  : " + std::to_string(interlace);
  text += "\nStatus     : " +
          std::string(status ? "Ok"
                             : "This algorithm does not resolve this image!");
  text += data;
  text += "\n************\n";

  return text;
}
