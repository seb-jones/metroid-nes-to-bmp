#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "met.h"

unsigned char *file_contents = NULL;
unsigned char *tile_pat_pointer = NULL;
unsigned char *pixels = NULL;
MetroidArea areas[NUMBER_OF_AREAS];
unsigned char name_table[32 * 30];
unsigned char attrib_table[8 * 8];
unsigned int current_area = 0;
unsigned char hardtype;
unsigned char defpalnum;

// Rotate bits to the left
// Adaptation of the MSVC intrinsic _rotl
unsigned int rotl(unsigned int value, int shift)
{
    return (value << shift) | (value >> (32 - shift));
}

void DrawSprite(unsigned int tilenum, unsigned int tileflip,
                unsigned int palnum, unsigned int areanum, unsigned int xpos,
                unsigned int ypos)
{
    unsigned int BitMapPos;
    unsigned char *TilePtr;
    unsigned int TileOfs;
    char TileData;
    char TileAdd;
    unsigned int BitIndex;

    TileOfs = areas[areanum].gfxptr[tilenum + 256];
    TilePtr = &file_contents[TileOfs];
    palnum += 4;
    TileAdd = (char)(palnum << 2);
    BitMapPos = (ypos << 8) + xpos; // Calculate position on bitmap

    switch (tileflip) {
    case 0:                         // Normal
        for (int a = 0; a < 8; a++) // Do eight lines
        {
            if (BitMapPos >= (256 * 240)) break;
            BitIndex = TilePtr[a] + (TilePtr[a + 8] << 8);
            for (int b = 0; b < 8; b++) {
                TileData =
                    ((char *)tile_pat_pointer)[(BitIndex << 3) + b] | TileAdd;
                if (TileData & 0x03) ((char *)pixels)[BitMapPos + b] = TileData;
            }
            BitMapPos += 256;
        }
        break;

    case 1:                         // Horizontal flip
        for (int a = 0; a < 8; a++) // Do eight lines
        {
            if (BitMapPos >= (256 * 240)) break;
            BitIndex = TilePtr[a] + (TilePtr[a + 8] << 8);
            for (int b = 0; b < 8; b++) {
                TileData =
                    ((char *)tile_pat_pointer)[(BitIndex << 3) + (7 - b)] |
                    TileAdd;
                if (TileData & 0x03) ((char *)pixels)[BitMapPos + b] = TileData;
            }
            BitMapPos += 256;
        }
        break;

    case 2:                          // Vertical flip
        for (int a = 7; a >= 0; a--) // Do eight lines
        {
            if (BitMapPos >= (256 * 240)) break;
            BitIndex = TilePtr[a] + (TilePtr[a + 8] << 8);
            for (int b = 0; b < 8; b++) {
                TileData =
                    ((char *)tile_pat_pointer)[(BitIndex << 3) + b] | TileAdd;
                if (TileData & 0x03) ((char *)pixels)[BitMapPos + b] = TileData;
            }
            BitMapPos += 256;
        }
        break;

    case 3:                          // Both
        for (int a = 7; a >= 0; a--) // Do eight lines
        {
            if (BitMapPos >= (256 * 240)) break;
            BitIndex = TilePtr[a] + (TilePtr[a + 8] << 8);
            for (int b = 0; b < 8; b++) {
                TileData =
                    ((char *)tile_pat_pointer)[(BitIndex << 3) + (7 - b)] |
                    TileAdd;
                if (TileData & 0x03) ((char *)pixels)[BitMapPos + b] = TileData;
            }
            BitMapPos += 256;
        }
        break;
    }
};

unsigned char *GetFramePointer(unsigned int framenum, unsigned int areanum)
{
    unsigned int frame2ptrofs;
    void *p;
    unsigned char *frame2data;

    frame2ptrofs =
        areas[areanum].frame2ptrofs - 0x08000 + areas[areanum].ROMofs;

    p = &file_contents[frame2ptrofs + (framenum << 1)];

    frame2data =
        *(unsigned short *)p - 0x08000 + areas[areanum].ROMofs + file_contents;

    return frame2data;
};

