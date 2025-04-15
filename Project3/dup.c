#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <sys/stat.h>
#include <ctype.h>
#include <limits.h>

// Constants
#define BUF_SIZE 1024
#define PROMPT "mysh> "
#define MODE_0640 (S_IRUSR | S_IWUSR | S_IRGRP)
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif


// List of builtin commands
char *builtIns[] = {"cd", "pwd", "which", "exit", "die"};
#define NUM_BUILTINS (sizeof(builtIns)/sizeof(builtIns[0]))

// Function Prototypes
char *read_line(int input_fd, int interactive);
char **tokenize(char *line, int *numTokens);
void free_tokens(char **tokens, int numTokens);
int has_wildcard(const char *str);
int match_pattern(const char *pattern, const char *str);
char **expand_wildcards(char **tokens, int *numTokens);
char *search_path(char *command);
int contains_slash(const char *str);
int run_builtin(char **tokens, int numTokens, int interactive);
int execute_external(char **argv, char *infile, char *outfile);
int execute_pipeline(char **tokens1, int numTokens1, char **tokens2, int numTokens2);
int process_command(char **tokens, int numTokens, int interactive, int *last_status);
int parse_redirection(char **tokens, int *numTokens, char **infile, char **outfile);

// -----------------------
// Main Function
// -----------------------
int main(int argc, char *argv[]) {
    int input_fd = STDIN_FILENO;
    if (argc == 2) {
        input_fd = open(argv[1], O_RDONLY);
        if (input_fd == -1) {
            perror("Failed to open file");
            exit(EXIT_FAILURE);
        }
    }

    int interactive = isatty(input_fd);
    int last_status = 0;
    if (interactive) {
        printf("Welcome to my shell!\n");
    }

    while (1) {
        if (interactive) {
            printf(PROMPT);
            fflush(stdout);
        }
        char *line = read_line(input_fd, interactive);
        if (!line) break;  // End-of-File
    
        int numTokens = 0;
        char **tokens = tokenize(line, &numTokens);
        free(line);
    
        if (numTokens == 0) {
            free_tokens(tokens, numTokens);
            continue;
        }
    
        // Remove comments and handle blank lines...
        for (int i = 0; i < numTokens; i++) {
            if (tokens[i][0] == '#') {
                numTokens = i;
                break;
            }
        }
        if (numTokens == 0) {
            free_tokens(tokens, numTokens);
            continue;
        }
    
        int ret = process_command(tokens, numTokens, interactive, &last_status);
        // Do NOT call free_tokens(tokens, numTokens) here since process_command has taken care of freeing tokens.
        if (ret == -2) { // exit or die encountered
            break;
        }
    }
    

    if (interactive)
        printf("Exiting my shell.\n");

    if (input_fd != STDIN_FILENO)
        close(input_fd);
    return EXIT_SUCCESS;
}

// -----------------------
// Reads a full line from input_fd (using read()).
// Returns a malloced null-terminated string (caller must free).
// -----------------------
char *read_line(int input_fd, int interactive) {
    char *buffer = malloc(BUF_SIZE);
    if (!buffer) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    int pos = 0;
    char ch;
    ssize_t nread;
    
    while ((nread = read(input_fd, &ch, 1)) > 0) {
        if (ch == '\n') {
            break;
        }
        if (pos < BUF_SIZE - 1) {
            buffer[pos++] = ch;
        }
    }
    // If we read no data and EOF reached.
    if (nread == 0 && pos == 0) {
        free(buffer);
        return NULL;
    }
    buffer[pos] = '\0';
    return buffer;
}

