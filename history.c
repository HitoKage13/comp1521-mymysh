// COMP1521 18s2 mysh ... command history
// Implements an abstract data object
// Completed by Jeremy Lim (z5209627), September/October 2018
// Version 1 (21/09)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "history.h"

// This is defined in string.h
// BUT ONLY if you use -std=gnu99
//extern char *strdup(const char *s);

// Command History
// array of command lines
// each is associated with a sequence number

#define MAXHIST 20
#define MAXSTR  200

#define HISTFILE ".mymysh_history"

typedef struct _history_entry {
   int   seqNumber;
   char *commandLine;
} HistoryEntry;

typedef struct _history_list {
   int nEntries;
   HistoryEntry commands[MAXHIST];
} HistoryList;

HistoryList CommandHistory;

// initCommandHistory()
// - initialise the data structure
// - read from .history if it exists

int initCommandHistory()
{
    // TODO
    int i = 0;
    HistoryList CommandHistory = malloc(sizeof(HistoryList));
    while (i < MAXHIST) {
        CommandHistory->commands[i].commandLine = malloc((MAXSTR+1)*sizeof(char));
        i++;
    }
    FILE *history;
    char *file;
    strcpy(file,"$HOME/.mymysh_history");
    if (getenv(file) != NULL) {
        char string[MAXSTR];
        char seqNumber[MAXLINE];
        history = fopen(file,"a");
        while (fgets(string,MAXSTR,history) != NULL) {
            seqNumber = strsep(string," ");
            printf("seqNumber = %s\n",seqNumber);
            printf("string = %s\n",string);
            CommandHistory->commands[i].seqNo = seqNumber;
            strcpy(CommandHistory->commands[i].commandLine,string);
            i++;
        }
    }
}

// addToCommandHistory()
// - add a command line to the history list
// - overwrite oldest entry if buffer is full

void addToCommandHistory(char *cmdLine, int seqNo)
{
    // TODO
    int oldestSeq = seqNo - 19;
    int i = 0;

    // If CommandHistory has less than 20 entries.
    if (CommandHistory->nEntries < MAXHIST) {
        strcpy(CommandHistory->commands[nEntries-1].commandLine,cmdLine);
        CommandHistory->commands[nEntries-1].seqNumber = seqNo;
        CommandHistory->nEntries++;
    } // If CommandHistory has 20 entries (needs to overwrite).
    else if (CommandHistory->nEntries == MAXHIST) {
        // Shifts all entries up one.
        while (i < MAXHIST - 1) {
            CommandHistory->commands[i].seqNo = CommandHistory->commands[i+1].seqNo;
            strcpy(CommandHistory->commands[i].commandLine,CommandHistory->commands[i+1].commandLine);
            i++;
        }
        strcpy(CommandHistory->commands[nEntries-1].commandLine,cmdLine);
        CommandHistory->commands[nEntries-1].seqNumber = seqNo;
    }
}

// showCommandHistory()
// - display the list of

void showCommandHistory(FILE *outf)
{
    // TODO
    int i = 0;
    FILE *output = fopen(outf,"r");
    char string[MAXSTR];
    while (fgets(string, MAXSTR, output) != NULL && i < MAXHIST) {
        printf("%s\n",string);
        i++;
    }
    fclose(output);
}

// getCommandFromHistory()
// - get the command line for specified command
// - returns NULL if no command with this number

char *getCommandFromHistory(int cmdNo)
{
    // TODO
    if (CommandHistory->commands[cmdNo-1] != NULL) {
        return CommandHistory->commands[cmdNo-1].commandLine;
    } else {
        return NULL;
    }
}

// saveCommandHistory()
// - write history to $HOME/.mymysh_history

void saveCommandHistory()
{
    // TODO
    FILE *history;
    char *file;
    strcpy(file,"$HOME/.mymysh_history");
    if (getenv(file) != NULL) {
        history = fopen(file,"a");
    }
}

// cleanCommandHistory
// - release all data allocated to command history

void cleanCommandHistory()
{
    // TODO
    free(CommandHistory);
}
