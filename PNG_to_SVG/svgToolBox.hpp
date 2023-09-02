////////////////////////////////////////////////////////////////////////////////
// svgToolBox.hpp
// Small tools for building applications from SVG images.
// Version with unused functions removed.
////////////////////////////////////////////////////////////////////////////////

#ifndef SMALLTOOLBOX_H_
#define SMALLTOOLBOX_H_

#include <algorithm>
#include <cmath>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace smalltoolbox {

#define Numbers     std::vector<double>
#define Points      std::vector<smalltoolbox::Point>
#define Strings     std::vector<std::string>
#define PI          std::numbers::pi

class Text {

public:

    // Trim string : Remove characters to the right.
    static auto rtrim(std::string str, char trimmer) -> std::string
    {
        int left = 0;
        auto right = str.size() - 1;
        right = right < 0 ? 0 : right;
        while (right >= 0) {
            if (str[right] != trimmer) {
                break;
            }
            right--;
        }

        return str.substr(left, 1 + right - left);
    }

    // Trim zeros: Formats numbers.
    static auto trimZeros(double value) -> std::string
    {
        auto str = std::to_string(value);
        return Text::rtrim(str, '0') + '0';
    }

};

class Math {

public:

    static auto angle(double radians) -> double
    {
        return radians * 180.0 / PI;
    }

    // Returns the angle of the line (x0,y0)(x1,y1).
    static auto angle(double x0, double y0, double x1, double y1) -> double
    {
        double result = angle(atan((y1 - y0) / (x1 - x0)));

        if (x0 == x1 && y0 == y1) {
            result = 0;
        }
        if (x0 <  x1 && y0 == y1) {
            result = 0;
        }
        if (x0 == x1 && y0 <  y1) {
            result = 90;
        }
        if (x0 >  x1 && y0 == y1) {
            result = 180;
        }
        if (x0 == x1 && y0 >  y1) {
            result = 270;
        }
        if (x0 >  x1 && y0 <  y1) {
            result += 180;
        }
        if (x0 >  x1 && y0 >  y1) {
            result += 180;
        }
        if (x0 <  x1 && y0 >  y1) {
            result += 360;
        }

        return result;
    }

};

class Check {

public:

    // Compare groups (vector).
    template<typename T>
    static auto equal(std::vector<T> group1, std::vector<T> group2, bool compareOrder = false) -> bool
    {
        if (group1.size() != group2.size()) {
            return false;
        }

        if (compareOrder) {
            for (unsigned i = 0; i < group1.size(); i++) {
                if (!(group1[i] == group2[i])) {
                    return false;
                }
            }
        }

        for (auto value1 : group1) {
            bool differentFromEveryone = true;
            for (auto value2 : group2) {
                if (value1 == value2) {
                    differentFromEveryone = false;
                    break;
                }
            }
            if (differentFromEveryone) {
                return false;
            }
        }

        return true;
    }

};

class IO {

public:

    // Save text file.
    static auto save(const std::string &text, std::string filePath) -> bool
    {
        if (text.empty()) {
            std::cerr << "Empty text! Export failed!\n";
            return false;
        }

        if (filePath.empty()) {
            filePath = "output.txt";
        }

        try {
            std::ofstream file(filePath, std::ios::out);
            file << text;
            file.close();
        }
        catch (const std::exception &e) {
            std::cout << "Error handling file writing.\n";
            std::cerr << e.what() << "\n";
            return false;
        }

        return true;
    }

};

// Point 2D (x,y)
class Point {

    struct Coordinate {
        double value{0};

        auto toStr(bool trimZeros = false) const -> std::string
        {
            return trimZeros ? Text::trimZeros(value) : std::to_string(value);
        }
    };

public:

    // Axis
    Coordinate X, Y;

    // Label
    std::string label{"Point"};

    // Point (0, 0)
    Point() : X{0}, Y{0} {};

    // Point (x, y)
    Point(double x, double y) : X{x}, Y{y} {};

    ~Point() = default;

    auto operator+(Point point) const -> Point
    {
        return {X.value + point.X.value, Y.value + point.Y.value};
    }

