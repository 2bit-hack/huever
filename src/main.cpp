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

    HSVPixel hsvPixel(h, s, v);
    return hsvPixel;
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

std::vector<RGBPixel>
getDominantColors(const std::vector<std::vector<HSVPixel>>& colorData,
                  const int k) {
    const int colorsToShow = std::min(k, 3);
    std::vector<RGBPixel> dominantColors(colorsToShow);

    const int hues = 360;
    std::vector<std::pair<int, int>> hueCount(hues + 1, {0, 0});
    std::vector<long long> totalSaturation(hues + 1, 0);
    std::vector<long long> totalValue(hues + 1, 0);

    for (auto colorRow : colorData) {
        for (auto hsvPixel : colorRow) {
            int hue = (int)floor(hsvPixel.h);
            hueCount[hue].first++;
            hueCount[hue].second = hue;
            totalSaturation[hue] += (int)floor(hsvPixel.s);
            totalValue[hue] += (int)floor(hsvPixel.v);
        }
    }

    std::sort(hueCount.begin(), hueCount.end());

    // std::cout << hueCount[358].second << " "
    //           << totalSaturation[hueCount[358].second] / hueCount[358].first
    //           << " " << totalValue[hueCount[358].second] /
    //           hueCount[358].first
    //           << "\n";

    return dominantColors;
}

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

    getDominantColors(colorData, 1);

    return 0;
}