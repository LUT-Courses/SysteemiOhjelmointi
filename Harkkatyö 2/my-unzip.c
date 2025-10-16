#include <stdio.h>
#include <stdlib.h>

int my_unzip(FILE *file_input, FILE *file_output) {
    if (file_input == NULL || file_output == NULL) { //tarkistetaan että molemmati tiedostot löytyvät
        printf("Cannot open file.\n");
        return 1;
    }
    int count;
    char chr;
    //Luetaan koko tiedosto
    while (fread(&count, sizeof(int), 1, file_input) == 1) {
        if (fread(&chr, sizeof(char), 1, file_input) != 1) {
            printf("File format is wrong.\n");
            return 1;
        }
            //Kuinka monta kertaa sama char on mainittu
        for (int i = 0; i < count; i++) {
            fputc(chr, file_output);
        }
    }
    return 0;
}

int main(int arg, char *argv[]) {
    if (arg < 2) {
        printf("Not enough arguments.\n");
        return 1;
    }
    //Käydään läpi tiivistetyt tiedostot
    for (int i = 1; i < arg; i++) {
        FILE *file = fopen(argv[i], "rb"); //avataan binääri tilassa ("rb")
        if (file == NULL) {
            printf("File cannot be opened for some reason.\n");
            return 1;
        }
        my_unzip(file, stdout);
        fclose(file);
    }

    return 0;
}