    auto operator+(double value) const -> Point
    {
        return {X.value + value, Y.value + value};
    }

    void operator+=(Point point)
    {
        sum(point.X.value, point.Y.value);
    }

    void operator+=(double value)
    {
        sum(value, value);
    }

    auto operator-(const Point point) const -> Point
    {
        return {X.value - point.X.value, Y.value - point.Y.value};
    }

    auto operator-(double value) const -> Point
    {
        return {X.value - value, Y.value - value};
    }

    void operator-=(const Point point)
    {
        sum(-point.X.value, -point.Y.value);
    }

    void operator-=(double value)
    {
        sum(-value, -value);
    }

    auto operator*(const Point point) const -> Point
    {
        return {X.value * point.X.value, Y.value * point.Y.value};
    }

    auto operator*(double value) const -> Point
    {
        return {X.value * value, Y.value * value};
    }

    void operator*=(const Point point)
    {
        multiply(point.X.value, point.Y.value);
    }

    void operator*=(double value)
    {
        multiply(value, value);
    }

    auto operator==(const Point point) const -> bool
    {
        return equal(point);
    }

    // X += x, Y += y
    void sum(double x, double y)
    {
        X.value += x;
        Y.value += y;
    }

    // Returns each coordinate by adding value.
    template <typename T>
    static auto sum(Points points, T value) -> Points
    {
        Points result{};
        for (const auto &p : points) {
            result.push_back(p + value);
        }
        return result;
    }

    // Sum : Point (Total X axis, Total Y axis).
    static auto total(const Points &points) -> Point
    {
        Point sum;
        for (auto point : points) {
            sum += point;
        }

        return sum;
    }

    // Average : Point (Total X axis / Points, Total Y axis / Points).
    static auto average(const Points &points, Point &point) -> bool
    {
        if (points.empty()) {
            point = Point(0, 0);
            return false;
        }

        point = total(points) * (1.0 / static_cast<double>(points.size())) ;

        return true;
    }

    // X *= x; Y *= y
    void multiply(double x, double y)
    {
        X.value *= x;
        Y.value *= y;
    }

    // Checks if coordinates are equal.
    auto equal(Point point) const -> bool
    {
        return X.value == point.X.value && Y.value == point.Y.value;
    }

    // Angle of the imaginary line between the current point and the other.
    auto angle(Point point) const -> double
    {
        return Math::angle(X.value, Y.value, point.X.value, point.Y.value);
    }

    // Sort the points clockwise using center point.
    static auto organize(Point center, Points points) -> Points
    {
        if (points.size() < 2) {
            return points;
        }

        // Map : Angle x Point.
        std::map<double, Points> mapPoint;

        for (auto value : points) {
            auto key = center.angle(value);
            if (mapPoint.find(key) == mapPoint.end()) {
                mapPoint.insert(make_pair(key, Points{value}));
            }
            else {
                mapPoint[key].push_back(value);
            }
        }

        Points result;
        for (const auto &item : mapPoint) {
            for (auto point : item.second) {
                result.push_back(point);
            }
        }

        return result;
    }

    auto toStr(bool trimZeros = false) const -> std::string
    {
        return X.toStr(trimZeros) + "," + Y.toStr(trimZeros);
    }

};

// Special point.
static const Point Origin = Point(0, 0);
static const Point Zero   = Point(0, 0);

class Base {

    // Store the last configuration.
    Points vertices;

    Point last_first, last_second, last_third, last_fourth;

    void update(Point first, Point second, Point third, Point fourth)
    {
        last_first = first;
        last_second = second;
        last_third = third;
        last_fourth = fourth;

        if (!vertices.empty()) {
            vertices[0] = first;
        }
        if (vertices.size() > 1) {
            vertices[1] = second;
        }
        if (vertices.size() > 2) {
            vertices[2] = third;
        }
        if (vertices.size() > 3) {
            vertices[3] = fourth;
        }
    }

    auto state() -> bool
    {
        return (!first.equal(last_first) || !second.equal(last_second) ||
                !third.equal(last_third) || !fourth.equal(last_fourth));
    }

public:

