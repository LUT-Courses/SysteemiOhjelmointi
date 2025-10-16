#include <stdio.h>
#include <stdlib.h>

void my_zip(FILE *file) {
    int i = 1;  //run lenght
    int curr = fgetc(file); //ensimmäinen char
    int next;   //seuraava char
    while ((next = fgetc(file)) != EOF) {
        if (next == curr) {
            i++; //Jos kirjain on sama niin run lenght suurenee
        } else {
            //Kirjoitetaan run lenght, jonka pituus 4 bytes ja char (1 byte)
            fwrite(&i, sizeof(int), 1, stdout);
            fwrite(&curr, sizeof(char), 1, stdout);
            i = 1;
            curr = next;
        }
    }
    fwrite(&i, sizeof(int), 1, stdout); //kirjoita viimeisin
    fwrite(&curr, sizeof(char), 1, stdout);
}

int main(int arg, char *argv[]) {
    if (arg < 2) {
        printf("Not enough arguments.\n");
        return 1;
    }

    for (int i = 1; i < arg; i++) {
        FILE *file = fopen(argv[i], "rb"); // avaa binääri tilassa
        if (file == NULL) {
            printf("File cannot be opened for some reason.\n");
            return 1;
        }
        my_zip(file);
        fclose(file);
    }

    return 0;
}