unsigned char *GetSpritePosPointer(unsigned int posnum, unsigned int areanum)
{
    unsigned int sprpos2ptrofs;
    void *p;
    unsigned char *sprpos2data;

    sprpos2ptrofs =
        areas[areanum].sprpos2ptrofs - 0x08000 + areas[areanum].ROMofs;

    p = &file_contents[sprpos2ptrofs + (posnum << 1)];

    sprpos2data =
        *(unsigned short *)p - 0x08000 + areas[areanum].ROMofs + file_contents;

    return sprpos2data;
};

void DrawFrame(unsigned int framenum, unsigned int areanum, unsigned int xpos,
               unsigned int ypos)
{
    unsigned char *frame2data;
    unsigned char *sprpos2data;
    unsigned int framepos = 0;
    unsigned int sprpos = 0;
    unsigned int xpostemp;
    unsigned int ypostemp;
    unsigned int palnum;
    unsigned int tileflip;
    bool done = false;

    frame2data = GetFramePointer(framenum, areanum);
    sprpos2data = GetSpritePosPointer(frame2data[0] & 0x0F, areanum);

    palnum = ((frame2data[0] >> 4) | hardtype) & 0x03;
    tileflip = frame2data[0] >> 6;
    ypos += frame2data[1];
    xpos += frame2data[2];
    framepos = 3;

    while (done == false) {
        switch (frame2data[framepos]) {
        case 0xFC:
            ypos = (ypos + frame2data[framepos + 1]) & 0xFF;
            xpos = (xpos + frame2data[framepos + 2]) & 0xFF;
            framepos += 3;
            break;

        case 0xFD:
            palnum = (frame2data[framepos + 1] | hardtype) & 0x03;
            tileflip = frame2data[framepos + 1] >> 6;
            framepos += 2;
            break;

        case 0xFE:
            sprpos += 2;
            framepos++;
            break;

        case 0xFF: done = true; break;

        default:
            ypostemp = (ypos + sprpos2data[sprpos]) & 0xFF;
            xpostemp = (xpos + sprpos2data[sprpos + 1]) & 0xFF;
            sprpos += 2;
            DrawSprite(frame2data[framepos++], tileflip, palnum, current_area,
                       xpostemp, ypostemp);
            break;
        }
    }
};

unsigned char *GetStructPointer(unsigned int structnum, unsigned int areanum)
{
    unsigned int structptrofs;
    void *p;
    unsigned char *structdata;

    structptrofs =
        areas[areanum].structptrofs - 0x08000 + areas[areanum].ROMofs;

    p = &file_contents[structptrofs + (structnum << 1)];

    structdata =
        *(unsigned short *)p - 0x08000 + areas[areanum].ROMofs + file_contents;

    return structdata;
};

void DrawStruct(unsigned char *objptr, unsigned int areanum)
{
    unsigned int posx;
    unsigned int posy;
    unsigned char *structdata;
    unsigned int structpos = 0;
    unsigned int xlength;
    unsigned char *macroptr;
    unsigned int macroofs;
    unsigned int macronum;
    unsigned char palnum;
    unsigned char palselect;
    unsigned char andval;
    unsigned char attribdata;

    posy = (objptr[0] & 0xF0) >> 3;

    structdata =
        GetStructPointer(objptr[1], areanum); // Get pointer to structure data

    palnum = objptr[2];

    while (structdata[structpos] != 0xFF) // Start drawing macros
    {
        posx = (objptr[0] & 0x0F) << 1; // Reset nametable x pos

        posx += (structdata[structpos] & 0xF0) >> 3;

        xlength = structdata[structpos++] &
                  0x0F; // Number of macros to draw horizontally
        for (unsigned int i = 0; i < xlength; i++) // Do'em all
        {
            // Draw one macro

            macronum = structdata[structpos++];
            if ((posy < 30) && (posx < 32)) { // Draw only if inside nametable
                macroofs =
                    areas[areanum].macroofs - 0x08000 + areas[areanum].ROMofs;
                macroptr =
                    &file_contents[macroofs +
                                   (macronum
                                    << 2)]; // Set up pointer to macro data

                // Update nametable

                name_table[(posy << 5) + posx] = macroptr[0];
                name_table[(posy << 5) + posx + 1] = macroptr[1];
                name_table[((posy + 1) << 5) + posx] = macroptr[2];
                name_table[((posy + 1) << 5) + posx + 1] = macroptr[3];

                // Update attribute table (if necessary)

                if (palnum != defpalnum) {
                    attribdata =
                        attrib_table[((posy & 0xFC) << 1) + (posx >> 2)];

                    palselect = (unsigned char)((posy & 2) + ((posx & 2) >> 1));

                    andval = file_contents[0x1F004 + palselect];

                    attribdata &= andval;

                    attribdata |= (unsigned char)(palnum << (palselect << 1));

                    attrib_table[((posy & 0xFC) << 1) + (posx >> 2)] =
                        attribdata;
                }
            }
            posx += 2;
        }
        posy += 2;
    }
};

