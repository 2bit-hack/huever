#include <iostream>
#include <vector>

typedef unsigned char byte;

// https://github.com/nothings/stb
// a header-only library for image loading
extern "C" {
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
}

bool loadImage(std::vector<byte>& imageData, const std::string& filename,
               int& x, int& y) {
    int n;
    bool loaded = false;
    byte* data = stbi_load(filename.c_str(), &x, &y, &n, 4);
    if (data != nullptr) {
        imageData = std::vector<byte>(data, data + x * y * 4);
        loaded = true;
    }
    stbi_image_free(data);
    return loaded;
}

int main() {
    // this is an example - no such file exists
    std::string filename = "image.png";
    int width, height;
    std::vector<byte> imageData;

    if (!loadImage(imageData, filename, width, height)) {
        std::cerr << "FAILED TO LOAD IMAGE!\n";
        return 1;
    }

    std::cout << "Width = " << width << "\nHeight = " << height << "\n";

    return 0;
}