    Point first, second, third, fourth;

    std::string label{"Base"};

    Base() = default;
    ~Base() = default;

    auto setup(const Points &points) -> Points
    {
        if (points.size() < 2) {
            vertices.clear();
            return vertices;
        }

        vertices = points;
        first  = vertices[0];
        second = vertices[1];
        third  = points.size() > 2 ? vertices[2] : Point();
        fourth = points.size() > 3 ? vertices[3] : Point();

        last_first  = first;
        last_second = second;
        last_third  = third;
        last_fourth = fourth;

        return vertices;
    }

    auto operator==(const Base &polygon) -> bool
    {
        return equal(polygon);
    }

    auto equal(Base polygon, bool compareOrder = false) -> bool
    {
        return Check::equal(points(), polygon.points());
    }

    // Returns the current vertices.
    auto points() -> Points
    {
        if (state()) {
            update(first, second, third, fourth);
        }

        return vertices;
    }

};

// Rectangle (x1,y1)(x2,y2)(x3,y3)(x4,y4)
class Rectangle : public Base {

public:

    Rectangle() = default;

    // Rectangle: Points (x1,y1),(x2,y2),(x3,y3),(x4,y4)
    Rectangle(Point first, Point second, Point third, Point fourth)
    {
        setup(first, second, third, fourth);
    };

    // Rectangle : Point (x,y), width and heigth.
    Rectangle(Point origin, double width, double heigth)
    {
        setup(origin, width, heigth);
    }

    ~Rectangle() = default;

    // Rectangle: Points (x1,y1),(x2,y2),(x3,y3),(x4,y4)
    // Returns vertices.
    auto setup(Point first, Point second, Point third, Point fourth) -> Points
    {
        Base::setup({first, second, third, fourth});
        label = "Rectangle";

        return points();
    }

    // Rectangle : Point (x,y), width and heigth.
    // Returns vertices.
    auto setup(Point origin, double width, double heigth) -> Points
    {
        return setup(origin,
                     origin + Point(width, 0),
                     origin + Point(width, heigth),
                     origin + Point(0, heigth));
    }

};

// SVG
class SVG {

public:

    static constexpr const char *WHITE = "#FFFFFF";
    static constexpr const char *BLACK = "#000000";
    static constexpr const char *RED   = "#FF0000";
    static constexpr const char *GREEN = "#00FF00";
    static constexpr const char *BLUE  = "#0000FF";

    // Metadata setup.
    // creator             : String with the name of the creator or developer,
    // title               : String with title,
    // publisherAgentTitle : String with the name of the publisher,
    // date                : String with creation date, if empty use current date.
    struct Metadata {
        std::string creator = "SVG created automatically by algorithm in C++.";
        std::string title = "SVG";
        std::string publisherAgentTitle;
        std::string date;

        Metadata() = default;
        Metadata(std::string creator, std::string title, std::string publisher)
            : creator{std::move(creator)}, title{std::move(title)},
              publisherAgentTitle{std::move(publisher)} {}
    };

    // Drawing setup.
    // name          : ID used in SVG element,
    // fill          : Fill color in hexadecimal string format (#FFFFFF),
    // stroke        : Stroke color in hexadecimal string format (#FFFFFF),
    // strokeWidth   : Line width,
    // fillOpacity   : Fill opacity or alpha value from 0 to 255.
    // strokeOpacity : Stroke opacity or alpha value from 0 to 255.
    struct Style {
        std::string name{"Shape"}, fill{WHITE}, stroke{BLACK};
        double strokeWidth{1.0};
        double fillOpacity{255.0}, strokeOpacity{255.0}; // 0.0 = 0%; 255 = 1.0 = 100%

        Style() = default;

        Style(std::string name,  std::string fill, std::string stroke, double strokeWidth, double fillOpacity, double strokeOpacity)
            : name{std::move(name)}, fill{std::move(fill)}, stroke{std::move(stroke)},
              strokeWidth{strokeWidth}, fillOpacity{fillOpacity}, strokeOpacity{strokeOpacity} {}
    };

