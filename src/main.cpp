#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <unordered_set>
#include <vector>

// https://github.com/nothings/stb
// a header-only library for image loading
extern "C" {
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
}

/*
Represents a single RGB pixel value
*/
struct RGBPixel {
    std::uint8_t r;
    std::uint8_t g;
    std::uint8_t b;

    RGBPixel() : r(0), g(0), b(0) {}
    RGBPixel(uint8_t _r, uint8_t _g, uint8_t _b) : r(_r), g(_g), b(_b) {}
};

/*
Comparators used for sorting
*/

bool cmpRed(const RGBPixel& x, const RGBPixel& y) { return x.r < y.r; }

bool cmpGreen(const RGBPixel& x, const RGBPixel& y) { return x.g < y.g; }

bool cmpBlue(const RGBPixel& x, const RGBPixel& y) { return x.b < y.b; }

/*
Determines primary color and sorts boxes by dominant color channel
*/
void sortBoxes(std::vector<RGBPixel>& box, std::uint8_t& redRange,
               std::uint8_t& greenRange, std::uint8_t& blueRange) {
    redRange = (*std::max_element(box.begin(), box.end(), cmpRed)).r -
               (*std::min_element(box.begin(), box.end(), cmpRed)).r;
    greenRange = (*std::max_element(box.begin(), box.end(), cmpGreen)).g -
                 (*std::min_element(box.begin(), box.end(), cmpGreen)).g;
    blueRange = (*std::max_element(box.begin(), box.end(), cmpBlue)).b -
                (*std::min_element(box.begin(), box.end(), cmpBlue)).b;

    if (redRange >= greenRange && redRange >= blueRange) {
        std::sort(std::begin(box), std::end(box), cmpRed);
    } else if (greenRange >= redRange && greenRange >= blueRange) {
        std::sort(std::begin(box), std::end(box), cmpGreen);
    } else if (blueRange >= redRange && blueRange >= greenRange) {
        std::sort(std::begin(box), std::end(box), cmpBlue);
    }
}

/*
Adapted from
https://indiegamedev.net/2020/01/17/median-cut-with-floyd-steinberg-dithering-in-c/
*/
std::vector<RGBPixel>
medianCutGeneratePalette(const std::vector<RGBPixel>& source,
                         const std::uint_fast32_t numColors) {
    typedef std::vector<RGBPixel> Box;
    typedef std::pair<std::uint8_t, Box> RangeBox;

    std::vector<RangeBox> boxes;
    Box init = source;
    boxes.push_back(RangeBox(0, init));

    while (boxes.size() < numColors) {
        for (RangeBox& boxData : boxes) {
            std::uint8_t redRange;
            std::uint8_t greenRange;
            std::uint8_t blueRange;
            if (std::get<0>(boxData) == 0) {
                sortBoxes(std::get<1>(boxData), redRange, greenRange,
                          blueRange);

                if (redRange >= greenRange && redRange >= blueRange) {
                    std::get<0>(boxData) = redRange;
                } else if (greenRange >= redRange && greenRange >= blueRange) {
                    std::get<0>(boxData) = greenRange;
                } else {
                    std::get<0>(boxData) = blueRange;
                }
            }
        }

        std::sort(boxes.begin(), boxes.end(),
                  [](const RangeBox& a, const RangeBox& b) {
                      return std::get<0>(a) < std::get<0>(b);
                  });

        std::vector<RangeBox>::iterator itr = std::prev(boxes.end());
        Box biggestBox = std::get<1>(*itr);
        boxes.erase(itr);

        Box splitA(biggestBox.begin(),
                   biggestBox.begin() + biggestBox.size() / 2);
        Box splitB(biggestBox.begin() + biggestBox.size() / 2,
                   biggestBox.end());

        boxes.push_back(RangeBox(0, splitA));
        boxes.push_back(RangeBox(0, splitB));
    }

    // each box in boxes can be averaged to determine the colour
    std::vector<RGBPixel> palette;
    for (const RangeBox& boxData : boxes) {
        Box box = std::get<1>(boxData);
        std::uint_fast32_t redAccum = 0;
        std::uint_fast32_t greenAccum = 0;
        std::uint_fast32_t blueAccum = 0;
        std::for_each(box.begin(), box.end(), [&](const RGBPixel& p) {
            redAccum += p.r;
            greenAccum += p.g;
            blueAccum += p.b;
        });
        redAccum /= static_cast<std::uint_fast32_t>(box.size());
        greenAccum /= static_cast<std::uint_fast32_t>(box.size());
        blueAccum /= static_cast<std::uint_fast32_t>(box.size());

        palette.push_back({static_cast<std::uint8_t>(
                               std::min(static_cast<int>(redAccum), 255)),
                           static_cast<std::uint8_t>(
                               std::min(static_cast<int>(greenAccum), 255)),
                           static_cast<std::uint8_t>(
                               std::min(static_cast<int>(blueAccum), 255))});
    }
    return palette;
}

