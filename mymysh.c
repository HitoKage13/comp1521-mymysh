// mysh.c ... a small shell
// Started by John Shepherd, September 2018
// Completed by Jeremy Lim (z5209627), September/October 2018
// Version 9 (06/10)

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <glob.h>
#include <assert.h>
#include <fcntl.h>
#include "history.h"

// This is defined in string.h
// BUT ONLY if you use -std=gnu99
//extern char *strdup(char *);

// Function forward references

void trim(char *);
int strContains(char *, char *);
char **tokenise(char *, char *);
char **fileNameExpand(char **);
void freeTokens(char **);
char *findExecutable(char *, char **);
int isExecutable(char *);
void prompt(void);
void execute(char **args, char **path, char **envp);
void printDir();


// Global Constants

#define MAXLINE 200

// Global Data

/* none ... unless you want some */


// Main program
// Set up enviroment and then run main loop
// - read command, execute command, repeat

int main(int argc, char *argv[], char *envp[]) {
    pid_t pid;   // pid of child process
    int stat;    // return status of child
    char **path; // array of directory names
    int cmdNo;   // command number
    int i;       // generic index

    // set up command PATH from environment variable
    for (i = 0; envp[i] != NULL; i++) {
        if (strncmp(envp[i], "PATH=", 5) == 0) break;
    }
    if (envp[i] == NULL)
        path = tokenise("/bin:/usr/bin",":");
    else
        // &envp[i][5] skips over "PATH=" prefix
        path = tokenise(&envp[i][5],":");
#ifdef DBUG
    for (i = 0; path[i] != NULL;i++)
        printf("path[%d] = %s\n",i,path[i]);
#endif

    // initialise command history
    // - use content of ~/.mymysh_history file if it exists
    cmdNo = initCommandHistory();
    // main loop: print prompt, read line, execute command

    char line[MAXLINE];

    prompt();
    while (fgets(line, MAXLINE, stdin) != NULL) {
        trim(line); // remove leading/trailing space

        // Empty command
        if (!strcmp(line,"")) {
            prompt();
            continue;
        }

        if (strchr(line,'|') != NULL) {
            printf("Pipelines not implemented\n");
            prompt();
            continue;
        }

        // ! History substitution
        int histNo;

        if (line[0] == '!') {
            // For if the command is '!!', '!!!', etc.
            if (line[1] == '!') {
                strcpy(line,getCommandFromHistory(cmdNo-1));
                printf("%s\n",line);
            } // To get the seqNumber from command history.
            else {
                char *number = &line[1];
                histNo = atoi(number);
                if (histNo == 0) {
                    printf("Invalid history substitution\n");
                    prompt();
                    continue;
                } else if (histNo >= cmdNo - 20 && histNo < cmdNo) {
                    strcpy(line,getCommandFromHistory(histNo));
                    printf("%s\n",line);
                } else if (histNo < cmdNo - 20 || histNo > cmdNo) {
                    printf("No command #%d\n",histNo);
                    prompt();
                    continue;
                }
            }
        }

        // Tokenise
        char **args = tokenise(line," ");

        i = 0;

        // Expand filename wildcards
        if (strchr(line,'*') != NULL || strchr(line,'?') != NULL
        || strchr(line,'[') != NULL || strchr(line,'~') != NULL) {
            args = fileNameExpand(args);
        }

        // Built-in shell commands
        if (!strcmp(line,"exit")) {
            saveCommandHistory();
            cleanCommandHistory();
            printf("\n");
            return(EXIT_SUCCESS);
        } else if (!strcmp(line,"h") || !strcmp(line,"history")) {
            showCommandHistory();
            addToCommandHistory(line,cmdNo);
            cmdNo++;
            prompt();
            continue;
        } else if (!strcmp(line,"pwd")) {
            printDir();
            addToCommandHistory(line,cmdNo);
            cmdNo++;
            prompt();
            continue;
        } else if (!strcmp(args[0],"cd")) {
            if (!strcmp(line,"cd")) {
                char *homedir = strdup(getenv("HOME"));
                chdir(homedir);
                printDir();
            } else {
                if (chdir(args[1]) == -1) {
                    printf("%s: No such file or directory\n",args[1]);
                } else {
                    printDir();
                }
            }
            addToCommandHistory(line,cmdNo);
            cmdNo++;
            prompt();
            continue;
        }

        // Check for input/output redirections
        int j = 0;
        int in = 0;
        int out = 0;
        int counter = 0;

        while (args[j] != NULL) {
            if (!strcmp(args[j],"<") || !strcmp(args[j],">")) {
                counter++;
            }
            j++;
        }

        // If there is more than 1 redirection in the line
        if (counter > 1) {
            printf("Invalid i/o redirection\n");
            prompt();
            continue;
        }

        j = 0;

        while (args[j] != NULL) {
            // Firstly, it checks for more than 1 redirection

            // Input
            if (!strcmp(args[j],"<")) {
                in = 1;
                break;
            } // Output
            else if (!strcmp(args[j],">")) {
                out = 1;
                break;
            }

            j++;
        }

        if (in || out) {
            // For case when the second last token is not '<' or '>'
            if (args[j+2] != NULL) {
                printf("Invalid i/o redirection\n");
                prompt();
                continue;
            }

            // For case when the file does not exist
            if (in) {
                int wowFd = open(args[j+1],O_RDONLY);
                if (access(args[j+1],R_OK) == 0) {
                    close(wowFd);
                } else {
                    printf("Input redirection: No such file or directory\n");
                    prompt();
                    continue;
                }
            }
        }

        // TODO
        // Code to implement mainloop goes here
        // Uses
        // - addToCommandHistory()
        // - showCommandHistory()
        // - and many other functions
        // TODO

        // Read and execute commands
        pid = fork();

        if (pid < 0) {
            fprintf(stderr, "Failed to create child process\n");
            exit(1);
        } else if (pid > 0) {
            // This is the parent process

            // This is done here AND in execute(),
            // to print the Executing command message
            // It is a bit repetitive, but I believe it is necessary
            // to do I/O redirections without messing up
            // the other test cases.
            if (findExecutable(args[0],path) != NULL) {
                char* command = NULL;

                if (args[0][0] == '/' || args[0][0] == '.') {
                    if (isExecutable(args[0]))
                        command = strdup(args[0]);
                } else
                for (i = 0; path[i] != NULL; i++) {
                    char* filePath = malloc(strlen(path[i]) + strlen(args[0]) + 2);
                    strcpy(filePath, path[i]);
                    strcat(filePath, "/");
                    strcat(filePath, args[0]);
                    if (isExecutable(filePath)) {
                        command = strdup(filePath);
                        free(filePath);
                        break;
                    }
                    free(filePath);
                }
                printf("Running %s ...\n",command);
                printf("--------------------\n");
            } else {
                printf("%s: Command not found\n",args[0]);
            }

            wait(&stat);

            if (findExecutable(args[0],path) != NULL) {
                printf("--------------------\n");
                printf("Return %d\n",WEXITSTATUS(stat));
                addToCommandHistory(line,cmdNo);
                cmdNo++;
            }
            freeTokens(args);
        } else {
            // This is the child process

            // Sorts out redirections
            if (in) {
                int inFd = open(args[j+1],O_RDONLY);
                if (access(args[j+1],R_OK) == 0) {
                    if (dup2(inFd,0) < 0) {
                        prompt();
                        continue;
                    }

                    free(args[j]);
                    args[j] = NULL;
                    free(args[j+1]);
                    args[j+1] = NULL;
                }
            } else if (out) {
                int outFd = open(args[j+1],O_WRONLY|O_CREAT,0666);
                if (access(args[j+1],W_OK) == 0) {
                    if (dup2(outFd,1) < 0) {
                        prompt();
                        continue;
                    }

                    free(args[j]);
                    args[j] = NULL;
                    free(args[j+1]);
                    args[j+1] = NULL;
                }
            }

            execute(args,path,envp);
        }

        prompt();
    }
    saveCommandHistory();
    cleanCommandHistory();
    // fclose(fp);
    printf("\n");
    return(EXIT_SUCCESS);
}

