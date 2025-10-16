#include <stdio.h>
#include <stdlib.h>

void read_file(FILE *file) {
   
    char mem[366];
    while (fgets(mem, sizeof(mem), file)) {
        printf("%s", mem);
    }
    
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "usage: my-cat <filename>\n");
        return(0);
    }
    for (int i = 1; i < argc; i++) {
        FILE *file = fopen(argv[i], "r");
        if (file == NULL) {
            fprintf(stderr, "my-cat: cannot open file\n");
            return(0);
        }

        read_file(file);
        fclose(file);
    }
    return(0);
}