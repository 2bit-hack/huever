# huever

A small CLI application to extract dominant colors from an image

![img](https://github.com/2bit-hack/huever/blob/master/img/huever.png)

## Usage

Run `make` to generate the executable

```
./huever path/to/image
```

This assumes the terminal you are using supports Truecolor

Alternatively, you could run

```
./huever path/to/image ANSI
```
On terminals that do not support Truecolor
(The color results may be inaccurate because of the limited palette)

Run `make clean` to clean up the executable

## Libraries used

[stb_image](https://github.com/nothings/stb) by Sean T Barrett, licensed under MIT

## References

[Median-Cut with Floyd–Steinberg dithering in C++](https://indiegamedev.net/2020/01/17/median-cut-with-floyd-steinberg-dithering-in-c/)