void DrawEnemy(unsigned char *objptr)
{
    unsigned char enemynum;

    enemynum = (unsigned char)(objptr[1] & 0x0F);

    hardtype = (unsigned char)(objptr[1] >> 7);

    DrawFrame(areas[current_area].frametable[enemynum], current_area,
              (objptr[2] & 0x0F) << 4, objptr[2] & 0xF0);
};

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
    // Allocate memory for bit patterns
    //

    tile_pat_pointer = (unsigned char *)malloc(65536 * 8);
    if (tile_pat_pointer == NULL) {
        perror("Error allocating memory for tile patterns");
        return 1;
    }

    // Generate all possible bit patterns

    {
        char bp1;
        char bp2;
        int index = 0;

        for (int a = 0; a < 65536; a++) {
            bp1 = (char)(a & 0xFF);
            bp2 = (char)(a >> 8);
            bp2 = rotl(bp2, 1);

            for (int b = 0; b < 8; b++) {
                bp1 = rotl(bp1, 1);
                bp2 = rotl(bp2, 1);
                ((char *)tile_pat_pointer)[index++] =
                    (char)((bp1 & 1) | (bp2 & 2));
            }
        }
    }

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

    file_contents = (unsigned char *)malloc(file_size);

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

    unsigned char map_x = file_contents[0x055E7];
    unsigned char map_y = file_contents[0x055E8];

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

    //
    // Draw Room to Bitmap
    //

    unsigned int areanum; // Area number (0..4)
    unsigned int roomnum; // Room number
    unsigned int palnum;

    roomnum = file_contents[0x254E + (map_y << 5) + map_x];

    areanum = MapIndex[(map_y << 5) + map_x];

    current_area = areanum;

    // Convert Area NES Palette to RGB

    unsigned char rgb_palette[32 * 4];
    {
        unsigned int palofs =
            areas[areanum].palofs - 0x08000 + areas[areanum].ROMofs;

        const unsigned char *p = &file_contents[palofs];

        int color;
        unsigned char data;

        for (int i = 0; i < 32; i++) {
            color = p[i];
            data = NESPalette[(color << 2) + 0];
            rgb_palette[(i << 2) + 2] = data;
            data = NESPalette[(color << 2) + 1];
            rgb_palette[(i << 2) + 1] = data;
            data = NESPalette[(color << 2) + 2];
            rgb_palette[(i << 2) + 0] = data;
            rgb_palette[(i << 2) + 3] = 0xff;
            /* rgb_palette[(i << 2) + 3] = 0x00; */
        }
    }

    // Write Test Palette Image
    {
        int image_write_result =
            stbi_write_png("palette.png", 32, 1, 4, rgb_palette, 32);

        assert(image_write_result != 0);
    }

    // Get Room Data

    unsigned char *room_data;
    {
        unsigned int roomptrofs;
        void *p;

        roomptrofs =
            areas[areanum].roomptrofs - 0x08000 + areas[areanum].ROMofs;

        p = &file_contents[roomptrofs + (roomnum << 1)];

        room_data = *(unsigned short *)p - 0x08000 + areas[areanum].ROMofs +
                    file_contents;
    }

    // Set Nametable Entries to Blank Tiles

    memset(name_table, 0x00, 32 * 30);

    // Set Attribute Table Entries to Default Palette Selector

    defpalnum = room_data[0];

    unsigned char attrib_data = file_contents[0x1F028 + defpalnum];

    memset(attrib_table, attrib_data, 8 * 8);

    // Draw room objects on name table

    unsigned int room_pos = 1; // Start at 2nd byte of room data

    while ((room_data[room_pos] != 0xFD) &&
           (room_data[room_pos] != 0xFF)) // Start object loop
    {
        DrawStruct(&room_data[room_pos], areanum);
        room_pos += 3; // Advance to next object
    }

    //
    // Render Room to Image
    //

    // Allocate Memory for Pixels

    pixels = (unsigned char *)malloc(256 * 240);

    memset(pixels, 0xff, 256 * 240);

    if (pixels == NULL) {
        perror("Error allocating memory for pixels");
        return 1;
    }

    // Render name table

    unsigned int bmpos = 0;
    unsigned int bmpostemp;
    unsigned char *TilePtr;
    unsigned char tilenum;
    unsigned int TileOfs;
    unsigned int TileAdd;
    unsigned int BitIndex;

    for (int y = 0; y < 30; y++) {
        for (int x = 0; x < 32; x++) {
            attrib_data = attrib_table[((y & 0xFC) << 1) + (x >> 2)];
            palnum = (attrib_data >> (((y & 2) << 1) + (x & 2))) & 3;
            tilenum = name_table[(y << 5) + x];
            TileOfs = areas[areanum].gfxptr[tilenum];
            TilePtr = &file_contents[TileOfs];
            TileAdd = (palnum << 2) + ((palnum << 2) << 8) +
                      ((palnum << 2) << 16) + ((palnum << 2) << 24);
            bmpostemp = bmpos;
            for (int a = 0; a < 8; a++) // Do eight lines
            {
                BitIndex = TilePtr[a] + (TilePtr[a + 8] << 8);
                ((int *)pixels)[bmpostemp] =
                    ((int *)tile_pat_pointer)[BitIndex << 1] | TileAdd;
                ((int *)pixels)[bmpostemp + 1] =
                    ((int *)tile_pat_pointer)[(BitIndex << 1) + 1] | TileAdd;
                bmpostemp += 256 / 4;
            }
            bmpos += 2;
        }
        bmpos += 512 - 64;
    } // End Y loop

    if (room_data[room_pos++] != 0xFD) {
        return 0;                     // FF reached, nothing to draw
    }

    // Render room sprites
    while (room_data[room_pos] != 0xFF) {
        switch (room_data[room_pos]) {
        case 0x02: // Door
            palnum = room_data[room_pos + 1] & 3;
            switch (room_data[room_pos + 1] & 0xF0) {
            case 0x0A0: // Right door
                DrawSprite(0x0F, 1, palnum, 0, 232, 80);
                DrawSprite(0x1F, 1, palnum, 0, 232, 88);
                DrawSprite(0x2F, 1, palnum, 0, 232, 96);
                DrawSprite(0x2F, 3, palnum, 0, 232, 104);
                DrawSprite(0x1F, 3, palnum, 0, 232, 112);
                DrawSprite(0x0F, 3, palnum, 0, 232, 120);
                break;

            case 0x0B0: // Left door
                DrawSprite(0x0F, 0, palnum, 0, 16, 80);
                DrawSprite(0x1F, 0, palnum, 0, 16, 88);
                DrawSprite(0x2F, 0, palnum, 0, 16, 96);
                DrawSprite(0x2F, 2, palnum, 0, 16, 104);
                DrawSprite(0x1F, 2, palnum, 0, 16, 112);
                DrawSprite(0x0F, 2, palnum, 0, 16, 120);
                break;
            }
            room_pos += 2;
            break;

        case 0x06: // Ridley & Kraid statues
            room_pos++;
            break;

        default: // Normal enemy
            DrawEnemy(&room_data[room_pos]);
            room_pos += 3;
            break;
        }
    }

    // Write Test Room Image
    {
        int screen_width = 256;
        int screen_height = 240;

        unsigned char *rgb =
            (unsigned char *)malloc(screen_width * screen_height * 3);

        memset(rgb, 0xff, screen_width * screen_height * 3);

        for (int y = 0; y < screen_height; ++y) {
            for (int x = 0; x < screen_width; ++x) {
                unsigned char pixel = pixels[y * screen_width + x];

                int rgb_index = (y * 3 * screen_width) + (x * 3);

                int palette_index = pixel * 4;

                rgb[rgb_index] = rgb_palette[palette_index + 2];     // R
                rgb[rgb_index + 1] = rgb_palette[palette_index + 1]; // G
                rgb[rgb_index + 2] = rgb_palette[palette_index];     // B
            }
        }

        int image_write_result =
            stbi_write_bmp("room.bmp", screen_width, screen_height, 3, rgb);

        assert(image_write_result != 0);
    }

    return 0;
}
