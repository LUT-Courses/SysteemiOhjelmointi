#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_ARGS 128
#define MAX_COMMANDS 64

const char error_message[30] = "An error has occurred\n";
char **paths = NULL;
int path_count = 0;

void print_error();
void init_path();
void free_paths();
void add_path(char *p);
int handle_builtin(char **args);
void parse_parallel(char *line, char **commands, int *num_commands);
void parse_args(char *command, char **args, char **outfile);
void execute_external(char **args, char *outfile);

void print_error() {
    write(STDERR_FILENO, error_message, strlen(error_message));
}

void free_paths() {
    for (int i = 0; i < path_count; i++) free(paths[i]);
    free(paths);
    paths = NULL;
    path_count = 0;
}

void add_path(char *p) {
    paths = realloc(paths, sizeof(char *) * (path_count + 1));
    paths[path_count] = strdup(p);
    path_count++;
}

int handle_builtin(char **args) {
    if (args[0] == NULL) return 1;

    if (strcmp(args[0], "exit") == 0) {
        if (args[1] != NULL) {
            print_error();
            return 1;
        }
        exit(0);
    }

    if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL || args[2] != NULL) {
            print_error();
        } else if (chdir(args[1]) != 0) {
            print_error();
        }
        return 1;
    }
    if (strcmp(args[0], "path") == 0) {
        free_paths();
        path_count = 0;
        for (int i = 1; args[i] != NULL; i++) {
            add_path(args[i]);
        }
        return 1;
    }

    return 0;
}

void parse_parallel(char *line, char **commands, int *num_commands) {
    char *token = strtok(line, "&");
    *num_commands = 0;
    while (token != NULL) {
        commands[(*num_commands)++] = token;
        token = strtok(NULL, "&");
    }
    commands[*num_commands] = NULL;
}

void parse_args(char *command, char **args, char **outfile) {
    *outfile = NULL;
    char *redir = strchr(command, '>');
    if (redir != NULL) {
        *redir = '\0';
        redir++;
        while (*redir == ' ' || *redir == '\t') redir++;
        char *filename = strtok(redir, " \t\n");
        if (filename == NULL || strtok(NULL, " \t\n") != NULL) {
            print_error();
            *outfile = NULL;
            args[0] = NULL;
            return;
        }
        *outfile = filename;
    }

    int i = 0;
    char *token = strtok(command, " \t\n");
    while (token != NULL && i < MAX_ARGS - 1) {
        args[i++] = token;
        token = strtok(NULL, " \t\n");
    }
    args[i] = NULL;
}

void execArgsRedirect(char **parsed, char *output_file) {
    pid_t p1 = fork(); 
    if (p1 < 0) {
        print_error(NULL);
        return;
    }
    if (p1 == 0) { //Child
        if (output_file != NULL) {
            // Avataan kirjoittamista varten
            int outfile_fd = open(output_file, O_CREAT | O_WRONLY | O_TRUNC, 0666);
            if (outfile_fd < 0) {
                print_error(NULL);
                exit(1);
            }
            // Uudelleen suuntaus
            dup2(outfile_fd, STDOUT_FILENO);
            dup2(outfile_fd, STDERR_FILENO);
            close(outfile_fd);
        }
        if (execvp(parsed[0], parsed) < 0) {
            print_error(NULL);
            exit(1);
        }
    } else { //Parent
        waitpid(pid, NULL, 0);
    }
}

int main(int argc, char *argv[]) {
    FILE *input = stdin;

    // Handle optional input file
    if (argc > 2) {
        print_error(NULL);
        exit(EXIT_FAILURE);
    }
    if (argc == 2) {
        input = fopen(argv[1], "r");
        if (!input) {
            print_error(NULL);
            exit(EXIT_FAILURE);
        }
    }

    // Initialize path list
    paths = malloc(sizeof(char *));
    if (!paths) {
        print_error(NULL);
        exit(EXIT_FAILURE);
    }
    paths[0] = strdup("/bin");
    path_count = 1;
    char *line = NULL;
    size_t len = 0;
    while (1) {
        if (input == stdin) {
            printf("wish> ");
            fflush(stdout);
        }
        if (getline(&line, &len, input) == -1) {
            break;
        }
        char *commands[MAX_COMMANDS];
        int num_commands = 0;
        parse_parallel(line, commands, &num_commands);

        pid_t pids[MAX_COMMANDS] = {0};
        for (int i = 0; i < num_commands; i++) {
            char *args[MAX_ARGS];
            char *outfile = NULL;

            parse_args(commands[i], args, &outfile);

            if (!args[0]) continue;
            if (handle_builtin(args)) continue;
            pid_t pid = fork();
            if (pid < 0) {
                print_error(NULL);
            } else if (pid == 0) {
                execArgsRedirect(args, outfile);
                exit(EXIT_FAILURE);
            } else {
                pids[i] = pid;
            }
        }
        for (int i = 0; i < num_commands; i++) {
            if (pids[i] > 0) waitpid(pids[i], NULL, 0);
        }
    }

    free(line);
    free_paths();
    return 0;
}