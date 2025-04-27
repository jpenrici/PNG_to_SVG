# PNG_to_SVG

A project that converts PNG images to SVG format by mapping each pixel into a colored rectangle.<br>
In this algorithm, the PNG file is loaded, processed and exported as SVG for use in graphics editors.

---

## Project Structure

```
PNG_to_SVG/
├── include/
│   ├── png2svg.h
│   └── svgToolBox.hpp
├── src/
│   ├── png2svg.cpp
│   └── main.cpp
├── images/
│   └── test.png
└── CMakeLists.txt
```

---

## Requirements

- CMake 3.20+
- A C++23 compatible compiler (e.g., GCC 13+, Clang 16+, MSVC 2022+)
- Zlib (for image decompression)

---

## Building

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

---

## How It Works

- The program loads a PNG file (Resources/image.png).
- It reads the image pixel by pixel.
- Each pixel is translated into an SVG <rect> with the corresponding color.
- Fully transparent pixels are ignored.
- The generated file output.svg preserves the color pattern of the input image.

---

## Usage

Simply run the executable:
```
./png2svg png=<input.png> svg=<output.svg>
```

---

## References

[SVG](https://www.w3.org/TR/SVG2/) : Documentation with W3C details and specifications.</br>
[PNG](https://www.w3.org/TR/png/) : Documentation with W3C details and specifications.</br>
[ZLIB](http://zlib.net/) : Library that implements the Deflate compression method.</br>

---

## License

This project is provided for educational purposes.<br>
Free to use, modify, and expand with proper attribution.

---

## Display

Result seen in Inkscape.<br>

![display](https://github.com/jpenrici/PNG_to_SVG/blob/main/display/display_inkscape.png)