    // Polygon and Polyline.
    // points : Points vector (x,y).
    struct NormalShape : Style {
        Points points{};

        NormalShape() = default;

        NormalShape(std::string name,  std::string fill, std::string stroke, double strokeWidth, double fillOpacity, double strokeOpacity, Points points)
            :  Style(std::move(name),  std::move(fill), std::move(stroke), strokeWidth, fillOpacity, strokeOpacity),
               points{std::move(points)} {}
    };

    // Converts decimal value to hexadecimal.
    static auto INT2HEX(unsigned value) -> std::string
    {
        std::string digits = "0123456789ABCDEF";
        std::string result;
        if (value < 16) {
            result.push_back('0');
            result.push_back(digits[value % 16]);
        }
        else {
            while (value != 0) {
                result = digits[value % 16] + result;
                value /= 16;
            }
        }
        return result;
    }

    // Formats values (Red, Green, Blue) to "#RRGGBB" hexadecimal.
    static auto RGB2HEX(int R, int G, int B) -> std::string
    {
        return "#" + INT2HEX(R) + INT2HEX(G) + INT2HEX(B);
    }

    // Returns SVG: <g> Elements </g>
    static auto group(std::string id, const std::string &elements) -> std::string
    {
        id = id.empty() ? "<g>\n" : "<g id=\"" + id + "\" >\n";
        return elements.empty() ? "" : id + elements + "</g>\n";
    }

private:

    // Validates and formats entries.
    static auto style(Style style, const std::string &name) -> std::string
    {
        style.name = name.empty() ? "Shape" : name;
        style.stroke = style.stroke.empty() ? "#000000" : style.stroke;
        style.fillOpacity = style.fillOpacity < 0 ? 0 : std::min(style.fillOpacity / 255, 1.0);
        style.strokeOpacity = style.strokeOpacity < 0 ? 0 : std::min(style.strokeOpacity / 255, 1.0);

        return {
            "id=\"" + style.name + "\"\nstyle=\"" +
            "opacity:" + Text::trimZeros(style.fillOpacity) + ";fill:" + style.fill +
            ";stroke:" + style.stroke + ";stroke-width:" + Text::trimZeros(style.strokeWidth) +
            ";stroke-opacity:" + Text::trimZeros(style.strokeOpacity) +
            ";stroke-linejoin:round;stroke-linecap:round\"\n" };
    }

public:

    // Return SVG: <polyline ... />
    static auto polyline(const NormalShape &shape) -> std::string
    {

        if (shape.points.empty()) {
            return "<!-- Empty -->\n";
        }

        std::string values;
        for (const auto &point : shape.points) {
            values += point.toStr(true) + " ";
        }

        return {
            "<polyline\n" + style(shape, shape.name) + "points=\"" + values + "\" />\n"
        };
    }