/*
Loads image as a 2D vector of RGB pixels and returns true, if successful
If image does not exist, or is unable to be read, the vector remains empty
and false is returned
*/
bool loadImage(std::vector<RGBPixel>& colorData, const std::string& filename) {
    int n;
    bool loaded = false;
    int width, height;
    const int channels = 3;

    // load 3 8-bit channels, (RGB)
    uint8_t* data = stbi_load(filename.c_str(), &width, &height, &n, channels);

    unsigned long long size = height * width * channels;
    unsigned long long dataSize = height * width;

    if (data != nullptr && height > 0 && width > 0) {
        colorData.resize(dataSize);

        unsigned long long dataIdx = 0;
        for (unsigned long long i = 0; i < size; i += channels) {
            uint8_t r = data[i + 0];
            uint8_t g = data[i + 1];
            uint8_t b = data[i + 2];
            RGBPixel pixel(r, g, b);
            colorData[dataIdx] = pixel;
            dataIdx++;
        }

        loaded = true;
    }
    stbi_image_free(data);
    return loaded;
}

/*
Uses a simple string hash to make all of the obtained colors unique
*/
std::vector<RGBPixel> makeColorsUnique(const std::vector<RGBPixel>& colors) {
    std::vector<RGBPixel> uniqueColors;

    std::unordered_set<std::string> uniqueColorSet;
    for (const auto& color : colors) {
        std::string hash = std::to_string(color.r) + "|" +
                           std::to_string(color.g) + "|" +
                           std::to_string(color.b);
        if (uniqueColorSet.find(hash) != std::end(uniqueColorSet))
            continue;

        uniqueColorSet.insert(hash);
        uniqueColors.push_back(color);
    }

    return uniqueColors;
}

/*
Display dominant colors in Truecolor (for supported terminals only)
*/
void displayTruecolor(const std::vector<RGBPixel>& colors) {
    std::cout << "\n";
    for (const auto& color : colors) {
        std::string colorString = "\x1b[38;2;" + std::to_string(color.r) + ";" +
                                  std::to_string(color.g) + ";" +
                                  std::to_string(color.b) +
                                  "m██████████\x1b[0m";
        std::cout << colorString << "\t";
        std::cout << static_cast<std::uint_fast32_t>(color.r) << " "
                  << static_cast<std::uint_fast32_t>(color.g) << " "
                  << static_cast<std::uint_fast32_t>(color.b) << "\n";
    }
}

// https://stackoverflow.com/a/26665998
// A rough approximation of RGB colors in ANSI

/*
Returns an ANSI color code that roughly corresponds to the given RGB color
*/
std::uint_fast32_t RGBtoANSI(const RGBPixel& color) {
    if (color.r == color.g && color.r == color.b) {
        if (color.r < 8) {
            return 16;
        }

        if (color.r > 248) {
            return 231;
        }

        return round(((color.r - 8) / 247) * 24) + 232;
    }

    std::uint_fast32_t ansiCode =
        16 + (36 * round(static_cast<float>(color.r) / 255 * 5)) +
        (6 * round(static_cast<float>(color.g) / 255 * 5)) +
        round(static_cast<float>(color.b) / 255 * 5);

    return ansiCode;
}

/*
Display dominant colors in ANSI
*/
void displayANSI(const std::vector<RGBPixel>& colors) {
    std::cout << "\n";
    for (const auto& color : colors) {
        std::uint_fast32_t ansiCode = RGBtoANSI(color);
        std::string colorString =
            "\033[38;5;" + std::to_string(ansiCode) + "m██████████\033[0;00m";
        std::cout << colorString << "\t";
        std::cout << static_cast<std::uint_fast32_t>(color.r) << " "
                  << static_cast<std::uint_fast32_t>(color.g) << " "
                  << static_cast<std::uint_fast32_t>(color.b) << "\n";
    }
    std::cout << "\nANSI\n";
}

int main(int argv, char** argc) {
    if (argv < 2) {
        std::cerr << "INVALID NUMBER OF ARGUMENTS!\n";
        return 1;
    }

    bool isTruecolor = true;
    if (argv == 3 && std::string(argc[2]) == std::string("ANSI")) {
        isTruecolor = false;
    }

    std::string filename(argc[1]);
    std::vector<RGBPixel> colorData;

    if (!loadImage(colorData, filename)) {
        std::cerr << "FAILED TO LOAD IMAGE!\n";
        return 1;
    }

    std::vector<RGBPixel> colors =
        makeColorsUnique(medianCutGeneratePalette(colorData, 8));

    if (isTruecolor)
        displayTruecolor(colors);
    else
        displayANSI(colors);

    return 0;
}