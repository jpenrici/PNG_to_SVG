#include "png2svg.h"

#include <print>

struct Images {
  std::string_view svg_path{"./output.svg"};
  ImgTool::Type conversion_type{ImgTool::Type::PIXEL};
};

auto process_image(std::string_view png_path, std::vector<Images> images)
    -> int {

  try {
    ImgTool img;
    if (img.load(png_path)) {
      img.summary();
      for (const auto &image : images) {
        img.exportSVG(image.svg_path, image.conversion_type);
      }
    }
  } catch (const std::exception &ex) {
    std::println(stderr, "Error: {}", ex.what());
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

auto process_image(std::string_view png_path, std::string_view svg_path,
                   ImgTool::Type conversion_type = ImgTool::Type::PIXEL)
    -> int {
  return process_image(png_path,
                       std::vector<Images>{{svg_path, conversion_type}});
}

auto test() {
  return process_image(
      "Resources/test.png",
      {{"Resources/output1_pixel.svg"}, // img.PIXEL
       {"Resources/output2_group.svg", ImgTool::Type::GROUP},
       {"Resources/output3_regions1.svg", ImgTool::Type::REGIONS1},
       {"Resources/output4_regions2.svg", ImgTool::Type::REGIONS2}});
}

auto main(int argc, char *argv[]) -> int {

  std::string png_input;
  std::string svg_output;
  int arg_type = 0;

  constexpr std::string_view info = {
      "use:\n"
      "    ./png2svg png=<input.png> svg=<output.svg>\n"
      "type= 0 (Pixel), 1 (Group), 2 or 3 (Experimental Regions)\n"
      "    ./png2svg png=<input.png> svg=<output.svg>\n type=1\n"
      "Or example:\n"
      "    ./png2svg --example\n"};

  if (argc < 2) {
    std::println(stderr, "{}", info);
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
        arg_type = std::stoi(arg);
      } catch (const std::exception &ex) {
        std::println(stderr, "Error: {}", ex.what());
        return EXIT_FAILURE;
      }
    }
  }

  if (png_input.empty() || svg_output.empty()) {
    std::println(stderr, "{}", info);
    return EXIT_FAILURE;
  }

  auto conversion_type = static_cast<ImgTool::Type>(arg_type);
  if (arg_type < 0 || conversion_type > ImgTool::Type::REGIONS2) {
    std::println(stderr, "Invalid type!\n{}", info);
    return EXIT_FAILURE;
  }

  return process_image(png_input, svg_output, conversion_type);
}
