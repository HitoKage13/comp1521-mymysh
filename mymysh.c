// mysh.c ... a small shell
// Started by John Shepherd, September 2018
// Completed by Jeremy Lim (z5209627), September/October 2018
// Version 5 (30/09)

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
int execute(char **args, char **path, char **envp);
void printDir();


// Global Constants

#define MAXLINE 200

// Global Data

/* none ... unless you want some */


// Main program
// Set up enviroment and then run main loop
// - read command, execute command, repeat

int main(int argc, char *argv[], char *envp[])
{
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
    /* char *file = malloc(MAXLINE*sizeof(char));
    file = getenv("HOME");
    strcat(file,"/.mymysh_history");
    FILE *fp = fopen(file,"a"); */
    prompt();
    while (fgets(line, MAXLINE, stdin) != NULL) {
        int flag = 1;
        trim(line); // remove leading/trailing space
        //   if empty command, ignore
        // handle ! history substitution
        // tokenise
        // handle *?[~ filename expansion
        // handle shell built-ins
        // check for input/output redirections
        // find executable using first token
        // if none, then Command not found
        // sort out any redirections
        // run the command
        // print prompt

        // Empty command
        if (!strcmp(line,"")) {
            prompt();
            continue;
        }

        // ! History substitution
        /* if (!strcmp(line[0],"!")) {
            if (!strcmp(line[1],"!")) {
                snprintf(line,MAXLINE,"%s",getCommandFromHistory(cmdNo-1));
            } else if (valid number) {
                find the command
                line = getCommandFromHistory(command);
            } else if (invalid number) {
                printf("No command #%d\n",command number);
                prompt();
                continue;
            }
        }*/

        // Tokenise
        char **args = tokenise(line," ");

        // Expand filename wildcards
        i = 0;
        int j = 0;
        char *s = malloc((2*(MAXLINE+1)*sizeof(char)));

        if (strchr(line,'*') != NULL || strchr(line,'?') != NULL
        || strchr(line,'[') != NULL || strchr(line,'~') != NULL) {
            while (args[i] != NULL) {
                if (strchr(args[i],'*') != NULL || strchr(args[i],'?') != NULL
                || strchr(args[i],'[') != NULL || strchr(args[i],'~') != NULL) {
                    glob_t globbuf = {0};
                    glob(args[i], GLOB_NOCHECK|GLOB_TILDE, NULL, &globbuf);
                    int j = 0;
                    while (j != globbuf.gl_pathc) {
                        strcat(s,globbuf.gl_pathv[j]);
                        strcat(s," ");
                        j++;
                    }

                } else if (i == 0) {
                    strcpy(s,args[i]);
                    strcat(s," ");
                } else {
                    strcat(s,args[i]);
                    strcat(s," ");
                }
                i++;
            }
            printf("bloop\n");
            args = tokenise(s," ");
        }

        /* if (strchr(line,"*") != NULL || strchr(line,"?") != NULL
        || strchr(line,"[") != NULL || strchr(line,"~") != NULL) {
            if (strchr(line,"*") != NULL) {
                glob("*",GLOB_TILDE,)
            } else if (strchr(line,"?") != NULL) {

            } else if (strchr(line,"[") != NULL) {

            } else if (strchr(line,"~") != NULL) {
                glob("~",GLOB_TILDE)
            }
        } */

        // Built-in commands
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
                printf("bloop de scoop\n");
                char *test = strdup(getenv("HOME"));
                chdir(test);
                printDir();
            } else {
                chdir(args[1]);
            }
            addToCommandHistory(line,cmdNo);
            cmdNo++;
            prompt();
            continue;
        } else if (!strcmp(line,"debug")) {
            debug();
        }

        // Check for input/output redirections (WIP)
        // j = 0;
        int k = 0;

        /* if (strchr(line,'<') != NULL || strchr(line,'>') != NULL) {
            // Input
            printf("wowee\n");
            if (strchr(line,'<') != NULL) {
                while (strcmp(args[j],"<") != 0) {
                    j++;
                }
                int fd1[2];
                int fd2[2];
                if (!strcmp(args[j+2],"\0")) {
                    int length = 0;
                    while (k < j) {
                        length = length + strlen(args[k]);
                        k++;
                    }
                    k = 0;
                    char *string = malloc((length+1)*sizeof(char));
                    char *read_string = malloc((length+1)*sizeof(char));
                    while (k < j) {
                        if (k == 0) {
                            strcpy(string,args[k]);
                        } else {
                            strcat(string,args[k]);
                        }
                        k++;
                    }
                    if (pipe(fd1) < 0) {
                        printf("error\n");
                        prompt();
                        continue;
                    }

                    if (pipe(fd2) < 0) {
                        printf("error\n");
                        prompt();
                        continue;
                    }

                    pid = fork();
                    if (pid < 0) {
                        printf("error\n");
                        prompt();
                        continue;
                    }
                    if (pid > 0) {
                        close(fd1[0]);
                        write(fd1[1],string,strlen(string)+1);
                        close(fd1[1]);
                        wait(NULL);
                        close(fd2[1]);
                        read(fd2[0],read_string,strlen(read_string+1));
                        close(fd2[0]);
                    } else {
                        close(fd1[1]);
                        read(fd1[0],read_string,strlen(read_string+1));
                        k = 0;
                        close(fd1[0]);
                        close(fd2[0]);
                        write(fd2[1],read_string,strlen(read_string+1));
                        close(fd2[1]);
                    }

                    addToCommandHistory(line,cmdNo);
                    cmdNo++;
                    prompt();
                    continue;
                }
            } // Output
            else if (strchr(line,'>') != NULL) {
                printf("wowzers\n");
                while (strcmp(args[j],">") != 0) {
                    j++;
                }
                printf("wack\n");
                printf("args[j-1] = %s\n",args[j-1]);
                printf("args[j+1] = %s\n",args[j+1]);
                /* if (!strcmp(args[j+2],"\0")) {
                    printf("testu\n");
                    int in = open(args[j-1],O_RDONLY);
                    dup2(in,STDIN_FILENO);
                    close(in);
                    int out = open(args[j+1],O_WRONLY|O_CREAT,0666);
                    dup2(out,STDOUT_FILENO);
                    close(out);
                //}
            }
        } */

        // Unused version (tbc)
        /* if (strchr(line,'<') != NULL || strchr(line,'>') != NULL) {
            // Input
            if (strchr(line,'<') != NULL) {
                while (!strcmp(args[j],"<")) {
                    j++;
                }
                int fd1[2];
                int fd2[2];
                if (!strcmp(args[j+2],"\0")) {
                    int length = 0;
                    while (k < j) {
                        length = length + strlen(args[k]);
                        k++;
                    }
                    k = 0;
                    char *string = malloc((length+1)*sizeof(char));
                    char *read_string = malloc((length+1)*sizeof(char));
                    while (k < j) {
                        if (k == 0) {
                            strcpy(string,args[k]);
                        } else {
                            strcat(string,args[k]);
                        }
                        k++;
                    }
                    if (pipe(fd1) < 0) {
                        printf("error\n");
                        prompt();
                        continue;
                    }

                    if (pipe(fd2) < 0) {
                        printf("error\n");
                        prompt();
                        continue;
                    }

                    pid = fork();
                    if (pid < 0) {
                        printf("error\n");
                        prompt();
                        continue;
                    }
                    if (pid > 0) {
                        close(fd1[0]);
                        write(fd1[1],string,strlen(string)+1);
                        close(fd1[1]);
                        wait(NULL);
                        close(fd2[1]);
                        read(fd2[0],read_string,strlen(read_string+1));
                        close(fd2[0]);
                    } else {
                        close(fd1[1]);
                        read(fd1[0],read_string,strlen(read_string+1));
                        k = 0;
                        close(fd1[0]);
                        close(fd2[0]);
                        write(fd2[1],read_string,strlen(read_string+1));
                        close(fd2[1]);
                    }

                    addToCommandHistory(line,cmdNo);
                    cmdNo++;
                    prompt();
                    continue;
                }
            } // Output
            else if ()
        } */

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
            wait(&stat);
            freeTokens(args);
        } else {
            execute(args,path,envp);
        }

        if (flag == 1) {
            printf("--------------------\n");
            printf("Return 0\n");
            addToCommandHistory(line,cmdNo);
            cmdNo++;
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
char **fileNameExpand(char **tokens)
{
   // TODO
   return NULL;
}

// findExecutable: look for executable in PATH
char *findExecutable(char *cmd, char **path)
{
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
int isExecutable(char *cmd)
{
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
char **tokenise(char *str, char *sep)
{
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
void trim(char *str)
{
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
int strContains(char *str, char *chars)
{
   for (char *s = str; *s != '\0'; s++) {
      for (char *c = chars; *c != '\0'; c++) {
         if (*s == *c) return 1;
      }
   }
   return 0;
}

// prompt: print a shell prompt
// done as a function to allow switching to $PS1
void prompt(void)
{
   printf("mymysh$ ");
}


// execute: run a program, given command-line args, path and envp
int execute(char **args, char **path, char **envp)
{
    // TODO: implement the find-the-executable and execve() it code
    //    args = tokenise the command line
    int flag = 1;
    // Used to denote if success or failed.
    char* command = NULL;
//    if (args[0] starts with '/' or '.') {
    if (args[0][0] == '/' || args[0][0] == '.') {
//    check if the file called args[0] is executable
        if (isExecutable(args[0]))
            command = strdup(args[0]);
//    if so, use args[0] as the command
    } else {
        int i = 0;
        for (i = 0; path[i] != NULL; i++) {
//       see if an executable file called "D/args[0]" exists
            char* filePath = malloc(strlen(path[i]) + strlen(args[0]) + 2);
            strcpy(filePath, path[i]);
            strcat(filePath, "/");
            strcat(filePath, args[0]);
//       if it does, use that file name as the command
            if (isExecutable(filePath)) {
                command = strdup(filePath);
                free(filePath);
                break;
            }
            free(filePath);
        }
    }
    if (command == NULL) {
        printf("%s: Command not found\n",args[0]);
        flag = 0;
        return flag;
    } else {
//    print the full name of the command being executed
        printf("Executing command: %s\n",command);
        printf("--------------------\n");
//    use execve() to attempt to run the command with args and envp
        if (execve(command,args,envp) == -1) {
            printf("Exec failed\n");
//    if doesn't run, perror("Exec failed")
        }
    }
// exit the child process
    exit(EXIT_SUCCESS);
    return flag;
}

// Prints current working directory.
void printDir() {
    char directory[MAXLINE];
    if (getcwd(directory, sizeof(directory)) != NULL)
        printf("%s\n",directory);
    else
       perror("getcwd() error");
}
