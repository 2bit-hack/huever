#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <vector>

typedef unsigned char byte;

// https://github.com/nothings/stb
// a header-only library for image loading
extern "C" {
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
}

struct RGBPixel {
    byte r;
    byte g;
    byte b;

    RGBPixel() : r(0), g(0), b(0) {}
    RGBPixel(byte _r, byte _g, byte _b) : r(_r), g(_g), b(_b) {}
};

struct HSVPixel {
    float h;
    float s;
    float v;

    HSVPixel() : h(0.0f), s(0.0f), v(0.0f) {}
    HSVPixel(float _h, float _s, float _v) : h(_h), s(_s), v(_v) {}
};

float max3(float a, float b, float c) {
    if (a >= b && a >= c)
        return a;
    else if (b >= a && b >= c)
        return b;
    else
        return c;
}

float min3(float a, float b, float c) {
    if (a <= b && a <= c)
        return a;
    else if (b <= a && b <= c)
        return b;
    else
        return c;
}

// https://www.tutorialspoint.com/c-program-to-change-rgb-color-model-to-hsv-color-model
HSVPixel RGBtoHSV(RGBPixel rgbPixel) {
    float h, s, v;

    float r_ = (float)rgbPixel.r / 255.0;
    float g_ = (float)rgbPixel.g / 255.0;
    float b_ = (float)rgbPixel.b / 255.0;

    float cmax = max3(r_, g_, b_);
    float cmin = min3(r_, g_, b_);
    float diff = cmax - cmin;

    if (cmax == cmin) {
        h = 0;
    } else if (cmax == r_) {
        h = fmod((60 * ((g_ - b_) / diff) + 360), 360.0);
    } else if (cmax == g_) {
        h = fmod((60 * ((b_ - r_) / diff) + 120), 360.0);
    } else if (cmax == b_) {
        h = fmod((60 * ((r_ - g_) / diff) + 240), 360.0);
    }

    if (cmax == 0) {
        s = 0;
    } else {
        s = (diff / cmax) * 100;
    }

    v = cmax * 100;

    HSVPixel hsvPixel(round(h), round(s), round(v));
    return hsvPixel;
}

// https://www.codespeedy.com/hsv-to-rgb-in-cpp/
RGBPixel HSVtoRGB(HSVPixel hsvPixel) {
    byte r, g, b;

    float H = hsvPixel.h;
    float s = hsvPixel.s;
    float v = hsvPixel.v;
    float C = s * v;
    float X = C * (1 - abs(fmod(H / 60.0f, 2) - 1));
    float m = v - C;
    float r_, g_, b_;

    if (H >= 0 && H < 60) {
        r_ = C, g_ = X, b_ = 0;
    } else if (H >= 60 && H < 120) {
        r_ = X, g_ = C, b_ = 0;
    } else if (H >= 120 && H < 180) {
        r_ = 0, g_ = C, b_ = X;
    } else if (H >= 180 && H < 240) {
        r_ = 0, g_ = X, b_ = C;
    } else if (H >= 240 && H < 300) {
        r_ = X, g_ = 0, b_ = C;
    } else {
        r_ = C, g_ = 0, b_ = X;
    }

    r = (r_ + m) * 255;
    g = (g_ + m) * 255;
    b = (b_ + m) * 255;

    RGBPixel rgbPixel(r, g, b);
    return rgbPixel;
}

/*
Loads image as a 2D vector of HSV pixels and returns true, if successful
If image does not exist, or is unable to be read, the vector remains empty and
false is returned
*/
bool loadImage(std::vector<std::vector<HSVPixel>>& colorData,
               const std::string& filename) {
    int n;
    bool loaded = false;
    int width, height;
    byte* data = stbi_load(filename.c_str(), &width, &height, &n, 4);
    if (data != nullptr) {
        colorData.resize(height, std::vector<HSVPixel>(width));

        /*
        Transform each RGB pixel to HSV
        */
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                int index = 4 * (i * width + j);
                byte R = data[index + 0];
                byte G = data[index + 1];
                byte B = data[index + 2];
                RGBPixel rgbPixel(R, G, B);
                HSVPixel hsvPixel = RGBtoHSV(rgbPixel);
                colorData[i][j] = hsvPixel;
            }
        }

        loaded = true;
    }
    stbi_image_free(data);
    return loaded;
}

/*
Gets an array of the dominant colors in RGB format
*/
std::vector<RGBPixel>
getDominantColors(const std::vector<std::vector<HSVPixel>>& colorData) {
    const int hues = 360;
    const int scalingFactor = 1;
    std::vector<std::pair<int, int>> hueCount(hues + 1, {0, 0});
    std::vector<long long> saturation(hues + 1, 0);
    std::vector<long long> value(hues + 1, 0);

    for (auto colorRow : colorData) {
        for (auto hsvPixel : colorRow) {
            int hue =
                (int)(((int)round(hsvPixel.h) / scalingFactor) * scalingFactor);
            if (hue == 0)
                hue = 1;
            hueCount[hue].first++;
            hueCount[hue].second = hue;
            saturation[hue] += (int)round(hsvPixel.s);
            value[hue] += (int)round(hsvPixel.v);
        }
    }

    /*
    Count the number of unique hues and average out the saturation and
    brightness for each hue
    */
    int uniqueHues = 0;
    for (int idx = 0; idx <= hues; idx++) {
        if (hueCount[idx].first != 0) {
            uniqueHues++;
            saturation[idx] /= hueCount[idx].first;
            value[idx] /= hueCount[idx].first;
        }
    }

    auto cmp = [](const std::pair<int, int>& a, const std::pair<int, int>& b) {
        return (a.first > b.first);
    };

    std::sort(hueCount.begin(), hueCount.end(), cmp);

    const int colorsToShow = std::min(uniqueHues, 10);
    std::vector<RGBPixel> dominantColors;
    for (int i = 0; i < colorsToShow; i++) {
        float h = (float)hueCount[i].second;
        float s = (float)saturation[hueCount[i].second];
        float v = (float)value[hueCount[i].second];

        HSVPixel hsvPixel(h, s, v);
        RGBPixel rgbPixel = HSVtoRGB(hsvPixel);
        dominantColors.push_back(rgbPixel);
    }

    return dominantColors;
}

/*
Display dominant colors in Truecolor (for supported terminals only)
*/
void displayTruecolor(const std::vector<RGBPixel>& colors) {
    for (auto color : colors) {
        std::string colorString = "\x1b[38;2;" + std::to_string((int)color.r) +
                                  ";" + std::to_string((int)color.g) + ";" +
                                  std::to_string((int)color.b) + "m" +
                                  std::to_string((int)color.r) + "|" +
                                  std::to_string((int)color.g) + "|" +
                                  std::to_string((int)color.b) + "\x1b[0m";
        std::cout << colorString << "\n";
    }
}

// TODO: Add ANSI color support for terminals that don't have Truecolor

int main(int argv, char** argc) {
    if (argv < 2) {
        std::cerr << "INVALID NUMBER OF ARGUMENTS!\n";
        return 1;
    }

    std::string filename(argc[1]);
    std::vector<std::vector<HSVPixel>> colorData;

    if (!loadImage(colorData, filename)) {
        std::cerr << "FAILED TO LOAD IMAGE!\n";
        return 1;
    }

    std::vector<RGBPixel> colors = getDominantColors(colorData);
    displayTruecolor(colors);

    return 0;
}