// -----------------------
// Tokenize the input line by whitespace.
// Returns an array of strings (tokens) and sets numTokens.
// -----------------------
char **tokenize(char *line, int *numTokens) {
    int capacity = 10;
    char **tokens = malloc(capacity * sizeof(char *));
    if (!tokens) {
        perror("malloc tokens");
        exit(EXIT_FAILURE);
    }
    int count = 0;
    char *p = line;
    while (*p) {
        while (*p && isspace(*p)) p++;
        if (*p == '\0')
            break;
        char *start = p;
        while (*p && !isspace(*p)) p++;
        int len = p - start;
        char *token = malloc(len + 1);
        if (!token) {
            perror("malloc token");
            exit(EXIT_FAILURE);
        }
        strncpy(token, start, len);
        token[len] = '\0';
        if (count >= capacity) {
            capacity *= 2;
            tokens = realloc(tokens, capacity * sizeof(char *));
            if (!tokens) {
                perror("realloc tokens");
                exit(EXIT_FAILURE);
            }
        }
        tokens[count++] = token;
    }
    *numTokens = count;
    return tokens;
}

void free_tokens(char **tokens, int numTokens) {
    for (int i = 0; i < numTokens; i++) {
        free(tokens[i]);
    }
    free(tokens);
}

// -----------------------
// Check if a token contains a wildcard character '*'
// -----------------------
int has_wildcard(const char *str) {
    return (strchr(str, '*') != NULL);
}

// -----------------------
// Match a filename against a simple wildcard pattern containing one '*'.
// The pattern is split into prefix and suffix.
// -----------------------
int match_pattern(const char *pattern, const char *str) {
    char *star = strchr(pattern, '*');
    if (!star) {
        return strcmp(pattern, str) == 0;
    }
    int prefix_len = star - pattern;
    int suffix_len = strlen(star + 1);
    if (strncmp(pattern, str, prefix_len) != 0)
        return 0;
    int str_len = strlen(str);
    if (str_len < prefix_len + suffix_len)
        return 0;
    if (suffix_len > 0 && strcmp(star + 1, str + str_len - suffix_len) != 0)
        return 0;
    return 1;
}

// -----------------------
// Expand wildcard tokens.
// For each token that contains '*', open the directory (if a slash is present use that directory,
// otherwise use current directory) and replace the token with matching filenames.
// If no match, leave the token unchanged.
// -----------------------
char **expand_wildcards(char **tokens, int *numTokens) {
    // We'll build a new token list.
    int capacity = *numTokens;
    char **newTokens = malloc(capacity * sizeof(char *));
    if (!newTokens) {
        perror("malloc newTokens");
        exit(EXIT_FAILURE);
    }
    int count = 0;
    for (int i = 0; i < *numTokens; i++) {
        if (has_wildcard(tokens[i])) {
            // Determine directory and pattern.
            char *slash = strrchr(tokens[i], '/');
            char dirpath[BUF_SIZE];
            char *pattern;
            if (slash) {
                int len = slash - tokens[i];
                strncpy(dirpath, tokens[i], len);
                dirpath[len] = '\0';
                pattern = slash + 1;
            } else {
                strcpy(dirpath, ".");
                pattern = tokens[i];
            }
            DIR *dir = opendir(dirpath);
            if (dir) {
                struct dirent *entry;
                int matched = 0;
                while ((entry = readdir(dir)) != NULL) {
                    // Ignore hidden files unless pattern starts with '.'
                    if (entry->d_name[0] == '.' && pattern[0] != '.')
                        continue;
                    if (match_pattern(pattern, entry->d_name)) {
                        // Build full token if directory was specified.
                        char fullpath[PATH_MAX];
                        if (slash) {
                            snprintf(fullpath, PATH_MAX, "%s/%s", dirpath, entry->d_name);
                        } else {
                            snprintf(fullpath, PATH_MAX, "%s", entry->d_name);

                        }
                        if (count >= capacity) {
                            capacity *= 2;
                            newTokens = realloc(newTokens, capacity * sizeof(char *));
                            if (!newTokens) { perror("realloc newTokens"); exit(EXIT_FAILURE); }
                        }
                        newTokens[count++] = strdup(fullpath);
                        matched = 1;
                    }
                }
                closedir(dir);
                if (!matched) {
                    // No match found; keep original token.
                    if (count >= capacity) {
                        capacity *= 2;
                        newTokens = realloc(newTokens, capacity * sizeof(char *));
                        if (!newTokens) { perror("realloc newTokens"); exit(EXIT_FAILURE); }
                    }
                    newTokens[count++] = strdup(tokens[i]);
                }
            } else {
                // If directory could not be opened, simply keep token.
                if (count >= capacity) {
                    capacity *= 2;
                    newTokens = realloc(newTokens, capacity * sizeof(char *));
                    if (!newTokens) { perror("realloc newTokens"); exit(EXIT_FAILURE); }
                }
                newTokens[count++] = strdup(tokens[i]);
            }
        } else {
            // Not a wildcard.
            if (count >= capacity) {
                capacity *= 2;
                newTokens = realloc(newTokens, capacity * sizeof(char *));
                if (!newTokens) { perror("realloc newTokens"); exit(EXIT_FAILURE); }
            }
            newTokens[count++] = strdup(tokens[i]);
        }
    }
    // Free old tokens.
    for (int i = 0; i < *numTokens; i++) {
        free(tokens[i]);
    }
    free(tokens);
    *numTokens = count;
    return newTokens;
}

