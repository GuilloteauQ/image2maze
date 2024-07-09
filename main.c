#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define AT(data, i, j) data[n * ((i) + (j) * w)]

int16_t* copy_to_16b(uint8_t* data, int w, int h, int n) {
    int16_t* data16 = malloc(sizeof(int16_t) * w * h * n);
    for (int i = 0; i < w * h * n; i++) {
        data16[i] = (int16_t)data[i]; 
    }
    return data16;
}

uint8_t* copy_to_8b(int16_t* data, int w, int h, int n) {
    uint8_t* data8 = malloc(sizeof(uint8_t) * w * h * n);
    for (int i = 0; i < w * h * n; i++) {
        data8[i] = (uint8_t)data[i]; 
    }
    return data8;
}


void to_black_and_white(int16_t* data, int w, int h, int n) {
    for (int i = 0; i < w; i++) {
        for (int j = 0; j < h; j++) {
            int idx = n*(i + j * w);
            int16_t mean = 0;
            for (int k = 0; k < n; k++) {
                mean += data[idx + k];
            }
            mean /= n;
            for (int k = 0; k < n; k++) {
                data[idx + k] = mean;
            }
        }
    }
}


void outline(int16_t* data, int w, int h, int n) {
    int16_t* outdata = malloc(sizeof(int16_t) * w * h * n);
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            int16_t new = data[n * ( (i + 0) + (j + 0) * w )] * 8;

            if (i + 1 < w) {
                if (j - 1 >= 0) {
                    new -= AT(data, i + 1, j - 1);
                }
                new -= AT(data, i + 1, j);
                if (j + 1 < h) {
                    new -= AT(data, i + 1, j + 1);
                }
            }
            if (j - 1 >= 0) {
                new -= AT(data, i , j - 1);
            }
            if (j + 1 < h) {
                new -= AT(data, i, j + 1);
            }
            if (i - 1 >= 0) {
                if (j - 1 >= 0) {
                    new -= AT(data, i - 1, j - 1);
                }
                new -= AT(data, i - 1, j);
                if (j + 1 < h) {
                    new -= AT(data, i - 1, j + 1);
                }
            }

            for (int k = 0; k < n; k++ ) {
                outdata[n * ( (i + 0) + (j + 0) * w )  + k] = new;
            }
        }
    }
    memcpy(data, outdata, sizeof(int16_t) * w * h * n);
    free(outdata);
}

void sharpen(int16_t* data, int w, int h, int n) {
    int16_t* outdata = malloc(sizeof(int16_t) * w * h * n);
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            int16_t new = data[n * ( (i + 0) + (j + 0) * w )] * 5;

            if (i + 1 < w) {
                if (j - 1 >= 0) {
                    new -= data[n * ( (i + 1) + (j - 1) * w )];
                }
                if (j + 1 < h) {
                    new -= data[n * ( (i + 1) + (j + 1) * w )];
                }
            }
            if (i - 1 >= 0) {
                if (j - 1 >= 0) {
                    new -= data[n * ( (i - 1) + (j - 1) * w )];
                }
                if (j + 1 < h) {
                    new -= data[n * ( (i - 1) + (j + 1) * w )];
                }
            }

            for (int k = 0; k < n; k++ ) {
                outdata[n * ( (i + 0) + (j + 0) * w )  + k] = new;
            }
        }
    }
    memcpy(data, outdata, sizeof(int16_t) * w * h * n);
    free(outdata);
}

void floydsteinberg(int16_t* data, int w, int h, int n) {
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            int idx = n * (i + j * h);
            int16_t old = data[idx];
            int16_t new = (old < 128) ? 0 : 255;
            int16_t err = old - new;

            for (int k = 0; k < n; k++ ) {
                data[n * ( (i + 0) + (j + 0) * w )  + k] = new;

                if (i + 1 < w)
                data[n * ( (i + 1) + (j + 0) * w )  + k] += 7 * err / 16;

                if (i - 1 >= 0 && j + 1 < h)
                data[n * ( (i - 1) + (j + 1) * w )  + k] += 3 * err / 16;

                if (j + 1 < h)
                data[n * ( (i + 0) + (j + 1) * w )  + k] += 5 * err / 16;

                if (i + 1 < w && j + 1 < h)
                data[n * ( (i + 1) + (j + 1) * w )  + k] += 1 * err / 16;
            }
        }
    }
}


int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "need 2 args\n");
        return 1;
    }
    const char* filename = argv[1];
    int x,y,n;
    unsigned char *data = stbi_load(filename, &x, &y, &n, 0);

    printf("Image: (%d, %d) n = %d\n", x, y, n);

    int16_t* data16 = copy_to_16b(data, x, y, n);
    stbi_image_free(data);

    to_black_and_white(data16, x, y, n);
    //sharpen(data16, x, y, n);
    outline(data16, x, y, n);
    floydsteinberg(data16, x, y, n);


    uint8_t* data8 = copy_to_8b(data16, x, y, n);
    stbi_write_png("out.png", x, y, n, data8, 0);
    free(data16);
    free(data8);


    return 0;
}

