#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "met.h"

MetroidArea areas[NUMBER_OF_AREAS];

int main()
{
    //
    // Initialize Area data
    //

    areas[BRINSTAR].ROMofs = 0x04010;
    areas[BRINSTAR].mapposx = 0x03;
    areas[BRINSTAR].mapposy = 0x0E;
    areas[BRINSTAR].numstruct = 50;
    areas[BRINSTAR].palofs = 0x0A274;
    areas[BRINSTAR].numrooms = 47;
    areas[BRINSTAR].gfxptr = BrinGFX;
    areas[BRINSTAR].coordptr = BrinCoords;
    areas[BRINSTAR].frametable = frametable0;

    areas[NORFAIR].ROMofs = 0x08010;
    areas[NORFAIR].mapposx = 0x16;
    areas[NORFAIR].mapposy = 0x0D;
    areas[NORFAIR].numstruct = 49;
    areas[NORFAIR].palofs = 0x0A17B;
    areas[NORFAIR].numrooms = 46;
    areas[NORFAIR].gfxptr = NorGFX;
    areas[NORFAIR].coordptr = NorCoords;
    areas[NORFAIR].frametable = frametable1;

    areas[TOURIAN].ROMofs = 0x0C010;
    areas[TOURIAN].mapposx = 0x03;
    areas[TOURIAN].mapposy = 0x04;
    areas[TOURIAN].numstruct = 32;
    areas[TOURIAN].palofs = 0x0A71B;
    areas[TOURIAN].numrooms = 21;
    areas[TOURIAN].gfxptr = TourGFX;
    areas[TOURIAN].coordptr = TourCoords;
    areas[TOURIAN].frametable = frametable2;

    areas[KRAID].ROMofs = 0x10010;
    areas[KRAID].mapposx = 0x07;
    areas[KRAID].mapposy = 0x14;
    areas[KRAID].numstruct = 39;
    areas[KRAID].palofs = 0x0A158;
    areas[KRAID].numrooms = 37;
    areas[KRAID].gfxptr = KraidGFX;
    areas[KRAID].coordptr = KraidCoords;
    areas[KRAID].frametable = frametable3;

    areas[RIDLEY].ROMofs = 0x14010;
    areas[RIDLEY].mapposx = 0x19;
    areas[RIDLEY].mapposy = 0x18;
    areas[RIDLEY].numstruct = 29;
    areas[RIDLEY].palofs = 0x0A0EE;
    areas[RIDLEY].numrooms = 42;
    areas[RIDLEY].gfxptr = RidGFX;
    areas[RIDLEY].coordptr = RidCoords;
    areas[RIDLEY].frametable = frametable4;

    //
    // Load the ROM file
    //

    FILE *file = fopen("./Metroid.nes", "rb");

    if (file == NULL) {
        perror("Error opening ./Metroid.nes");
        return 1;
    }

    fseek(file, 0, SEEK_END);

    long file_size = ftell(file);

    if (file_size != 131088) {
        perror("Error: ./Metroid.nes is not 131,088 bytes");
        fclose(file);
        return 1;
    }

    rewind(file);

    unsigned char *file_contents = (unsigned char *)malloc(file_size);

    if (file_contents == NULL) {
        perror("Error allocating memory for ./Metroid.nes");
        fclose(file);
        return 1;
    }

    if (fread(file_contents, 1, file_size, file) != file_size) {
        perror("Error reading ./Metroid.nes");
        fclose(file);
        free(file_contents);
        return 1;
    }

    fclose(file);

    int rom_id_comparison = memcmp(&file_contents[0x1FFF9], "METROID", 7);
    if (rom_id_comparison != 0) {
        perror("Error: ./Metroid.nes ID string is not correct, this is not a "
               "valid Metroid ROM");
        free(file_contents);
        return 1;
    }

    unsigned char map_pos_x = file_contents[0x055E7];
    unsigned char map_pos_y = file_contents[0x055E8];

    for (int a = 0; a < NUMBER_OF_AREAS; a++) {
        unsigned short *p = (unsigned short *)(areas[a].ROMofs - 0x08000 +
                                               0x09598 + file_contents);
        areas[a].itemofs = p[0];
        areas[a].roomptrofs = p[1];
        areas[a].structptrofs = p[2];
        areas[a].macroofs = p[3];
        areas[a].frame2ptrofs = p[4];
        areas[a].sprpos2ptrofs = p[6];
        areas[a].anim2ofs = p[7];
    }

    return 0;
}