// -----------------------
// Search for the command in /usr/local/bin, /usr/bin, /bin if no '/' is in the command.
// Returns a malloced string with the full path if found, or NULL if not found.
// -----------------------
char *search_path(char *command) {
    char *dirs[] = {"/usr/local/bin", "/usr/bin", "/bin"};
    int num_dirs = 3;
    for (int i = 0; i < num_dirs; i++) {
        char path[BUF_SIZE];
        snprintf(path, BUF_SIZE, "%s/%s", dirs[i], command);
        if (access(path, X_OK) == 0) {
            return strdup(path);
        }
    }
    return NULL;
}

// -----------------------
// Check if a string contains a '/' character.
// -----------------------
int contains_slash(const char *str) {
    return (strchr(str, '/') != NULL);
}

// -----------------------
// run_builtin executes built-in commands.
// Returns 0 on success, -1 on error, and -2 if the shell should terminate (exit or die).
// -----------------------
int run_builtin(char **tokens, int numTokens, int interactive) {
    if (strcmp(tokens[0], "cd") == 0) {
        if (numTokens != 2) {
            fprintf(stderr, "cd: expected exactly one argument\n");
            return 1;
        }
        if (chdir(tokens[1]) == -1) {
            perror("cd");
            return 1;
        }
        return 0;
    }
    else if (strcmp(tokens[0], "pwd") == 0) {
        if (numTokens != 1) {
            fprintf(stderr, "pwd: no arguments expected\n");
            return 1;
        }
        char cwd[BUF_SIZE];
        if (getcwd(cwd, BUF_SIZE) == NULL) {
            perror("pwd");
            return 1;
        }
        printf("%s\n", cwd);
        return 0;
    }
    else if (strcmp(tokens[0], "which") == 0) {
        if (numTokens != 2) {
            fprintf(stderr, "which: expected exactly one argument\n");
            return 1;
        }
        // Do not allow built-ins.
        for (int i = 0; i < NUM_BUILTINS; i++) {
            if (strcmp(tokens[1], builtIns[i]) == 0) {
                fprintf(stderr, "which: %s is a built-in command\n", tokens[1]);
                return 1;
            }
        }
        char *path = search_path(tokens[1]);
        if (path) {
            printf("%s\n", path);
            free(path);
            return 0;
        } else {
            fprintf(stderr, "which: %s not found\n", tokens[1]);
            return 1;
        }
    }
    else if (strcmp(tokens[0], "exit") == 0) {
        // Exit the shell successfully.
        return -2;
    }
    else if (strcmp(tokens[0], "die") == 0) {
        // Print any arguments and exit with failure.
        if (numTokens > 1) {
            for (int i = 1; i < numTokens; i++) {
                fprintf(stderr, "%s ", tokens[i]);
            }
            fprintf(stderr, "\n");
        }
        return -2;
    }
    return -1; // not a built-in
}

