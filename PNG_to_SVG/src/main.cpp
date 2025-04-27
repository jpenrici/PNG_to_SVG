#include "png2svg.h"

#include <iostream>

struct Images {
  std::string svg_path{"./output.svg"};
  int conversion_type{ImgTool::PIXEL};
};

auto process_image(std::string png_path, std::vector<Images> images) -> int {

  try {
    ImgTool img;
    if (img.load(png_path)) {
      img.summary();
      for (const auto &image : images) {
        img.exportSVG(image.svg_path, image.conversion_type);
      }
    }
  } catch (const std::exception &ex) {
    std::cerr << "Error: " << ex.what() << '\n';
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

auto process_image(std::string png_path, std::string svg_path,
                   int conversion_type = ImgTool::PIXEL) -> int {
  return process_image(png_path,
                       std::vector<Images>{{svg_path, conversion_type}});
}

auto test() {
  return process_image("Resources/test.png",
                       {{"Resources/output1_pixel.svg"}, // img.PIXEL
                        {"Resources/output2_group.svg", ImgTool::GROUP},
                        {"Resources/output3_regions1.svg", ImgTool::REGIONS1},
                        {"Resources/output4_regions2.svg", ImgTool::REGIONS2}});
}

auto main(int argc, char *argv[]) -> int {

  std::string png_input;
  std::string svg_output;
  int conversion_type = 0;

  std::string info = {
      "use:\n"
      "    ./png2svg png=<input.png> svg=<output.svg>\n"
      "type= 0 (Pixel), 1 (Group), 2 or 3 (Experimental Regions)\n"
      "    ./png2svg png=<input.png> svg=<output.svg>\n type=1\n"
      "Or example:\n"
      "    ./png2svg --example\n"};

  if (argc < 2) {
    std::cout << info;
    return EXIT_FAILURE;
  }

  if (argc == 2) {
    if (argv[1] == std::string("--example")) {
      return test();
    }
  }

  auto process_argument = [](std::string &arg, const std::string &prefix) {
    if (arg.starts_with(prefix)) {
      arg = arg.substr(prefix.length());
      return true;
    }
    return false;
  };

  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (process_argument(arg, "png=")) {
      png_input = arg;
    }
    if (process_argument(arg, "svg=")) {
      svg_output = arg;
    }
    if (process_argument(arg, "type=")) {
      try {
        conversion_type = std::stoi(arg);
      } catch (const std::exception &ex) {
        std::cerr << "Error: " << ex.what() << '\n';
        return EXIT_FAILURE;
      }
    }
  }

  if (png_input.empty() || svg_output.empty()) {
    std::cout << info;
    return EXIT_FAILURE;
  }

  if (conversion_type < 0 || conversion_type >= sizeof(ImgTool::Type)) {
    std::cout << "Invalid type!\n" << info;
    return EXIT_FAILURE;
  }

  return process_image(png_input, svg_output, conversion_type);
}
