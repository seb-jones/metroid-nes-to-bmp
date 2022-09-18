#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
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

    return 0;
}
