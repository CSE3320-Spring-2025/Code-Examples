#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define BUFFER_SIZE 1024

// Function to count occurrences of a word in a given text buffer
int count_occurrences(const char *buffer, const char *word) {
    int count = 0;
    const char *ptr = buffer;
    size_t word_len = strlen(word);

    while ((ptr = strstr(ptr, word)) != NULL) {
        count++;
        ptr += word_len; // Move past the current occurrence
    }

    return count;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <filename> <word> <num_processes>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *filename = argv[1];
    char *word = argv[2];
    int num_processes = atoi(argv[3]);

    if (num_processes < 1) {
        fprintf(stderr, "Error: Number of processes must be at least 1.\n");
        return EXIT_FAILURE;
    }

    // Open file
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }

    // Determine file size
    off_t file_size = lseek(fd, 0, SEEK_END);
    if (file_size == -1) {
        perror("Error determining file size");
        close(fd);
        return EXIT_FAILURE;
    }

    lseek(fd, 0, SEEK_SET); // Reset file pointer

    // Divide the file into roughly equal parts
    off_t chunk_size = file_size / num_processes;
    int pipes[num_processes][2];

    for (int i = 0; i < num_processes; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("Pipe creation failed");
            return EXIT_FAILURE;
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("Fork failed");
            return EXIT_FAILURE;
        }

        if (pid == 0) { // Child process
            close(pipes[i][0]); // Close read end

            off_t start = i * chunk_size;
            off_t end = (i == num_processes - 1) ? file_size : start + chunk_size;

            lseek(fd, start, SEEK_SET);

            char buffer[BUFFER_SIZE + 1];
            int word_count = 0;
            ssize_t bytes_read;
            off_t pos = start;

            while (pos < end && (bytes_read = read(fd, buffer, BUFFER_SIZE)) > 0) {
                buffer[bytes_read] = '\0'; // Null-terminate buffer
                word_count += count_occurrences(buffer, word);
                pos += bytes_read;
            }

            write(pipes[i][1], &word_count, sizeof(int)); // Send count to parent
            close(pipes[i][1]);
            close(fd);
            exit(0);
        }
    }

    close(fd);

    // Parent process collects results
    int total_count = 0;
    for (int i = 0; i < num_processes; i++) {
        close(pipes[i][1]); // Close write end

        int child_count;
        read(pipes[i][0], &child_count, sizeof(int));
        close(pipes[i][0]);

        total_count += child_count;
        wait(NULL); // Wait for child to finish
    }

    printf("Total occurrences of '%s': %d\n", word, total_count);
    return EXIT_SUCCESS;
}

