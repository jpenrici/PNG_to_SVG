#ifndef SMALLTOOLBOX_H_
#define SMALLTOOLBOX_H_

#include <algorithm>
#include <array>
#include <cmath>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <numbers>
#include <random>
#include <string>
#include <string_view>
#include <vector>
#include <stdexcept>
#include <unordered_map>
#include <locale>

namespace smalltoolbox {

#define Numbers     std::vector<double>
#define Point2D     smalltoolbox::Point
#define Points      std::vector<smalltoolbox::Point>
#define Polygon     smalltoolbox::RegularPolygon
#define Strings     std::vector<std::string>
#define PI          std::numbers::pi
#define MAXDOUBLE   std::numeric_limits<double>::max()

class Text {

public:

    // Split string by delimiter.
    static auto split(const std::string &str, char delimiter) -> Strings
    {
        Strings result;
        std::string strTemp{};
        for (char character : str) {
            if (character == delimiter) {
                result.push_back(strTemp);
                strTemp.clear();
            }
            else {
                strTemp += character;
            }
        }
        if (!strTemp.empty()) {
            result.push_back(strTemp);
        }

        return result;
    }

    // Replace all occurrences of the character with the string.
    static auto replace(const std::string &str, char character,
                        const std::string &strReplace) -> std::string
    {
        std::string result{};
        for (char characterTemp : str) {
            result += (characterTemp == character ? strReplace : std::string{characterTemp});
        }

        return result;
    }

    // Replace all occurrences of the character with another character.
    static auto replace(const std::string &str, char character,
                        char charReplace) -> std::string
    {
        return Text::replace(str, character, std::string{charReplace});
    }

    // Trim string.
    static auto trim(std::string str, char trimmer) -> std::string
    {
        return rtrim(ltrim(std::move(str), trimmer), trimmer);
    }

    // Trim strings.
    static auto trim(Strings vStr, char trimmer) -> Strings
    {
        for (auto &item : vStr) {
            item = Text::trim(item, trimmer);
        }

        return vStr;
    }

    // Trim string : Remove characters to the left.
    static auto ltrim(std::string str, char trimmer) -> std::string
    {
        int left = 0;
        auto right = str.size() - 1;
        right = right < 0 ? 0 : right;
        while (left < str.size()) {
            if (str[left] != trimmer) {
                break;
            }
            left++;
        }

        return str.substr(left, 1 + right - left);
    }

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

    // Join strings.
    static auto join(Strings vStr, char delimiter) -> std::string
    {
        std::string result{};
        for (unsigned i = 0; i < vStr.size(); i++) {
            result += vStr[i] + (i < vStr.size() - 1 ? std::string{delimiter} : "");
        }

        return result;
    }

    // String with all characters in uppercase.
    static auto strUpper(const std::string &str) -> std::string
    {
        std::locale loc;
        std::string result{str};
        for (std::string::size_type i = 0; i < result.length(); ++i) {
            result[i] = toupper(result[i], loc);
        }

        return result;
    }

    // String with all characters in lowercase.
    static auto strLower(const std::string &str) -> std::string
    {
        std::locale loc;
        std::string result{str};
        for (std::string::size_type i = 0; i < result.length(); ++i) {
            result[i] = tolower(result[i], loc);
        }

        return result;
    }

};

class Math {

public:

    static auto radians(double angle) -> double
    {
        return angle * PI / 180.0;
    }

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

    // Return: value + radius * cos(angle).
    static auto cos(double value, double radius, double angle) -> double
    {
        return value + radius * std::cos(radians(angle));
    }

    // Return: value + radius * sin(angle).
    static auto sin(double value, double radius, double angle) -> double
    {
        return value + radius * std::sin(radians(angle));
    }

    // Returns the distance between two points.
    static auto distance(double x0, double y0, double x1, double y1) -> double
    {
        return std::sqrt(std::pow(x0 - x1, 2) + std::pow(y0 - y1, 2));
    }

    // Rounds value to N digits after decimal point.
    // Number of Decimal Places < 0, returns the same value.
    static auto round(double value, int decimalPlaces = -1) -> double
    {
        if (decimalPlaces < 0) {
            return value;
        }

        if (decimalPlaces == 0) {
            return static_cast<int>(value);
        }

        const int limit = 10;
        decimalPlaces = decimalPlaces > limit ? limit : decimalPlaces;

        const int base = 10;
        return std::round(value * std::pow(base, decimalPlaces)) / std::pow(base, decimalPlaces);
    }

    // Round values.
    // Number of Decimal Places < 0, returns the same value.
    static auto round(Numbers values, int decimalPlaces) -> Numbers
    {
        for (double &value : values) {
            values = round(values, decimalPlaces);
        }

        return values;
    }

    // Sort numbers.
    static auto sort(Numbers numbers, bool ascendingOrder = true) -> Numbers
    {
        if (ascendingOrder) {
            std::sort(numbers.begin(), numbers.end(), std::less<>());
        }
        else {
            std::sort(numbers.begin(), numbers.end(), std::greater<>());
        }

        return numbers;
    }

    // Calculates the area formed by three coordinates.
    static auto triangleArea(double x0, double y0,
                             double x1, double y1,
                             double x2, double y2) -> double
    {
        // Heron's formula
        auto a = distance(x0, y0, x1, y1);
        auto b = distance(x1, y1, x2, y2);
        auto c = distance(x2, y2, x0, y0);
        auto s = (a + b + c) / 2.0;

        return std::sqrt(s * (s - a) * (s - b) * (s - c));
    }

    // Calculates the greatest height formed by three coordinates.
    static auto triangleHeight(double x0, double y0,
                               double x1, double y1,
                               double x2, double y2) -> double
    {
        auto area = triangleArea(x0, y0, x1, y1, x2, y2);
        auto height0 = 2 * area / distance(x0, y0, x1, y1);
        auto height1 = 2 * area / distance(x1, y1, x2, y2);
        auto height2 = 2 * area / distance(x2, y2, x0, y0);

        return std::max(std::max(height0, height1), height2);
    }

};

class Check {

public:

    // Compare groups (vector).
    template<typename T>
    static auto equal(std::vector<T> group1, std::vector<T> group2,
                      bool compareOrder = false) -> bool
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

    // Error message with line indication.
    static void error(int line, const std::string &message)
    {
        throw std::domain_error("[Line: " + std::to_string(line) + "]: " + message);
    }
};

class IO {

public:

    // Return the extension of a file.
    static auto getExtension(const std::string &filePath) -> std::string
    {
        auto extFile = std::filesystem::path(filePath).extension();

        return std::string{extFile};
    }

