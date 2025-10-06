#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

char **read_file(FILE *file, int *count) {
    char **lines = NULL;
    int size = 0;
    *count = 0;
    char mem[366];//varataan muisti
    while (fgets(mem, sizeof(mem), file)) {
        //Tehdään puskirista isompi jos tarve
        if (*count >= size) {
            if (size == 0) {
                size = 32;
            } else {
                size = size * 2;
            }
            lines = realloc(lines, size * sizeof(char *));
        }
        lines[*count] = strdup(mem);//Kopioi stringin
        (*count)++;
    }
    return lines; // palautetaan mainiin
}

int main(int argc, char *argv[]) {
    FILE *input_file = NULL;
    FILE *output_file = stdout;
    // Katsotaan paljonko argumentteja on annettu
    if (argc > 3) {
        fprintf(stderr, "usage: reverse <input> <output>\n");
        exit(1);
    }
//Jos annettu väh 1 arg
    if (argc >= 2) {
        input_file = fopen(argv[1], "r");//avataan tiedosto, jotta voidaan lukea
        if (!input_file) {
            fprintf(stderr, "error: cannot open file '%s'\n", argv[1]);
            exit(1);
        }
    }else {
        input_file = stdin;
    }
// Jos annettu kaksi argumenttia
    if (argc == 3) {
        if (strcmp(argv[1], argv[2]) == 0) {
            fprintf(stderr, "Input and output file must differ\n");
            fclose(input_file);
            exit(1);
        }
        output_file = fopen(argv[2], "w");//avataan tiedosto, jotta voidaan kirjoittaa
        if (!output_file) {
            fprintf(stderr, "error: cannot open file '%s'\n", argv[2]);
            fclose(input_file);
            exit(1);
        }
    }

    int count = 0;
    char **lines = read_file(input_file, &count);//luetaan kaikki rivit read_file avulla
// Käydään rivit läpi käänteisessä järjestyksessä
    for (int i = count - 1; i >= 0; i--) {
        if (lines[i][strlen(lines[i]) - 1] != '\n') {
            fprintf(output_file, "%s\n", lines[i]);
        } else {
            fputs(lines[i], output_file);//kirjoitetaan käänteinen järjestys tiedostoon
        }

        free(lines[i]); //vapautetaan muisti
    }
//suljetaan tiedostot
    if (input_file != stdin) fclose(input_file);
    if (output_file != stdout) fclose(output_file);

    return 0;
}
