#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static int read_number(const char *prompt, int minimum)
{
    char line[128], *end;
    long value;

    printf("%s", prompt);
    if (fgets(line, sizeof line, stdin) == NULL)
        return -1;
    errno = 0;
    value = strtol(line, &end, 10);
    if (end == line || errno != 0 || value < minimum || value > INT_MAX)
        return -1;
    while (isspace((unsigned char)*end))
        ++end;
    if (*end != '\0' || (line[0] && line[strlen(line) - 1] != '\n' && !feof(stdin)))
        return -1;
    return (int)value;
}

static int wait_for_voters(int count)
{
    int failed = 0;
    for (int i = 0; i < count; ++i) {
        int status;
        pid_t result;
        do {
            result = wait(&status);
        } while (result == -1 && errno == EINTR);
        if (result == -1 || !WIFEXITED(status) || WEXITSTATUS(status) != 0)
            failed = 1;
    }
    return failed;
}

int main(void)
{
    /* Avoid duplicated buffered output after fork, including redirected output. */
    setvbuf(stdout, NULL, _IONBF, 0);
    int voters = read_number("Enter the number of voters: ", 3);
    if (voters == -1) {
        fprintf(stderr, "The number of voters must be an integer >= 3.\n");
        return EXIT_FAILURE;
    }
    int rounds = read_number("Enter the number of rounds: ", 1);
    if (rounds == -1) {
        fprintf(stderr, "The number of rounds must be a positive integer.\n");
        return EXIT_FAILURE;
    }
    int (*pipes)[2] = calloc((size_t)voters, sizeof *pipes);
    if (pipes == NULL) {
        perror("calloc");
        return EXIT_FAILURE;
    }

    /* Step 2: create every pipe before creating any voters. */
    for (int i = 0; i < voters; ++i) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            for (int j = 0; j < i; ++j) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            free(pipes);
            return EXIT_FAILURE;
        }
    }

    int voter_id = -1;
    /* Step 3: only the judge forks; each child keeps its loop index. */
    for (int i = 0; i < voters; ++i) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            for (int j = 0; j < voters; ++j) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            /* Closing all read ends also releases writers through SIGPIPE. */
            wait_for_voters(i);
            free(pipes);
            return EXIT_FAILURE;
        }
        if (pid == 0) {
            voter_id = i;
            break;  /* Children continue to Step 4 without calling fork again. */
        }
    }

    for (int i = 0; i < voters; ++i) {
        if (voter_id == -1) {
            close(pipes[i][1]);
        } else {
            close(pipes[i][0]);
            if (i != voter_id)
                close(pipes[i][1]);
        }
    }
    if (voter_id != -1)
        srand((unsigned int)time(NULL) ^ (unsigned int)getpid());

    int failed = 0;
    /* Step 4: the same processes participate in every round. */
    for (int round = 0; round < rounds; ++round) {
        if (voter_id == -1) {
            int yes = 0;
            printf("\n------ Round %d -------\n", round + 1);
            for (int i = 0; i < voters; ++i) {
                unsigned char vote;
                ssize_t received;
                do {
                    received = read(pipes[i][0], &vote, sizeof vote);
                } while (received == -1 && errno == EINTR);
                if (received != sizeof vote || vote > 1) {
                    fprintf(stderr, "Failed to read vote from voter %d.\n", i);
                    failed = 1;
                    break;
                }
                yes += vote;
            }
            if (failed)
                break;
            if (yes > voters / 2)
                printf("--- Round %d: The voting result is a YES.\n", round + 1);
            else if (voters - yes > voters / 2)
                printf("--- Round %d: The voting result is a NO.\n", round + 1);
            else
                printf("--- Round %d: The voting result is a TIE.\n", round + 1);
        } else {
            unsigned char vote = (unsigned char)(rand() % 2);
            printf("Round %d: Voter %d votes: %s\n",
                   round + 1, voter_id, vote ? "YES" : "NO");
            ssize_t sent;
            do {
                sent = write(pipes[voter_id][1], &vote, sizeof vote);
            } while (sent == -1 && errno == EINTR);
            if (sent != sizeof vote) {
                perror("write");
                failed = 1;
                break;
            }
        }
    }

    if (voter_id == -1) {
        for (int i = 0; i < voters; ++i)
            close(pipes[i][0]);
        failed |= wait_for_voters(voters);
    } else {
        close(pipes[voter_id][1]);
    }
    free(pipes);
    return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