    // Return the name of a file.
    static auto getFilename(const std::string &filePath) -> std::string
    {
        auto fileName = std::filesystem::path(filePath).filename();

        return std::string{fileName};
    }

    // Check if file exists.
    static auto exists(const std::string &filePath) -> bool
    {
        if (filePath.empty()) {
            return false;
        }

        const auto fpath = std::filesystem::path(filePath);
        if (!std::filesystem::exists(fpath)) {
            std::cerr << "File not found!\n";
            return false;
        }

        return true;
    }

    // Checks if the file extension is the expected one.
    static auto equal(const std::string &filePath,
                      const std::string &fileExtensionExpected) -> bool
    {
        return Text::strUpper(getExtension(filePath)) ==  Text::strUpper(fileExtensionExpected);
    }

    static auto isBitmap(const std::string &filePath) -> bool
    {
        auto extFile = Text::strUpper(getExtension(filePath));
        return extFile == "PNG" || extFile == "JPEG" || extFile == "JPG" ||
               extFile == "BMP";
    }

    static auto isSVG(const std::string &filePath) -> bool
    {
        return Text::strUpper(getExtension(filePath)) == "SVG";
    }

    // Load text file.
    static auto load(const std::string &filePath, std::string filenameExtension) -> std::string
    {
        if (filePath.empty()) {
            return {};
        }

        // Check extension.
        if (!filenameExtension.empty()) {
            if (!equal(filePath, filenameExtension)) {
                std::cerr << "Invalid file!\n";
                return {};
            }

            if (!IO::exists(filePath)) {
                return {};
            }
        }

        std::string str{};
        try {
            std::ifstream fileIn(filePath, std::ios::in);
            if (fileIn.is_open()) {
                std::string line{};
                while (getline(fileIn, line)) {
                    str += line + "\n";
                }
                fileIn.close();
            }
        }
        catch (...) {
            // pass
        }

        return str;
    }

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

    // Add suffix to file name.
    static auto addSuffix(const std::string &filePath, const std::string suffix)
    {
        return getFilename(filePath) + suffix + getExtension(filePath);
    }
};

// Point 2D (x,y)
class Point {

    struct Coordinate {
        double value{0};

        auto toStr() const -> std::string
        {
            return std::to_string(value);
        }
    };

public:

    // Axis
    Coordinate X, Y;

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

