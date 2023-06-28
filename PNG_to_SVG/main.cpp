#include "png2svg.h"

#include <iostream>

int main()
{
    // TEST
    std::string input   = "Resources/test.png";
    std::string output1 = "Resources/output1.svg";
    std::string output2 = "Resources/output2.svg";

    ImgTool img;
    if (img.load(input)) {
        img.summary();
        img.exportSVG(output1, img.PIXEL);
        img.exportSVG(output2, img.GROUP);
    } else {
        std::cerr << "Error reading file!\n";
    }

    return 0;
}
