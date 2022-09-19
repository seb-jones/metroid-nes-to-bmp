#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "khansen.c"

#define MAP_WIDTH 32
#define MAP_HEIGHT 32

int main()
{
    if (load_rom()) return 1;

    unsigned char *map_pixels = malloc(32 * 32 * 256 * 240 * 3);

    if (map_pixels == NULL) {
        perror("Error allocating memory for map pixels");
        return 1;
    }

    for (int y = 0; y < MAP_WIDTH; ++y) {
        for (int x = 0; x < MAP_HEIGHT; ++x) {
            if (draw_room(x, y)) return 1;
        }
    }

    return 0;
}
