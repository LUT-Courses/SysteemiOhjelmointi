#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

int main(int argc, char *argv[]) {
    FILE *input_file = NULL;
    FILE *output_file = NULL;

    if (argc > 3) {
        fprintf(stderr, "usage: reverse <input> <output>\n");
        exit(1);
    }

    if (argc >= 2) {
        input_file = fopen(argv[1], "r");
        if (!input_file) {
            fprintf(stderr, "error: cannot open file '%s'\n", argv[1]);
            exit(1);
        }
    }else {
        input_file = stdin;
    }

    if (argc == 3) {
        if (strcmp(argv[1], argv[2]) == 0) {
            fprintf(stderr, "Input and output file must differ\n");
            fclose(input_file);
            exit(1);
        }
        output_file = fopen(argv[2], "w");
        if (!output_file) {
            fprintf(stderr, "error: cannot open file '%s'\n", argv[2]);
            fclose(input_file);
            exit(1);
        }
    }

    int count = 0;
    char **lines = read_lines(input_file, &count);

    for (int i = count - 1; i >= 0; i--) {
        if (lines[i][strlen(lines[i]) - 1] != '\n') {
            fprintf(output_file, "%s\n", lines[i]);
        } else {
            fputs(lines[i], output_file);
        }

        free(lines[i]);
    }

    if (input_file != stdin) fclose(input_file);
    if (output_file != stdout) fclose(output_file);

    return 0;
}
