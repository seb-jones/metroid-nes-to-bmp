#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "khansen.c"

#define MAP_WIDTH 32
#define MAP_HEIGHT 32
#define IMAGE_WIDTH (MAP_WIDTH * SCREEN_WIDTH)
#define IMAGE_HEIGHT (MAP_HEIGHT * SCREEN_HEIGHT)

// TODO render RGB pixels on the fly so we don't have to store these palettes
#define PALETTE_BYTES (32 * 4)

int write_room_pixels_to_map(unsigned char *map_pixels, int map_x, int map_y);
int write_map_pixels_to_image(unsigned char *map_pixels,
                              unsigned char *map_palettes);

int main()
{
    if (load_rom()) return 1;

    unsigned char *map_pixels = malloc(IMAGE_WIDTH * IMAGE_HEIGHT);

    if (map_pixels == NULL) {
        perror("Error allocating memory for map pixels");
        return 1;
    }

    unsigned char *map_palettes =
        malloc(MAP_WIDTH * MAP_HEIGHT * PALETTE_BYTES);

    if (map_palettes == NULL) {
        perror("Error allocating memory for map palettes");
        return 1;
    }

    for (int y = 0; y < MAP_WIDTH; ++y) {
        for (int x = 0; x < MAP_HEIGHT; ++x) {
            if (draw_room(x, y)) return 1;

            memcpy(map_palettes + (y * MAP_WIDTH + x) * PALETTE_BYTES,
                   rgb_palette, PALETTE_BYTES);

            if (write_room_pixels_to_map(map_pixels, x, y)) return 1;
        }
    }

    return write_map_pixels_to_image(map_pixels, map_palettes);
}

int write_room_pixels_to_map(unsigned char *map_pixels, int map_x, int map_y)
{
    int stride = MAP_WIDTH * SCREEN_WIDTH;

    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            int map_index = (map_y * SCREEN_HEIGHT + y) * stride +
                            (map_x * SCREEN_WIDTH + x);
            int screen_index = y * SCREEN_WIDTH + x;
            map_pixels[map_index] = pixels[screen_index];
        }
    }

    return 0;
}

int write_map_pixels_to_image(unsigned char *map_pixels,
                              unsigned char *map_palettes)
{
    unsigned char *rgb =
        (unsigned char *)malloc(IMAGE_WIDTH * IMAGE_HEIGHT * 3);

    memset(rgb, 0xff, IMAGE_WIDTH * IMAGE_HEIGHT * 3);

    for (int y = 0; y < IMAGE_HEIGHT; ++y) {
        for (int x = 0; x < IMAGE_WIDTH; ++x) {
            unsigned char pixel = map_pixels[y * IMAGE_WIDTH + x];

            int rgb_index = (y * 3 * IMAGE_WIDTH) + (x * 3);

            unsigned char *palette =
                map_palettes + (y / SCREEN_HEIGHT) * MAP_WIDTH * PALETTE_BYTES +
                (x / SCREEN_WIDTH) * PALETTE_BYTES;

            int palette_index = pixel * 4;

            rgb[rgb_index] = palette[palette_index + 2];     // R
            rgb[rgb_index + 1] = palette[palette_index + 1]; // G
            rgb[rgb_index + 2] = palette[palette_index];     // B
        }
    }

    return stbi_write_bmp("room.bmp", IMAGE_WIDTH, IMAGE_HEIGHT, 3, rgb) == 0;
}
