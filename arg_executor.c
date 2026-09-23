#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define INPUT_SIZE 1024
#define MAX_ARGS 64

static int parse_command(char *input, char *arguments[])
{
    int argument_count = 0;
    char *token = strtok(input, " \t\n");

    while (token != NULL) {
        if (argument_count >= MAX_ARGS - 1) {
            fprintf(stderr, "Error: too many arguments (maximum is %d).\n",
                    MAX_ARGS - 1);
            return -1;
        }

        arguments[argument_count] = token;
        argument_count++;
        token = strtok(NULL, " \t\n");
    }

    arguments[argument_count] = NULL;
    return argument_count;
}

int main(void)
{
    char input[INPUT_SIZE];
    char *arguments[MAX_ARGS];

    printf("========================================\n");
    printf(" Command Argument Passing System\n");
    printf(" A small POSIX mini-shell demonstration\n");
    printf("========================================\n");
    printf("Type a command and its arguments, or 'exit'/'quit' to stop.\n\n");

    while (1) {
        int argument_count;

        printf("argsh> ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL) {
            if (ferror(stdin)) {
                perror("fgets");
                return EXIT_FAILURE;
            }

            printf("\nEnd of input. Exiting.\n");
            break;
        }

        argument_count = parse_command(input, arguments);
        if (argument_count < 0) {
            continue;
        }
        if (argument_count == 0) {
            continue;
        }

        if (strcmp(arguments[0], "exit") == 0 ||
            strcmp(arguments[0], "quit") == 0) {
            printf("Exiting argsh.\n");
            break;
        }

        printf("[Parent] Forking child to execute: %s\n", arguments[0]);
        fflush(stdout);

        pid_t child_pid = fork();
        if (child_pid < 0) {
            perror("fork");
            continue;
        }

        if (child_pid == 0) {
            execvp(arguments[0], arguments);

            fprintf(stderr, "[Child] Error: exec failed for '%s': ",
                    arguments[0]);
            perror("");
            _exit(EXIT_FAILURE);
        }

        printf("[Parent] Child PID: %ld\n", (long) child_pid);

        int wait_status;
        pid_t waited_pid;
        do {
            waited_pid = waitpid(child_pid, &wait_status, 0);
        } while (waited_pid < 0 && errno == EINTR);

        if (waited_pid < 0) {
            perror("waitpid");
            continue;
        }

        if (WIFEXITED(wait_status)) {
            printf("[Parent] Child (PID %ld) exited with status %d\n",
                   (long) child_pid, WEXITSTATUS(wait_status));
        } else if (WIFSIGNALED(wait_status)) {
            printf("[Parent] Child (PID %ld) terminated by signal %d\n",
                   (long) child_pid, WTERMSIG(wait_status));
        }
    }

    return EXIT_SUCCESS;
}
