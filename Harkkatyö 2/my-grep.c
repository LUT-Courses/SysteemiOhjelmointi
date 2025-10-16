#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
//Vastaa avainsanan etsimisestä tiedostosta/ syötteestä
void search(FILE *stream, const char *search_term) {
    char *line = NULL;
    size_t len = 0;
    ssize_t nread;
    //Loopataan läpi kaikki tiedoston rivit
    while ((nread = getline(&line, &len, stream)) != -1) {
        if (strstr(line, search_term) != NULL) {
            printf("%s", line);
        }
    }

    free(line);
}

int main(int argc, char *argv[]) {
    //Jos ei anneta avainsanaa, annetaan virheilmoitus
    if (argc < 2) {
        fprintf(stderr, "usage: my-cat <searchterm>\n");
        return (1);
    }

    const char *search_term = argv[1];
    //Jos ei anneta tiedostonimeä etsitään stdin:stä
    if (argc == 2) {
        search(stdin, search_term);
        return (0);
    }
    //Loopataan läpi kaikki syötetiedostot
    for (int i = 2; i < argc; i++) {
        FILE *file = fopen(argv[i], "r");
        if (file == NULL) {
            fprintf(stderr, "my-grep: cannot open file\n");
            return(1);
        }

        search(file, search_term);
        fclose(file);
    }

    return (0);
}