// -----------------------
// execute_external forks a child to run an external command.
// It handles redirections if infile or outfile is non-NULL.
// Returns the child's exit status.
// -----------------------
int execute_external(char **argv, char *infile, char *outfile) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }
    if (pid == 0) {
        // Child process.
        // Handle input redirection.
        if (infile) {
            int fd_in = open(infile, O_RDONLY);
            if (fd_in < 0) {
                perror("open input file");
                exit(1);
            }
            dup2(fd_in, STDIN_FILENO);
            close(fd_in);
        }
        // Handle output redirection.
        if (outfile) {
            int fd_out = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, MODE_0640);
            if (fd_out < 0) {
                perror("open output file");
                exit(1);
            }
            dup2(fd_out, STDOUT_FILENO);
            close(fd_out);
        }
        // Determine program path.
        char *prog;
        if (contains_slash(argv[0])) {
            prog = argv[0];
        } else {
            prog = search_path(argv[0]);
            if (!prog) {
                fprintf(stderr, "Command not found: %s\n", argv[0]);
                exit(1);
            }
        }
        execv(prog, argv);
        perror("execv");
        exit(1);
    } else {
        // Parent: wait for child.
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status))
            return WEXITSTATUS(status);
        return 1;
    }
}

// -----------------------
// execute_pipeline executes two commands connected by a pipe.
// (Subcommands in a pipeline are assumed not to have redirection.)
//
// Returns the exit status of the second command (the right side).
// -----------------------
int execute_pipeline(char **tokens1, int numTokens1, char **tokens2, int numTokens2) {
    int pipefd[2];
    if (pipe(pipefd) < 0) {
        perror("pipe");
        return 1;
    }
    pid_t pid1 = fork();
    if (pid1 < 0) {
        perror("fork");
        return 1;
    }
    if (pid1 == 0) {
        // Left process: redirect stdout to pipe write end.
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);
        // Execute built-in? For pipeline, we treat built-ins like external commands.
        char *prog;
        if (contains_slash(tokens1[0])) {
            prog = tokens1[0];
        } else {
            prog = search_path(tokens1[0]);
            if (!prog) { 
                fprintf(stderr, "Command not found: %s\n", tokens1[0]);
                exit(1);
            }
        }
        execv(prog, tokens1);
        perror("execv");
        exit(1);
    }
    pid_t pid2 = fork();
    if (pid2 < 0) {
        perror("fork");
        return 1;
    }
    if (pid2 == 0) {
        // Right process: redirect stdin to pipe read end.
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[1]);
        close(pipefd[0]);
        char *prog;
        if (contains_slash(tokens2[0])) {
            prog = tokens2[0];
        } else {
            prog = search_path(tokens2[0]);
            if (!prog) { 
                fprintf(stderr, "Command not found: %s\n", tokens2[0]);
                exit(1);
            }
        }
        execv(prog, tokens2);
        perror("execv");
        exit(1);
    }
    // Parent closes pipe and waits.
    close(pipefd[0]);
    close(pipefd[1]);
    int status;
    waitpid(pid1, &status, 0);
    waitpid(pid2, &status, 0);
    if (WIFEXITED(status))
        return WEXITSTATUS(status);
    return 1;
}

// -----------------------
// parse_redirection looks for "<" and ">" tokens in the array,
// sets *infile and *outfile accordingly, and removes those tokens from the token list.
// Returns 0 on success, 1 on error.
// -----------------------
int parse_redirection(char **tokens, int *numTokens, char **infile, char **outfile) {
    int i = 0;
    while (i < *numTokens) {
        if (strcmp(tokens[i], "<") == 0) {
            if (i + 1 >= *numTokens) {
                fprintf(stderr, "Syntax error near '<'\n");
                return 1;
            }
            *infile = strdup(tokens[i+1]);
            // Remove these two tokens by shifting the array.
            for (int j = i; j < *numTokens - 2; j++) {
                tokens[j] = tokens[j+2];
            }
            *numTokens -= 2;
            continue;
        }
        else if (strcmp(tokens[i], ">") == 0) {
            if (i + 1 >= *numTokens) {
                fprintf(stderr, "Syntax error near '>'\n");
                return 1;
            }
            *outfile = strdup(tokens[i+1]);
            for (int j = i; j < *numTokens - 2; j++) {
                tokens[j] = tokens[j+2];
            }
            *numTokens -= 2;
            continue;
        }
        i++;
    }
    return 0;
}

