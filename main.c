#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

void redir(const char *inp, const char *cmd, const char *out){
    pid_t pid;
    int input_file, output_file;

    if (strcmp(inp, "-") == 0) {
        input_file = STDIN_FILENO;
    } else {
        input_file = open(inp, O_RDONLY);
        if (input_file < 0) {
            perror("Cannot open input file");
            exit(EXIT_FAILURE);
        }
    }
    if (strcmp(out, "-") == 0) {
        output_file = STDOUT_FILENO;
    } else {
        output_file = open(out, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (output_file < 0) {
            perror("Cannot open output file");
            close(input_file);
            exit(EXIT_FAILURE);
        }
    }
    //fork
    if ((pid = fork()) == 0) {
        if(dup2(input_file, STDIN_FILENO) < 0) {
            perror("failure in dup2 input file");
            exit(EXIT_FAILURE);
        }
        if(dup2(output_file, STDOUT_FILENO) < 0) {
            perror("failure in dup2 output file");
            exit(EXIT_FAILURE);
        }
        close(input_file);
        close(output_file);

        char *args[64];
        char *copy_of_cmd = strdup(cmd);
        char *token;
        int i = 0;

        token = strtok(copy_of_cmd, " ");
        while (token != NULL) {
            args[i++] = token;
            token = strtok(NULL, " ");
        }
        
        args[i] = NULL;

        execvp(args[0], args);
        perror("execvp has failed");
        free(copy_of_cmd);
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("fork has failed");
        exit(EXIT_FAILURE);
    }

    //process
    close(input_file);
    close(output_file);
    int status;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status)) {
        printf("Child has exited. Status: %d\n", WEXITSTATUS(status));
    }
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <inp> <cmd> <out>\n", argv[0]);
        return EXIT_FAILURE;
    }
    redir(argv[1], argv[2], argv[3]);
    return EXIT_SUCCESS;
}
