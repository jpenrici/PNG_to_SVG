#include "png2svg.h"

#include <iostream>

int main()
{
    // TEST
    std::string input   = "Resources/test.png";

    ImgTool img;
    if (img.load(input)) {
        img.summary();
        img.exportSVG("Resources/output1_pixel.svg");// img.PIXEL
        img.exportSVG("Resources/output2_group.svg",    img.GROUP);
        img.exportSVG("Resources/output3_regions1.svg", img.REGIONS1);
        img.exportSVG("Resources/output4_regions2.svg", img.REGIONS2);
    }
    else {
        img.summary();
        std::cerr << "Error reading file!\n";
    }

    return 0;
}