// -----------------------
// process_command handles one lines tokens.
// It checks for conditionals, pipeline, performs wildcard expansion,
// parses redirection, and executes the command.
// Returns -2 if the shell should exit (exit/die); otherwise returns the commands exit status.
// -----------------------
int process_command(char **tokens, int numTokens, int interactive, int *last_status) {
    // Check for conditionals ("and" or "or") at the beginning.
    int conditional = 0; // 0 = none, 1 = and, 2 = or
    if (strcmp(tokens[0], "and") == 0) {
        conditional = 1;
        // Remove the conditional token.
        for (int i = 0; i < numTokens - 1; i++) {
            tokens[i] = tokens[i+1];
        }
        numTokens--;
        if (*last_status != 0) {
            // previous command failed so skip this command.
            return 0;
        }
    }
    else if (strcmp(tokens[0], "or") == 0) {
        conditional = 2;
        for (int i = 0; i < numTokens - 1; i++) {
            tokens[i] = tokens[i+1];
        }
        numTokens--;
        if (*last_status == 0) {
            // previous command succeeded so skip this command.
            return 0;
        }
    }
    
    // Check if the command contains a pipe.
    int pipe_index = -1;
    for (int i = 0; i < numTokens; i++) {
        if (strcmp(tokens[i], "|") == 0) {
            pipe_index = i;
            break;
        }
    }
    
    if (pipe_index != -1) {
        // Split the tokens array into two independent arrays for left and right commands.
        int numTokens1 = pipe_index;
        int numTokens2 = numTokens - pipe_index - 1;
        if (numTokens2 <= 0) {
            fprintf(stderr, "Syntax error: missing command after pipe\n");
            return 1;
        }

        // Allocate new arrays for left and right tokens.
        char **leftTokens = malloc(numTokens1 * sizeof(char *));
        char **rightTokens = malloc(numTokens2 * sizeof(char *));
        if (!leftTokens || !rightTokens) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }

        // Copy the left tokens.
        for (int i = 0; i < numTokens1; i++) {
            leftTokens[i] = strdup(tokens[i]);
        }
        // Copy the right tokens.
        for (int i = 0; i < numTokens2; i++) {
            rightTokens[i] = strdup(tokens[pipe_index + 1 + i]);
        }
        // tokens = expand_wildcards(tokens, &numTokens);

        leftTokens = expand_wildcards(leftTokens, &numTokens1);
        rightTokens = expand_wildcards(rightTokens, &numTokens2);

        int status = execute_pipeline(leftTokens, numTokens1, rightTokens, numTokens2);
        free_tokens(leftTokens, numTokens1);
        free_tokens(rightTokens, numTokens2);
        *last_status = status;
        return status;        
    }
    
    // For a single command, first parse redirection.
    char *infile = NULL, *outfile = NULL;
    if (parse_redirection(tokens, &numTokens, &infile, &outfile)) {
        // Parsing error.
        free(infile);
        free(outfile);
        return 1;
    }
    
    // Expand wildcards.
    
    

    // NULL-terminate the token array for exec functions.
    tokens = realloc(tokens, (numTokens + 1) * sizeof(char *));
    tokens[numTokens] = NULL;
    
    // Check if the command is a built-in.
    int builtin_status = run_builtin(tokens, numTokens, interactive);
    if (builtin_status != -1) {
        // built-in command executed
        *last_status = (builtin_status >= 0 ? builtin_status : 0);
        return builtin_status;
    }
    
    // Otherwise, execute as an external command.
    int status = execute_external(tokens, infile, outfile);
    *last_status = status;
    
    free(infile);
    free(outfile);
    return status;
}
