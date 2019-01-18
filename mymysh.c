// mysh.c ... a small shell
// Started by John Shepherd, September 2018
// Completed by Jeremy Lim (z5209627), September/October 2018
// Version 2 (24/09)

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
   // char *file;
   // strcpy(file,"$HOME/.mymysh_history");
   // FILE *fp = fopen(file,"a");
   prompt();
   while (fgets(line, MAXLINE, stdin) != NULL) {
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

        // Built-in commands
        if (!strcmp(line,"exit")) {
            break;
        } else if (!strcmp(line,"h") || !strcmp(line,"history")) {
            // showCommandHistory(fp);
            addToCommandHistory(line,cmdNo);
            prompt();
            continue;
        } else if (!strcmp(line,"pwd")) {
            printDir();
            addToCommandHistory(line,cmdNo);
            prompt();
            continue;
        } else if (!strcmp(line,"")) {
            prompt();
            continue;
        } else if (!strcmp(line,"debug")) {
            debug();
        }

        // TODO
        // Code to implement mainloop goes here
        // Uses
        // - addToCommandHistory()
        // - showCommandHistory()
        // - and many other functions
        // TODO

        // Read and execute commands
        char **args = tokenise(line," ");
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
        printf("--------------------\n");
        printf("Return 0\n");
        addToCommandHistory(line,cmdNo);
        cmdNo++;
        prompt();
    }
    // saveCommandHistory();
    // cleanCommandHistory();
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
void execute(char **args, char **path, char **envp)
{
   // TODO: implement the find-the-executable and execve() it code
   //    args = tokenise the command line
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
    } else {
//    print the full name of the command being executed
        printf("Executing command: %s\n",command);
        printf("--------------------\n");
//    use execve() to attempt to run the command with args and envp
        if (execve(command,args,envp) == -1)
            printf("Exec failed\n");
//    if doesn't run, perror("Exec failed")
    }
// exit the child process
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