    // X = 0; Y = 0
    void reset()
    {
        X.value = 0;
        Y.value = 0;
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

    // Angle between three points.
    // Signal: True, respects the order of the vectors.
    static auto angle(Point origin, Point first, Point second,
                      bool signal = false) -> double
    {
        auto angle1 = origin.angle(first);
        auto angle2 = origin.angle(second);

        if (signal) {
            return angle1 - angle2;
        }

        return std::max(angle1, angle2) - std::min(angle1, angle2);
    }

    // Distance between the current point and another.
    auto distance(Point point) const -> double
    {
        return Math::distance(X.value, Y.value, point.X.value, point.Y.value);
    }

    // Sum all the distances between the vertices.
    static auto sumDistances(Points points) -> double
    {
        double sum{0};
        for (int i = 1; i < points.size(); ++i) {
            sum += points[i].distance(points[i - 1]);
        }
        return sum;
    }

    // Position from angle and radius.
    // Current point as origin.
    auto position(double angle, double horizontalRadius,
                  double verticalRadius) const -> Point
    {
        return {Math::cos(X.value, horizontalRadius, angle),
                Math::sin(Y.value, verticalRadius, angle)};
    }

    // Position from angle and radius.
    // Current point as origin.
    auto position(double angle, double radius) const -> Point
    {
        return position(angle, radius, radius);
    }

    // Area formed by the current point and two others.
    auto triangleArea(Point point1, Point point2)  -> double
    {
        return Math::triangleArea(X.value, Y.value,
                                  point1.X.value, point1.Y.value,
                                  point2.X.value, point2.Y.value);
    }

    // Area formed by three points.
    static auto triangleArea(Point point1, Point point2, Point point3)  -> double
    {
        return Math::triangleArea(point1.X.value, point1.Y.value,
                                  point2.X.value, point2.Y.value,
                                  point3.X.value, point3.Y.value);
    }

    // Greatest height formed between current point and two others.
    auto triangleHeight(Point point1, Point point2) -> double
    {
        return Math::triangleHeight(X.value, Y.value,
                                    point1.X.value, point1.Y.value,
                                    point2.X.value, point2.Y.value);
    }

    // Greatest height formed between three points.
    static auto triangleHeight(Point point1, Point point2, Point point3)  -> double
    {
        return Math::triangleHeight(point1.X.value, point1.Y.value,
                                    point2.X.value, point2.Y.value,
                                    point3.X.value, point3.Y.value);
    }

    // Calculates the point of intersection between two lines.
    // Returns false if the lines are parallel or coincident.
    // Line 1 (x0, y0) - (x1, y1),
    // Line 2 (x2, y2) - (x3, y4).
    static auto lineIntersect(double x0, double y0, double x1, double y1,
                              double x2, double y2, double x3, double y3,
                              Point &point) -> bool
    {
        double d = (y3 - y2) * (x1 - x0) - (x3 - x2) * (y1 - y0);
        if (d == 0) {   // Two lines are parallel or coincident ...
            point.X.value = MAXDOUBLE;
            point.Y.value = MAXDOUBLE;
            return false;
        }

        double t = ((x3 - x2) * (y0 - y2) - (y3 - y2) * (x0 - x2)) / d;
        double u = ((x1 - x0) * (y0 - y2) - (y1 - y0) * (x0 - x2)) / d;

        if (t >= 0.0 && t <= 1.0 && u >= 0 && u <= 1.0) {
            point.X.value = x0 + t * (x1 - x0);
            point.Y.value = y0 + t * (y1 - y0);
            return true;
        }

        // Lines do not intersect.
        return false;
    }

    // Calculates the point of intersection between two lines.
    // Returns false if the lines are parallel or coincident.
    // Line 1 (Point 1) - (Point 2),
    // Line 2 (Point 3) - (Point 3).
    static auto lineIntersect(Point point1, Point point2,
                              Point point3, Point point4,
                              Point &point) -> bool
    {
        return lineIntersect(point1.X.value, point1.Y.value,
                             point2.X.value, point2.Y.value,
                             point3.X.value, point3.Y.value,
                             point4.X.value, point4.Y.value,
                             point);
    }

    // Returns the rounded current coordinates.
    auto round(int decimalPlaces = 2) const -> Point
    {
        return {Math::round(X.value, decimalPlaces),
                Math::round(Y.value, decimalPlaces)};
    }

    // Round points.
    // Number of Decimal Places < 0, returns the same value.
    static auto round(Points points, int decimalPlaces) -> Points
    {
        for (auto &point : points) {
            point = point.round(decimalPlaces);
        }

        return points;
    }

    // Returns Vector[2] with X and Y values.
    auto XY() -> Numbers
    {
        return {X.value, Y.value};
    }

    // Sort points by X or Y axis.
    static auto sort(Points points, bool X_axis = true) -> Points
    {
        if (points.empty()) {
            return {};
        }

        if (points.size() == 1) {
            return points;
        }

        std::map<double, Numbers > mapPoint;

        for (auto point : points) {
            auto key = X_axis ? point.X.value : point.Y.value;
            auto value = X_axis ? point.Y.value : point.X.value;
            if (mapPoint.find(key) == mapPoint.end()) {
                mapPoint.insert({key, {value}});
            }
            else {
                mapPoint[key].push_back(value);
            }
        }

        Points result;
        for (const auto &item : mapPoint) {
            if (X_axis) {
                for (auto value : item.second) { // Y
                    result.emplace_back(Point(item.first, value));
                }
            }
            else {
                auto values = Math::sort(item.second);
                for (auto value : values) {     // X
                    result.emplace_back(Point(value, item.first));
                }
            }
        }

        return result;
    }

    // Sort the Points clockwise.
    static auto organize(Points points) -> Points
    {
        if (points.size() < 2) {
            return points;
        }

        // Map : Angle x Point.
        std::map<double, Points > mapPoint;

        for (auto value : points) {
            auto key = Point(0, 0).angle(value);
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

    auto toStr(bool trimmed = false) const -> std::string
    {
        if (!trimmed) {
            return X.toStr() + "," + Y.toStr();
        }

        return Text::rtrim(X.toStr(), '0') + "," + Text::rtrim(Y.toStr(), '0');
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

    auto isConvex() -> bool
    {
        auto sides = vertices.size();
        if (sides < 3) {
            return false;
        }

        // Cross product of two vectors.
        auto crossProduct = [](Point origin, Point first, Point second) {
            auto x1 = first.X.value  - origin.X.value;
            auto y1 = first.Y.value  - origin.Y.value;
            auto x2 = second.X.value - origin.X.value;
            auto y2 = second.Y.value - origin.Y.value;
            return x1 * y2 - y1 * x2;
        };

        double previous = 0;
        for (unsigned i = 0; i < sides; i++) {
            auto product = crossProduct(vertices[i],
                                        vertices[(i + 1) % sides],
                                        vertices[(i + 2) % sides]);
            if (product != 0) {
                if (product * previous >= 0) {
                    previous = product;
                }
                else {
                    return false;
                }
            }
        }

        return true;
    }

    // Rearrange the polygon points.
    auto organize() -> Points
    {
        return Point::organize(vertices);
    }

    // Calculates the Area by triangular subdivisions.
    auto area() -> double
    {
        if (vertices.size() < 3) {
            return 0;
        }

        double result = 0;
        if (vertices.size() == 3) {
            result = Point::triangleArea(vertices[0], vertices[1], vertices[2]);
        }
        else if (isConvex()) {
            for (unsigned i = 2; i < vertices.size(); ++i) {
                result += Point::triangleArea(vertices[0], vertices[i - 1], vertices[i]);
            }
        }
        else {
            // TO DO
            // Concave
        }

        return result;
    }

    auto perimeter() -> double
    {
        auto perimeter = Point::sumDistances(vertices);

        if (vertices.size() > 2) {
            perimeter += vertices.back().distance(vertices.front());
        }

        return perimeter;
    }

    auto equal(Base polygon, bool compareOrder = false) -> bool
    {
        return Check::equal(points(), polygon.points());
    }

    auto round(int decimalPlaces = 2) -> Base
    {
        Base polygon;
        polygon.vertices = Point::round(vertices, decimalPlaces);

        return polygon;
    }

    // Return the length of each side.
    auto lengthOfSides() -> Numbers
    {
        if (vertices.size() < 2) {
            return {0};
        }

        if (vertices.size() == 2) {
            return {perimeter()};
        }

        Numbers lengths;
        for (unsigned i = 0; i < vertices.size(); i++) {
            lengths.push_back(vertices[i].distance(vertices[(i + 1) % vertices.size()]));
        }

        return lengths;
    }

    // Average length.
    auto averageLength() -> double
    {
        if (!vertices.empty()) {
            return perimeter() / static_cast<double>(vertices.size());
        }

        return 0;
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

// Line (x1,y1)(x2,y2)
class Line : public Base {

public:

    Line() = default;

    // Line : Point 1 (x1,y1) to Point 2 (x2,y2).
    Line(Point first, Point second)
    {
        setup(first, second);
    };

    // Line : Point (x,y), angle and length.
    Line(Point origin, double angle, double length)
    {
        setup(origin, angle, length);
    }

    // Line : Point 1 (x1,y1) to Point 2 (x2,y2).
    Line(double x1, double y1, double x2, double y2)
    {
        setup(Point(x1, y1), Point(x2, y2));
    };

    ~Line() = default;

    // Line : Point 1 (x1,y1) to Point 2 (x2,y2).
    // Returns vertices.
    auto setup(Point first, Point second = Origin) -> Points
    {
        Base::setup({first, second});

        return points();
    }

    // Line : Point (x,y), angle and length.
    // Returns vertices.
    auto setup(Point origin, double angle, double length) -> Points
    {
        return setup(origin, origin.position(angle, length));
    }

    // Line : Point 1 (x1,y1) to Point 2 (x2,y2).
    // Returns vertices.
    auto setup(double x1, double y1, double x2, double y2) -> Points
    {
        return setup(Point(x1, y1), Point(x2, y2));
    }

    // Distance between the two points.
    auto length() -> double
    {
        return perimeter();
    }

    // Line angle.
    // Direction: first point to second point.
    auto angle() -> double
    {
        // Update
        auto vertices = points();

        if (vertices.size() < 2) {
            return 0;
        }

        return first.angle(second);
    }

    // Midpoint
    auto middle() -> Point
    {
        // Update
        points();

        return first.position(angle(), length() / 2);
    }

    // Returns the intersection point with another line.
    auto intersection(const Line &line, Point &point) -> bool
    {
        return Point::lineIntersect(first.X.value, first.Y.value,
                                    second.X.value, second.Y.value,
                                    line.first.X.value, line.first.Y.value,
                                    line.second.X.value, line.second.Y.value,
                                    point);
    }

    // Calculates the point of intersection between two lines.
    // Returns false if the lines are parallel or coincident.
    static auto lineIntersect(Line line1, const Line &line2, Point &point) -> bool
    {
        return line1.intersection(line2, point);
    }


    // Perpendicular line passing through the point.
    auto perpendicular(Point point) -> Line
    {
        // Dummy triangle area.
        double area = first.triangleArea(second, point);
        // Pythagorean theorem : a^2 + b^2 = c^2
        double c = first.distance(point);               // hypotenuse
        double b = 2 * area / first.distance(second);   // area = base * height / 2
        double a = sqrt(pow(c, 2) - pow(b, 2));

        // Line with base slope.
        return {first.position(angle(), a), point};
    }

    // Perpendicular line passing through the point.
    static auto perpendicular(Line line, Point point) -> Line
    {
        return line.perpendicular(point);
    }

};

// Triangle (x1,y1)(x2,y2)(x3,y3)
class Triangle : public Base {

public:

    Triangle() = default;

    // Triangle: Points (x1,y1),(x2,y2),(x3,y3)
    Triangle(Point first, Point second, Point third)
    {
        setup(first, second, third);
    };

    // Triangle: Points (x1,y1),(x2,y2) and height.
    // Returns vertices.
    Triangle(Point first, Point second, double height)
    {
        setup(Line(first, second), height);
    }

    ~Triangle() = default;

    // Triangle: Points (x1,y1),(x2,y2),(x3,y3)
    // Returns vertices.
    auto setup(Point first, Point second, Point third) -> Points
    {
        Base::setup({first, second, third});

        return points();
    }

    // Triangle: Line and height.
    // Returns vertices.
    auto setup(Line side, double height) -> Points
    {
        return setup(side.first,
                     side.second,
                     Point(side.middle()).position(90 + side.angle(), height));
    }

    // Triangle: Points (x1,y1),(x2,y2) and height.
    // Returns vertices.
    auto setup(Point first, Point second, double height) -> Points
    {
        return setup(Line(first, second), height);
    }

    auto height() -> double
    {
        return first.triangleHeight(second, third);
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

// Regular Polygon (x,y)...(xN,yN)
class RegularPolygon : public Base {

    Point last_center{0, 0};
    double last_angle{0};
    double last_horizontalRadius{0};
    double last_verticalRadius{0};
    unsigned last_sides{0};

    void update()
    {
        // If there has been modification in the parameters.
        if (state()) {
            setup(center, horizontalRadius, verticalRadius, angle, sides);
        }
    }

public:

    Point center{0, 0};
    double angle{0};
    double horizontalRadius{0};
    double verticalRadius{0};
    unsigned sides{0};

    // Returns true if data changes.
    auto state() -> bool
    {
        return !(center == last_center &&
                 angle == last_angle &&
                 horizontalRadius == last_horizontalRadius &&
                 verticalRadius == last_verticalRadius &&
                 sides == last_sides);
    }

    RegularPolygon() = default;

    // Center : polygon center point (x,y),
    // radius : distance from the center (>= 1),
    // angle  : starting point angle,
    // sides  : divisions of a circle (>= 3).
    RegularPolygon(Point center, double radius, double angle, unsigned sides)
    {
        setup(center, radius, radius, angle, sides);
    }

    // Center : polygon center point (x,y),
    // horizontalRadius : distance from the center on X axis (>= 1),
    // verticalRadius   : distance from the center on Y axis (>= 1),
    // angle  : starting point angle,
    // sides  : divisions of a circle (>= 3).
    RegularPolygon(Point center, double horizontalRadius, double verticalRadius,
                   double angle, unsigned sides)
    {
        setup(center, horizontalRadius, verticalRadius, angle, sides);
    }

    ~RegularPolygon() = default;

    // Center : polygon center point (x,y),
    // horizontalRadius : distance from the center on X axis (>= 1),
    // verticalRadius   : distance from the center on Y axis (>= 1),
    // angle  : starting point angle,
    // sides  : divisions of a circle (>= 3).
    // Returns vertices.
    auto setup(Point center, double horizontalRadius, double verticalRadius,
               double angle, unsigned sides) -> Points
    {
        // Update.
        this->center = center;
        this->angle = angle;
        this->horizontalRadius = horizontalRadius;
        this->verticalRadius = verticalRadius;
        this->sides = sides;

        last_center = center;
        last_angle = angle;
        last_horizontalRadius = horizontalRadius;
        last_verticalRadius = verticalRadius;
        last_sides = sides;

        // Check.
        sides = sides > 360 ? 360 : sides;
        if (horizontalRadius < 1 || verticalRadius < 1 || sides < 3) {
            return Base::setup({});
        }

        Points points;
        int begin = static_cast<int>(angle);
        int end = 360 + static_cast<int>(angle);
        int step = 360 / static_cast<int>(sides);
        for (auto a = begin; a < end; a += step) {
            points.emplace_back(center.position(a, horizontalRadius, verticalRadius));
        }

        return Base::setup(points);
    }

    // Center : polygon center point (x,y),
    // radius : distance from the center (>= 1),
    // angle  : starting point angle,
    // sides  : divisions of a circle (>= 3).
    auto setup(Point center, double radius, double angle, unsigned sides) -> Points
    {
        return setup(center, radius, radius, angle, sides);
    }

    auto sideLength(int decimalPlaces = 2) -> double
    {
        if (sides < 2) {
            return 0;
        }

        auto result = first.distance(second);

        if (decimalPlaces > 0) {
            return result;
        }

        const int base = 10;
        return std::round(result * std::pow(base, decimalPlaces)) / std::pow(base, decimalPlaces);
    }

    // Returns the current vertices.
    auto points() -> Points
    {
        update();

        return Base::points();
    }

};

class Ellipse : public RegularPolygon {

public:

    Ellipse() = default;

    Ellipse(Point center, double horizontalRadius, double verticalRadius)
    {
        setup(center, horizontalRadius, verticalRadius);
    }

    ~Ellipse() = default;

    auto setup(Point center, double horizontalRadius, double verticalRadius) -> Points
    {
        RegularPolygon::setup(center, horizontalRadius, verticalRadius, 0, 360);

        return points();
    }

    auto area() -> double
    {
        return horizontalRadius * verticalRadius * PI;
    }

    auto perimeter() -> double
    {
        // TO DO
        return -1;
    }
};

class Circle : public Ellipse {

public:

    Circle() = default;

    Circle(Point center, double radius)
    {
        setup(center, radius);
    }

    ~Circle() = default;

    auto setup(Point center, double radius) -> Points
    {
        Ellipse::setup(center, radius, radius);

        return points();
    }

};

// Irregular Polygon (x,y)...(xN,yN)
class IrregularPolygon : public Base {

public:

    IrregularPolygon() = default;

    explicit IrregularPolygon(const Points &points)
    {
        setup(points);
    }

    explicit IrregularPolygon(Triangle triangle)
    {
        setup(triangle.points());
    }

    explicit IrregularPolygon(Rectangle rectangle)
    {
        setup(rectangle.points());
    }

    explicit IrregularPolygon(RegularPolygon polygon)
    {
        setup(polygon.points());
    }

    ~IrregularPolygon() = default;
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
            : creator{std::move(creator)}, title{std::move(title)}, publisherAgentTitle{std::move(publisher)} {}
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

        Style(std::string name,  std::string fill, std::string stroke, double strokeWidth)
            : name{std::move(name)}, fill{std::move(fill)}, stroke{std::move(stroke)}, strokeWidth{strokeWidth} {}
        Style(std::string name,  std::string fill, std::string stroke, double strokeWidth,
              double fillOpacity, double strokeOpacity)
            : name{std::move(name)}, fill{std::move(fill)}, stroke{std::move(stroke)}, strokeWidth{strokeWidth},
              fillOpacity{fillOpacity}, strokeOpacity{strokeOpacity} {}
    };

    // Polygon and Polyline.
    // points : Points vector (x,y).
    struct Shape : Style {
        Points points{};

        Shape() = default;

        Shape(std::string name,  std::string fill, std::string stroke, double strokeWidth)
            : Style(std::move(name), std::move(fill), std::move(stroke), strokeWidth) {}
        Shape(std::string name,  std::string fill, std::string stroke, double strokeWidth,
              double fillOpacity, double strokeOpacity)
            :  Style(std::move(name),  std::move(fill), std::move(stroke), strokeWidth, fillOpacity, strokeOpacity) {}

        Shape(std::string name,  std::string fill, std::string stroke, double strokeWidth, Points points)
            :  Style(std::move(name),  std::move(fill), std::move(stroke), strokeWidth), points{std::move(points)} {}
        Shape(std::string name,  std::string fill, std::string stroke, double strokeWidth,
              double fillOpacity, double strokeOpacity, Points points)
            :  Style(std::move(name),  std::move(fill), std::move(stroke), strokeWidth, fillOpacity, strokeOpacity),
               points{std::move(points)} {}
    };

    // Circle and Ellipse.
    // center           : Central point, (x,y)
    // horizontalRadius : Horizontal radius of circle, rx
    // verticalRadius   : Vertical radius of circle, ry
    struct CircleShape : Style {
        Point center{Origin};
        double horizontalRadius{1};
        double verticalRadius{1};

        CircleShape() = default;

        CircleShape(std::string name,  std::string fill, std::string stroke, double strokeWidth)
            : Style(std::move(name),  std::move(fill), std::move(stroke), strokeWidth) {}
        CircleShape(std::string name,  std::string fill, std::string stroke, double strokeWidth,
                    double fillOpacity, double strokeOpacity)
            :  Style(std::move(name),  std::move(fill), std::move(stroke), strokeWidth, fillOpacity, strokeOpacity) {}

        CircleShape(std::string name,  std::string fill, std::string stroke, double strokeWidth,
                    Point center, double horizontalRadius, double verticalRadius)
            :  Style(std::move(name),  std::move(fill), std::move(stroke), strokeWidth), center(center),
               horizontalRadius(horizontalRadius), verticalRadius(verticalRadius) {}
        CircleShape(std::string name,  std::string fill, std::string stroke, double strokeWidth,
                    double fillOpacity, double strokeOpacity,
                    Point center, double horizontalRadius, double verticalRadius)
            :  Style(std::move(name),  std::move(fill), std::move(stroke), strokeWidth, fillOpacity, strokeOpacity),
               center(center), horizontalRadius(horizontalRadius), verticalRadius(verticalRadius) {}
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

    // Converts hexadecimal value to decimal
    static auto HEX2INT(std::string value) -> int
    {
        if (value.starts_with('#')) {
            value = value.substr(1);
        }

        if (value.empty()) {
            return 0;
        }

        unsigned result = 0;
        try {
            result = stoul(value, nullptr, 16);
        }
        catch (...) {
            // pass
        }

        return result;
    }

    // Return vector unsigned {Red, Green, Blue, Alpha}
    static auto HEX2RGB(std::string value) -> std::vector<int>
    {
        if (value.starts_with('#')) {
            value = value.substr(1);
        }

        if (value.empty()) {
            return {};
        }

        if (value.size() > 8) {
            value = value.substr(0, 8);
        }

        std::vector<int> result;
        if (value.size() % 2 == 0) {
            while (!value.empty()) {
                std::string num = value.substr(0, 2);
                value = value.substr(2);
                result.push_back(static_cast<unsigned>(HEX2INT(num)));
            }
        }

        return result;
    }

    // Formats values (Red, Green, Blue) to "#RRGGBB" hexadecimal.
    static auto RGB2HEX(int R, int G, int B) -> std::string
    {
        return "#" + INT2HEX(R) + INT2HEX(G) + INT2HEX(B);
    }

    // Formats values (Red, Green, Blue, Alpha) to "#RRGGBBAA" hexadecimal.
    static auto RGBA2HEX(int R, int G, int B, int A) -> std::string
    {
        return RGB2HEX(R, G, B) + INT2HEX(A);
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
            "opacity:" + std::to_string(style.fillOpacity) + ";fill:" + style.fill +
            ";stroke:" + style.stroke + ";stroke-width:" + std::to_string(style.strokeWidth) +
            ";stroke-opacity:" + std::to_string(style.strokeOpacity) +
            ";stroke-linejoin:round;stroke-linecap:round\"\n" };
    }

public:

    // Return SVG: <polyline ... />
    static auto polyline(const Shape &shape) -> std::string
    {

        if (shape.points.empty()) {
            return "<!-- Empty -->\n";
        }

        std::string values;
        for (const auto &point : shape.points) {
            values += point.toStr() + " ";
        }

        return {
            "<polyline\n" + style(shape, shape.name) + "points=\"" + values + "\" />\n"
        };
    }

    // Return SVG: <path ... />
    static auto polygon(Shape shape) -> std::string
    {
        if (shape.points.empty()) {
            return "<!-- Empty -->\n";
        }

        std::string values;
        for (unsigned i = 0; i < shape.points.size() - 1; i++) {
            values += shape.points[i].toStr() + " L ";
        }
        values += shape.points[shape.points.size() - 1].toStr();

        return {
            "<path\n" + style(shape, shape.name) + "d=\"M " + values + " Z\" />\n"
        };
    }

    // Return SVG : <ellipse ... />
    static auto circle(const CircleShape &circle) -> std::string
    {
        if (circle.horizontalRadius < 1 || circle.verticalRadius < 1) {
            return "<!-- Empty -->\n";
        }
        return {
            "<ellipse\n" + style(circle, circle.name) +
            "cx=\"" + circle.center.X.toStr() + "\" " +
            "cy=\"" + circle.center.Y.toStr() + "\" " +
            "rx=\"" + std::to_string(circle.horizontalRadius) + "\" " +
            "ry=\"" + std::to_string(circle.verticalRadius) + "\" />\n"
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

// Sketch
// Organizes SVG development.
class Sketch : public SVG, public Color {

public:

    // Return basic SVG::Shape with Polygon base.
    static auto Shape(Base base, std::string label) -> SVG::Shape
    {
        SVG::Shape shape;
        shape.name = std::move(label);
        shape.points = base.points();

        return shape;
    }

    // Return basic SVG::CircleShape with Ellipse base.
    static auto CircleShape(const Ellipse &ellipse, std::string label) -> SVG::CircleShape
    {
        SVG::CircleShape shape;
        shape.name = std::move(label);
        shape.center = ellipse.center;
        shape.horizontalRadius = ellipse.horizontalRadius;
        shape.verticalRadius = ellipse.verticalRadius;

        return shape;
    }

    // Return SVG::polyline with Polygon base.
    static auto SvgPolyline(const Base &base, std::string label) -> std::string
    {

        return SVG::polygon(Shape(base, std::move(label)));
    }

    // Return SVG::polyline with Polygon base.
    static auto SvgPolyline(Base base, std::string label, RGBA fill, RGBA stroke) -> std::string
    {

        return SVG::polyline(SVG::Shape(std::move(label),
                                        SVG::RGB2HEX(fill.R, fill.G, fill.B),
                                        SVG::RGB2HEX(stroke.R, stroke.G, stroke.B),
                                        1.0,    // strokeWidth
                                        base.points()));
    }

    // Return SVG::polyline with Polygon base.
    static auto SvgPolyline(Base base, std::string label, RGBA fill, RGBA stroke,
                            double fillOpacity, double strokeOpacity) -> std::string
    {

        return SVG::polyline(SVG::Shape(std::move(label),
                                        SVG::RGB2HEX(fill.R, fill.G, fill.B),
                                        SVG::RGB2HEX(stroke.R, stroke.G, stroke.B),
                                        1.0,    // strokeWidth
                                        fillOpacity,
                                        strokeOpacity,
                                        base.points()));
    }

    // Return SVG::polyline with Polygon base.
    static auto SvgPolyline(Base base, std::string label, RGBA fill, RGBA stroke,
                            double fillOpacity, double strokeOpacity,
                            double strokeWidth) -> std::string
    {

        return SVG::polyline(SVG::Shape(std::move(label),
                                        SVG::RGB2HEX(fill.R, fill.G, fill.B),
                                        SVG::RGB2HEX(stroke.R, stroke.G, stroke.B),
                                        strokeWidth,
                                        fillOpacity,
                                        strokeOpacity,
                                        base.points()));
    }

    // Return SVG::polygon with Polygon base.
    static auto SvgPolygon(const Base &base, std::string label) -> std::string
    {

        return SVG::polygon(Shape(base, std::move(label)));
    }

    // Return SVG::polygon with Polygon base.
    static auto SvgPolygon(Base base, std::string label, RGBA fill, RGBA stroke) -> std::string
    {

        return SVG::polygon(SVG::Shape(std::move(label),
                                       SVG::RGB2HEX(fill.R, fill.G, fill.B),
                                       SVG::RGB2HEX(stroke.R, stroke.G, stroke.B),
                                       1.0, // strokeWidth
                                       base.points()));
    }

    // Return SVG::polygon with Polygon base.
    static auto SvgPolygon(Base base, std::string label, RGBA fill, RGBA stroke,
                           double fillOpacity, double strokeOpacity) -> std::string
    {

        return SVG::polygon(SVG::Shape(std::move(label),
                                       SVG::RGB2HEX(fill.R, fill.G, fill.B),
                                       SVG::RGB2HEX(stroke.R, stroke.G, stroke.B),
                                       1.0, // strokeWidth
                                       fillOpacity,
                                       strokeOpacity,
                                       base.points()));
    }

    // Return SVG::polygon with Polygon base.
    static auto SvgPolygon(Base base, std::string label, RGBA fill, RGBA stroke,
                           double fillOpacity, double strokeOpacity,
                           double strokeWidth) -> std::string
    {

        return SVG::polygon(SVG::Shape(std::move(label),
                                       SVG::RGB2HEX(fill.R, fill.G, fill.B),
                                       SVG::RGB2HEX(stroke.R, stroke.G, stroke.B),
                                       strokeWidth,
                                       fillOpacity,
                                       strokeOpacity,
                                       base.points()));
    }

    // Return SVG::circle with Ellipse base.
    static auto SvgCircle(const Ellipse &ellipse, std::string label) -> std::string
    {
        return SVG::circle(CircleShape(ellipse, std::move(label)));
    }

    // Return SVG::circle with Ellipse base.
    static auto SvgCircle(const Ellipse &ellipse, std::string label, RGBA fill, RGBA stroke) -> std::string
    {
        return SVG::circle(SVG::CircleShape(std::move(label),
                                            SVG::RGB2HEX(fill.R, fill.G, fill.B),
                                            SVG::RGB2HEX(stroke.R, stroke.G, stroke.B),
                                            1.0,    // strokeWidth
                                            255,    // fillOpacity
                                            255,    // strokeOpacity
                                            ellipse.center,
                                            ellipse.horizontalRadius,
                                            ellipse.verticalRadius));
    }

    // Return SVG::circle with Ellipse base.
    static auto SvgCircle(const Ellipse &ellipse, std::string label, RGBA fill, RGBA stroke,
                          double fillOpacity, double strokeOpacity) -> std::string
    {
        return SVG::circle(SVG::CircleShape(std::move(label),
                                            SVG::RGB2HEX(fill.R, fill.G, fill.B),
                                            SVG::RGB2HEX(stroke.R, stroke.G, stroke.B),
                                            1.0,    // strokeWidth
                                            fillOpacity,
                                            strokeOpacity,
                                            ellipse.center,
                                            ellipse.horizontalRadius,
                                            ellipse.verticalRadius));
    }

    // Return SVG::circle with Ellipse base.
    static auto SvgCircle(const Ellipse &ellipse, std::string label, RGBA fill, RGBA stroke,
                          double fillOpacity, double strokeOpacity,
                          double strokeWidth) -> std::string
    {
        return SVG::circle(SVG::CircleShape(std::move(label),
                                            SVG::RGB2HEX(fill.R, fill.G, fill.B),
                                            SVG::RGB2HEX(stroke.R, stroke.G, stroke.B),
                                            strokeWidth,
                                            fillOpacity,
                                            strokeOpacity,
                                            ellipse.center,
                                            ellipse.horizontalRadius,
                                            ellipse.verticalRadius));
    }

    // Returns SVG Elements.
    static auto Join(const std::vector<Base> &bases, const std::string &label = "") -> std::string
    {
        std::string strShape{};
        for (const auto &item : bases) {
            strShape += Sketch::SvgPolygon(item, label);
        }

        return strShape;
    }

};

// Interpreter
// Conversion of text into commands for SVG construction.
class Interpreter {

    enum elementType {
        EMPTY, POINTS, LINE, TRIANGLE, RECTANGLE, CIRCLE, ELLIPSE, POLYGON, BASE
    };

    Strings labelType {
        "EMPTY", "POINTS", "LINE", "TRIANGLE", "RECTANGLE",
        "CIRCLE", "ELLIPSE", "POLYGON", "BASE"
    };

    const char SPACE = static_cast<char>(32);
    const std::string ERROR   = "\tERROR";
    const std::string WARNING = "\tWARNING";

public:

    Interpreter() = default;
    ~Interpreter() = default;

    auto svg(std::string line, std::string &error) -> std::string
    {
        if (line.empty()) {
            error = "[LINE EMPTY]\n";
            return {};
        }

        std::string result{};
        std::string bkp = line;

        // Prepare
        line = Text::trim(line, SPACE); // Remove spaces from the ends.
        transform(line.begin(), line.end(), line.begin(), ::toupper); // Format.

        // Check command.
        std::string command{};
        unsigned element = EMPTY;
        for (unsigned i = 1; i < labelType.size(); i++) {
            command = labelType[i] + ":";   // Syntax.
            if (line.starts_with(command)) {
                element = i;
                break;
            }
        }
        if (element < POINTS || element > POLYGON) {
            error = bkp + WARNING + "[Ignore]\n";
            return result;
        }

        // Prepare
        line = line.substr(command.size()); // Remove command word.

        // Check points container: { }
        auto counter = count_if(line.begin(), line.end(),
        [](char c) {
            return c == '{';
        });
        if (counter > 1) {
            error = bkp + ERROR + "[Curly braces]\n";
            return result;
        }
        counter -= count_if(line.begin(), line.end(),
        [](char c) {
            return c == '}';
        });
        if (counter != 0) {
            error = bkp + ERROR + "[Curly braces]\n";
            return result;
        }

        // Check coordinates container. ( )
        counter = count_if(line.begin(), line.end(),
        [](char c) {
            return c == '(';
        }) -
        count_if(line.begin(), line.end(),
        [](char c) {
            return c == ')';
        });
        if (counter != 0) {
            error = bkp + ERROR + "[Parentheses]\n";
            return result;
        }

        // Reset
        Points points{};
        double angle{0};
        double width{0};
        double height{0};
        double length{0};
        double horizontalRadius{0};
        double verticalRadius{0};
        double fillOpacity{255};
        double strokeOpacity{255};
        double strokeWidth{1.0};
        unsigned sides{0};
        std::string label{};
        Color::RGBA fillColor(255, 255, 255, 255);
        Color::RGBA strokeColor(0, 0, 0, 255);

        // Check content between braces.
        // Expected: {(x0,y1)(xN,yN)}
        std::string content{};
        unsigned first = line.find_first_of('{');
        unsigned second = line.find_first_of('}');
        if (second < first) {   // } {
            error = bkp + ERROR + "[Curly braces]\n";
            return result;
        }
        if (first < second) {   // { ... } or {}
            content = line.substr(first + 1, second - first - 1);
            line = line.substr(0, first) + line.substr(second + 1);
        }
        if (!content.empty()) {
            content = Text::replace(content, SPACE, "");  // (x0, y0)( x1,y1) to (x0,y0)(x1,y1)
            content = Text::replace(content, '(', " (");  // (x0,y0)(x1,y1) to (x0,y0) (x1,y1)
            for (auto str : Text::split(content, SPACE)) {
                str = Text::trim(str, SPACE);
                if (str.starts_with('(') && str.ends_with(')')) { // (x,y)
                    str = Text::trim(Text::trim(str, '('), ')');              //  x,y
                    try {
                        auto values = Text::split(str, ',');            //  x y
                        if (values.size() == 2) {
                            // Convert to Point.
                            points.emplace_back(Point(stod(values[0]),
                                                      stod(values[1])));
                        }
                    }
                    catch (...) {
                        error = bkp + ERROR + "[Invalid numeric formatting!]\n";
                        return result;
                    }
                }
            }
        }

        if (points.empty()) {
            error = bkp + WARNING + "[Ignore]\n";
            return result;
        }

        // Check other arguments.
        for (const auto &str : Text::split(line, SPACE)) {
            Strings complements {
                "ANGLE", "SIDES", "WIDTH", "HEIGHT", "LENGTH", "RADIUS", "HRADIUS", "VRADIUS",
                "STROKEW"
            };
            for (unsigned i = 0; i < complements.size(); i++) {
                auto arg = complements[i] + "=";
                if (str.starts_with(arg)) {
                    try {
                        auto value = stod(str.substr(arg.size()));
                        switch (i) {
                        case 0:
                            angle = value;
                            break;
                        case 1:
                            sides = static_cast<unsigned>(value);
                            break;
                        case 2:
                            width = value;
                            break;
                        case 3:
                            height = value;
                            break;
                        case 4:
                            length = value;
                            break;
                        case 5:
                        // radius = horizontalRadius
                        case 6:
                            horizontalRadius = value;
                            break;
                        case 7:
                            verticalRadius = value;
                            break;
                        case 8:
                            strokeWidth = value;
                            break;
                        default:
                            break;
                        }
                    }
                    catch (...) {
                        // pass
                    }
                }
            }
            std::string arg = "LABEL=";
            if (str.starts_with(arg)) {
                try {
                    label = str.substr(arg.size());
                }
                catch (...) {
                    // pass
                }
            }
            arg = "FILL=";
            if (str.starts_with("FILL=")) {
                try {
                    auto numbers = Text::split(str.substr(arg.size()), ',');
                    fillColor = Color::RGBA(stoi(numbers[0]),
                                            stoi(numbers[1]),
                                            stoi(numbers[2]),
                                            255);
                    fillOpacity = (numbers.size() == 4 ? stoi(numbers[3]) : 255);
                }
                catch (...) {
                    // pass
                }
            }
            arg = "STROKE=";
            if (str.starts_with(arg)) {
                try {
                    auto numbers = Text::split(str.substr(arg.size()), ',');
                    strokeColor = Color::RGBA(stoi(numbers[0]),
                                              stoi(numbers[1]),
                                              stoi(numbers[2]),
                                              255);
                    strokeOpacity = (numbers.size() == 4 ? stoi(numbers[3]) : 255);
                }
                catch (...) {
                    // pass
                }
            }
        }

        // Converter
        label = label.empty() ? labelType[element] : label;
        switch (element) {
        case POINTS:
            if (points.size() == 1) {
                // Isolated points in SVG are not recommended.
                error = bkp + WARNING + "[Ignore - A minimum of two points is expected!]\n";
                return {};
            }
            if (points.size() > 1) {
                result = Sketch::SvgPolygon(IrregularPolygon(points),
                                            label, fillColor, strokeColor);
            }
            break;
        case LINE:
            if (points.size() == 1 && angle > 0 && length > 0) {
                result = Sketch::SvgPolygon(Line(points.front(), angle, length),
                                            label, fillColor, strokeColor);
            }
            else if (points.size() == 2) {
                result = Sketch::SvgPolygon(IrregularPolygon(points),
                                            label, fillColor, strokeColor);
            }
            else {
                error = bkp + WARNING + "[Incomplete or wrong syntax!]\n";
                return {};
            }
            break;
        case TRIANGLE:
            if (points.size() == 2) {
                result = Sketch::SvgPolygon(Triangle(points[0], points[1], height),
                                            label, fillColor, strokeColor,
                                            fillOpacity, strokeOpacity, strokeWidth);
            }
            else if (points.size() == 3) {
                result = Sketch::SvgPolygon(IrregularPolygon(points),
                                            label, fillColor, strokeColor,
                                            fillOpacity, strokeOpacity, strokeWidth);
            }
            else {
                error = bkp + WARNING + "[Incomplete or wrong syntax!]\n";
                return {};
            }
            break;
        case RECTANGLE:
            if (points.size() == 1 && width > 0 && height > 0) {
                result = Sketch::SvgPolygon(Rectangle(points.front(), width, height),
                                            label, fillColor, strokeColor,
                                            fillOpacity, strokeOpacity, strokeWidth);
            }
            else if (points.size() == 4) {
                result = Sketch::SvgPolygon(IrregularPolygon(points),
                                            label, fillColor, strokeColor,
                                            fillOpacity, strokeOpacity, strokeWidth);
            }
            else {
                error = bkp + WARNING + "[Incomplete or wrong syntax!]\n";
                return {};
            }
            break;
        case CIRCLE:
            if (points.size() == 1 && horizontalRadius > 0) {
                result = Sketch::SvgCircle(Circle(points.front(), horizontalRadius),
                                           label, fillColor, strokeColor,
                                           fillOpacity, strokeOpacity, strokeWidth);
            }
            else {
                error = bkp + WARNING + "[Incomplete or wrong syntax!]\n";
                return {};
            }
            break;
        case ELLIPSE:
            if (points.size() == 1 && horizontalRadius > 0 && verticalRadius > 0) {
                result = Sketch::SvgCircle(Ellipse(points.front(),
                                                   horizontalRadius,
                                                   verticalRadius),
                                           label, fillColor, strokeColor,
                                           fillOpacity, strokeOpacity, strokeWidth);
            }
            else {
                error = bkp + WARNING + "[Incomplete or wrong syntax!]\n";
                return {};
            }
            break;
        case POLYGON:
            if (points.size() == 1 && horizontalRadius > 0 && angle > 0 && sides > 2) {
                result = Sketch::SvgPolygon(RegularPolygon(points.front(),
                                            horizontalRadius, angle, sides),
                                            label, fillColor, strokeColor,
                                            fillOpacity, strokeOpacity, strokeWidth);
            }
            else if (points.size() > 1) {
                result = Sketch::SvgPolygon(IrregularPolygon(points),
                                            label, fillColor, strokeColor,
                                            fillOpacity, strokeOpacity);
            }
            else {
                error = bkp + WARNING + "[Incomplete or wrong syntax!]\n";
                return {};
            }
            break;
        default:
            error = bkp + WARNING + "[Incomplete or wrong syntax!]\n";
            result = {};
            break;
        }

        return result;
    }

    auto load(const std::string &path, std::string &errors) -> std::string
    {
        auto text = IO::load(path, ".txt");
        if (text.empty()) {
            errors = path + ERROR + "[FILE EMPTY]\n";
            return {};
        }

        std::string result{};
        errors.clear();
        auto lines = Text::split(text, '\n');
        for (unsigned i = 0; i < lines.size(); i++) {
            auto line = lines[i];
            if (line.starts_with('#')) {
                errors += std::to_string(i + 1) + ": [COMMENT LINE]\n";
                continue;
            }
            std::string error{};
            auto res = svg(line, error);
            if (!res.empty()) {
                result += res;
            }
            else {
                errors += std::to_string(i + 1) + ": " + error;
            }
        }

        return result;
    }

};

class Console {

public:

    // Std::cout : double.
    static void view(double value)
    {
        std::cout << std::to_string(value) << '\n';
    }

    // Std::cout : Point(x,y).
    static void view(Point point)
    {
        std::cout << "(" << point.toStr() << ")" << '\n';
    }

    // Std::cout : Vector of Point(x,y).
    static void view(Points points)
    {
        std::string str{};
        for (unsigned i = 0; i < points.size(); i++) {
            str += "(" + points[i].toStr() + ")" + (i < points.size() - 1 ? "," : "");
        }

        if (str.empty()) {
            std::cout << "Empty\n";
        }
        else {
            std::cout << str << '\n';
        }
    }

    // Std::cout : Vector of numbers.
    template<typename T>
    static void view(std::vector<T> values)
    {
        std::string str{};
        for (unsigned i = 0; i < values.size(); i++) {
            str += std::to_string(values[i]) + (i < values.size() - 1 ? "," : "");
        }

        if (str.empty()) {
            std::cout << "Empty\n";
        }
        else {
            std::cout << str << '\n';
        }
    }

    // Std::cout : Vector of strings.
    static void view(Strings values)
    {
        std::string str{};
        for (unsigned i = 0; i < values.size(); i++) {
            str += values[i] + (i < values.size() - 1 ? "," : "");
        }

        if (str.empty()) {
            std::cout << "Empty\n";
        }
        else {
            std::cout << str << '\n';
        }
    }

    // Std::cout : string.
    static void view(const std::string &str)
    {
        std::cout << str << '\n';
    }

};

} // namespace smalltoolbox

#endif // SMALLTOOLBOX_H_