// fileNameExpand: expand any wildcards in command-line args
// - returns a possibly larger set of tokens
char **fileNameExpand(char **tokens) {
    int i = 0;
    char *s = malloc((2*(MAXLINE+1)*sizeof(char)));
    while (tokens[i] != NULL) {
        if (strchr(tokens[i],'*') != NULL || strchr(tokens[i],'?') != NULL
        || strchr(tokens[i],'[') != NULL || strchr(tokens[i],'~') != NULL) {
            glob_t globbuf = {0};
            glob(tokens[i], GLOB_NOCHECK|GLOB_TILDE, NULL, &globbuf);
            int j = 0;
            // Reiterates through the array of tokens and adds into the string
            while (j != globbuf.gl_pathc) {
                strcat(s,globbuf.gl_pathv[j]);
                strcat(s," ");
                j++;
            }

        } else if (i == 0) {
            strcpy(s,tokens[i]);
            strcat(s," ");
        } else {
            strcat(s,tokens[i]);
            strcat(s," ");
        }
        i++;
    }

    // Retokenise args
    char **args = tokenise(s," ");
    free(s);
    return args;
}

// findExecutable: look for executable in PATH
char *findExecutable(char *cmd, char **path) {
    char executable[MAXLINE];
    executable[0] = '\0';
    if (cmd[0] == '/' || cmd[0] == '.') {
        strcpy(executable, cmd);
        if (!isExecutable(executable))
            executable[0] = '\0';
    }
    else {
        int i;
        for (i = 0; path[i] != NULL; i++) {
            sprintf(executable, "%s/%s", path[i], cmd);
            if (isExecutable(executable)) break;
        }
        if (path[i] == NULL) executable[0] = '\0';
    }
    if (executable[0] == '\0')
        return NULL;
    else
        return strdup(executable);
}

