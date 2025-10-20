#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_ARGS 128
#define MAX_COMMANDS 64

char **paths = NULL;
int path_count = 0;

void print_error(const char *message);
void init_path();
void free_paths();
void add_path(char *path);
int handle_builtin_command(char **args);
void parse_parallel(char *input_line, char **commands, int *num_commands);
void parse_args(char *command, char **args, char **output_file);
void execute_external(char **args, char *outfile);

//Tulostaa virheviestin, mikäli tämä on annettu. Jos ei, antaa geneerisen virheviestin
void print_error(const char *message) {
    if (message && strlen(message)> 0){
        dprintf(STDERR_FILENO, "Error: %s\n", message);
    }
    else{
        dprintf(STDERR_FILENO, "An error has occurred\n");
    }
}
void init_path() {
    paths = malloc(sizeof(char *) * 1);
    paths[0] = strdup("/bin");
    path_count = 1;
}

//Vapauttaa tiedostopolun
void free_paths() {
    for (int i = 0; i < path_count; i++) {
        free(paths[i]);
    }
    free(paths);
    paths = NULL;
    path_count = 0;
}

void add_path(char *path) {
    paths = realloc(paths, sizeof(char *) * (path_count + 1));
    paths[path_count] = strdup(path);
    path_count++;
}

//Ajaa komennot exit, cd, path ja palauttaa 1. Mikäli komento ei ollut "built-in"-komento, palauttaa 0
int handle_builtin_command(char **args) {
    if (args[0] == NULL) {
        return 1;
    }

    if (strcmp(args[0], "exit") == 0) {
        if (args[1] != NULL) {
            print_error("exit does not take arguments");
            return 1;
        }
        exit(0);
    }

    if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL) {
            print_error("cd requires one argument");
        } else if (args[2] != NULL) {
            print_error("cd takes only one argument");
        } else if (chdir(args[1]) != 0) {
            print_error("failed to change directory");
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

//Ajaa usean komennon samanaikaisesti. Käyttää '&'-merkkiä erottamaan komennot
void parse_parallel(char *input_line, char **commands, int *num_commands) {
    char *token = strtok(input_line, "&");
    *num_commands = 0;

    while (token != NULL) {
        commands[(*num_commands)++] = token;
        token = strtok(NULL, "&");
    }
    commands[*num_commands] = NULL;
}

void parse_args(char *command, char **args, char **output_file) {
    *output_file = NULL;
    char *redir = strchr(command, '>');
    if (redir != NULL) {
        *redir = '\0';
        redir++;
        while (*redir == ' ' || *redir == '\t') redir++;
        char *filename = strtok(redir, " \t\n");
        if (filename == NULL || strtok(NULL, " \t\n") != NULL) {
            print_error("Invalid redirection");
            *output_file = NULL;
            args[0] = NULL;
            return;
        }
        *output_file = filename;
    }

    int i = 0;
    char *token = strtok(command, " \t\n");
    while (token != NULL && i < MAX_ARGS - 1) {
        args[i++] = token;
        token = strtok(NULL, " \t\n");
    }
    args[i] = NULL;
}

void execute_external(char **args, char *outfile) {
    if (args[0] == NULL) return;

    char full_path[256];
    for (int i = 0; i < path_count; i++) {
        snprintf(full_path, sizeof(full_path), "%s/%s", paths[i], args[0]);
        if (access(full_path, X_OK) == 0) {
            pid_t pid = fork();
            if (pid == 0) {
                if (outfile != NULL) {
                    int fd = open(outfile, O_CREAT | O_WRONLY | O_TRUNC, 0666);
                    if (fd < 0) {
                        print_error();
                        exit(1);
                    }
                    dup2(fd, STDOUT_FILENO);
                    dup2(fd, STDERR_FILENO);
                    close(fd);
                }
                execv(full_path, args);
                print_error();
                exit(1);
            } else if (pid > 0) {
                waitpid(pid, NULL, 0);
                return;
            } else {
                print_error();
                return;
            }
        }
    }
    print_error();
}

int main(int argc, char *argv[]) {
    FILE *input = stdin;

    if (argc > 2) {
        print_error();
        exit(1);
    } else if (argc == 2) {
        input = fopen(argv[1], "r");
        if (input == NULL) {
            print_error();
            exit(1);
        }
    }

    init_path();
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

        pid_t pids[MAX_COMMANDS];

        for (int i = 0; i < num_commands; i++) {
            char *args[MAX_ARGS];
            char *outfile = NULL;
            parse_args(commands[i], args, &outfile);

            if (args[0] == NULL) continue;
            if (handle_builtin_command(args)) continue;
            pid_t pid = fork();
            if (pid == 0) {
                execute_external(args, outfile);
                exit(0);
            } else if (pid > 0) {
                pids[i] = pid;
            } else {
                print_error();
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