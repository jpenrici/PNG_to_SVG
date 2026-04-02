# PNG_to_SVG

A project that converts PNG images to SVG format by mapping each pixel into a colored rectangle.
The PNG file is loaded, processed and exported as SVG for use in graphics editors.

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
├── CMakeLists.txt
└── test_sample.sh
```

---

## Requirements

- CMake 3.20+
- A C++23 compatible compiler (e.g., GCC 13+, Clang 16+, MSVC 2022+)
- Zlib (for image decompression)

> **Note:** If using a manually installed GCC (e.g. GCC 15), the system `libstdc++`
> may be older than required. The project links statically to avoid runtime mismatches:
> `-static-libstdc++ -static-libgcc`

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

The program loads a PNG file, reads it pixel by pixel and exports an SVG file
where each non-transparent pixel becomes a colored `<polyline>` element.

**PNG support:** 8-bit Truecolor with Alpha (RGBA) only. Interlacing not supported.

**Conversion methods:**

| Type | Description |
|------|-------------|
| `0` — PIXEL   | Each pixel becomes an individual square element |
| `1` — GROUP   | Adjacent pixels of equal color are grouped together |
| `2` — REGIONS1 | Experimental — edge detection, borders rendered as pixel groups |
| `3` — REGIONS2 | Experimental — edge detection, borders rendered as vertices |

---

## Usage

```bash
./png2svg png=<input.png> svg=<output.svg>
./png2svg png=<input.png> svg=<output.svg> type=<0|1|2|3>
./png2svg --example
```

**Examples:**

```bash
# Default conversion (pixel to pixel)
./png2svg png=images/test.png svg=output.svg

# Grouped pixels
./png2svg png=images/test.png svg=output.svg type=1

# Run built-in example with all four methods
./png2svg --example
```

---

## Implementation Notes

- PNG chunks parsed following the [W3C PNG spec](https://www.w3.org/TR/png/) — `IHDR`, `IDAT`, `IEND`
- Multiple `IDAT` chunks are concatenated before decompression (correct per spec)
- PNG signature validated with `constexpr std::array`
- All buffers use RAII (`std::vector<std::byte>`) — no raw `new`/`delete`
- Flood fill algorithm (`connect`) uses an iterative stack — no recursion
- SVG output built with `std::format` throughout

---

## References

[SVG](https://www.w3.org/TR/SVG2/) : Documentation with W3C details and specifications.
[PNG](https://www.w3.org/TR/png/) : Documentation with W3C details and specifications.
[ZLIB](http://zlib.net/) : Library that implements the Deflate compression method.

---

## License

This project is provided for educational purposes.
Free to use, modify, and expand with proper attribution.

---

## Display

Result seen in Inkscape.

![display](https://github.com/jpenrici/PNG_to_SVG/blob/main/display/display_inkscape.png)