// isExecutable: check whether this process can execute a file
int isExecutable(char *cmd) {
    struct stat s;
    // must be accessible
    if (stat(cmd, &s) < 0)
        return 0;
    // must be a regular file
    //if (!(s.st_mode & S_IFREG))
    if (!S_ISREG(s.st_mode))
        return 0;
    // if it's owner executable by us, ok
    if (s.st_uid == getuid() && s.st_mode & S_IXUSR)
        return 1;
    // if it's group executable by us, ok
    if (s.st_gid == getgid() && s.st_mode & S_IXGRP)
        return 1;
    // if it's other executable by us, ok
    if (s.st_mode & S_IXOTH)
        return 1;
    return 0;
}

// tokenise: split a string around a set of separators
// create an array of separate strings
// final array element contains NULL
char **tokenise(char *str, char *sep) {
    // temp copy of string, because strtok() mangles it
    char *tmp;
    // count tokens
    tmp = strdup(str);
    int n = 0;
    strtok(tmp, sep); n++;
    while (strtok(NULL, sep) != NULL) n++;
    free(tmp);
    // allocate array for argv strings
    char **strings = malloc((n+1)*sizeof(char *));
    assert(strings != NULL);
    // now tokenise and fill array
    tmp = strdup(str);
    char *next; int i = 0;
    next = strtok(tmp, sep);
    strings[i++] = strdup(next);
    while ((next = strtok(NULL,sep)) != NULL)
        strings[i++] = strdup(next);
    strings[i] = NULL;
    free(tmp);
    return strings;
}

// freeTokens: free memory associated with array of tokens
void freeTokens(char **toks)
{
    for (int i = 0; toks[i] != NULL; i++)
        free(toks[i]);
    free(toks);
}

// trim: remove leading/trailing spaces from a string
void trim(char *str) {
    int first, last;
    first = 0;
    while (isspace(str[first])) first++;
    last  = strlen(str)-1;
    while (isspace(str[last])) last--;
    int i, j = 0;
    for (i = first; i <= last; i++) str[j++] = str[i];
    str[j] = '\0';
}

// strContains: does the first string contain any char from 2nd string?
int strContains(char *str, char *chars) {
    for (char *s = str; *s != '\0'; s++) {
        for (char *c = chars; *c != '\0'; c++) {
            if (*s == *c) return 1;
        }
    }
    return 0;
}

// prompt: print a shell prompt
// done as a function to allow switching to $PS1
void prompt(void) {
    printf("mymysh$ ");
}


// execute: run a program, given command-line args, path and envp
void execute(char **args, char **path, char **envp) {
    char* command = NULL;
    if (args[0][0] == '/' || args[0][0] == '.') {
        if (isExecutable(args[0]))
            command = strdup(args[0]);
    } else {
        int i = 0;
        for (i = 0; path[i] != NULL; i++) {
            char* filePath = malloc(strlen(path[i]) + strlen(args[0]) + 2);
            strcpy(filePath, path[i]);
            strcat(filePath, "/");
            strcat(filePath, args[0]);
            if (isExecutable(filePath)) {
                command = strdup(filePath);
                free(filePath);
                break;
            }
            free(filePath);
        }
    }

    if (command != NULL) {
        if (execve(command,args,envp) == -1) {
            printf("%s: unknown type of executable\n",command);
        }
    }

    exit(EXIT_SUCCESS);
}

// Prints current working directory.
void printDir() {
    char directory[MAXLINE];
    if (getcwd(directory, sizeof(directory)) != NULL)
        printf("%s\n",directory);
    else
       perror("getcwd() error");
}