    // Return full SVG.
    static auto svg(int width, int height, const std::string &xml,
                    Metadata metadata) -> std::string
    {
        std::string now;
        try {
            time_t t = time(nullptr);
            tm *const pTm = localtime(&t);
            now = std::to_string(1900 + pTm->tm_year);
        }
        catch (...) {
            // pass
        }

        metadata.date = metadata.date.empty() ? now : metadata.date;

        return {
            "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n"
            "<svg\n"
            "xmlns:dc=\"http://purl.org/dc/elements/1.1/\"\n"
            "xmlns:cc=\"http://creativecommons.org/ns#\"\n"
            "xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\"\n"
            "xmlns:svg=\"http://www.w3.org/2000/svg\"\n"
            "xmlns=\"http://www.w3.org/2000/svg\"\n"
            "xmlns:xlink=\"http://www.w3.org/1999/xlink\"\n"
            "width=\"" + std::to_string(width) + "\"\n" +
            "height=\"" + std::to_string(height) + "\"\n" +
            "viewBox= \"0 0 " + std::to_string(width) + " " + std::to_string(height) + "\"\n" +
            "version=\"1.1\"\n" +
            "id=\"svg8\">\n" +
            "<title\n" +
            "id=\"title1\">" + metadata.title + "</title>\n" +
            "<defs\n" +
            "id=\"defs1\" />\n" +
            "<metadata\n" +
            "id=\"metadata1\">\n" +
            "<rdf:RDF>\n" +
            "<cc:Work\n" +
            "rdf:about=\"\">\n" +
            "<dc:format>image/svg+xml</dc:format>\n" +
            "<dc:type\n" +
            "rdf:resource=\"http://purl.org/dc/dcmitype/StillImage\" />\n" +
            "<dc:title>" + metadata.title + "</dc:title>\n" +
            "<dc:date>" + metadata.date + "</dc:date>\n" +
            "<dc:publisher>\n" +
            "<cc:Agent>\n" +
            "<dc:title>" + metadata.publisherAgentTitle + "</dc:title>\n" +
            "</cc:Agent>\n" +
            "</dc:publisher>\n" +
            "<dc:subject>\n" +
            "<rdf:Bag>\n" +
            "<rdf:li></rdf:li>\n" +
            "<rdf:li></rdf:li>\n" +
            "<rdf:li></rdf:li>\n" +
            "<rdf:li></rdf:li>\n" +
            "</rdf:Bag>\n" +
            "</dc:subject>\n" +
            "<dc:creator>\n" +
            "<cc:Agent>\n" +
            "<dc:title>" + metadata.creator + "</dc:title>\n" +
            "</cc:Agent>\n" +
            "</dc:creator>\n" +
            "<cc:license\n" +
            "rdf:resource=\"http://creativecommons.org/publicdomain/zero/1.0/\" />\n" +
            "<dc:description>SVG created automatically by algorithm in C++.</dc:description>\n" +
            "</cc:Work>\n" +
            "<cc:License\n" +
            "rdf:about=\"http://creativecommons.org/publicdomain/zero/1.0/\">\n" +
            "<cc:permits\n" +
            "rdf:resource=\"http://creativecommons.org/ns#Reproduction\" />\n" +
            "<cc:permits\n" +
            "rdf:resource=\"http://creativecommons.org/ns#Distribution\" />\n" +
            "<cc:permits\n" +
            "rdf:resource=\"http://creativecommons.org/ns#DerivativeWorks\" />\n" +
            "</cc:License>\n" +
            "</rdf:RDF>\n" +
            "</metadata>\n" +
            "<!--      Created in C++ algorithm       -->\n" +
            "<!-- Attention: do not modify this code. -->\n" +
            "\n" + xml + "\n" +
            "<!-- Attention: do not modify this code. -->\n" +
            "</svg>"
        };
    }
};

// Color
class Color {

public:

    struct RGBA {
        int R{0}, G{0}, B{0}, A{0};

        RGBA() = default;

        RGBA(int r, int g, int b, int a)
            : RGBA(std::vector<int>
        {
            r, g, b, a
        }) {}

        explicit RGBA(std::vector<int> rgba)
            : RGBA()
        {
            switch (rgba.size()) {
            case 4:
                A = rgba[3];
            case 3:
                B = rgba[2];
            case 2:
                G = rgba[1];
            case 1:
                R = rgba[0];
                break;
            default:
                break;
            }

            R = R < 0 ? 0 : R % 256;
            G = G < 0 ? 0 : G % 256;
            B = B < 0 ? 0 : B % 256;
            A = A < 0 ? 0 : A % 256;
        }

        auto operator==(RGBA rgba) const -> bool
        {
            return equal(rgba);
        }

        auto empty() const -> bool
        {
            return R == 0 && G == 0 && B == 0 && A == 0;
        }

        auto equal(RGBA rgba) const  ->bool
        {
            return R == rgba.R && G == rgba.G && B == rgba.B && A == rgba.A;
        }

        auto toStr(bool alpha = true) const  ->std::string
        {
            return {
                std::to_string(R) + "," + std::to_string(G) + "," +
                std::to_string(B) + (alpha ? "," + std::to_string(A) : "")
            };
        }
    };

};

} // namespace smalltoolbox

#endif // SMALLTOOLBOX_H_
