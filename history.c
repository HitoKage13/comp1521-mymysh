// COMP1521 18s2 mysh ... command history
// Implements an abstract data object
// Completed by Jeremy Lim (z5209627), September/October 2018
// Version 2 (28/09)

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

typedef struct HistoryEntry {
   int   seqNumber;
   char *commandLine;
} HistoryEntry;

typedef struct HistoryList {
   int nEntries;
   HistoryEntry commands[MAXHIST];
} HistoryList;

HistoryList CommandHistory;

// initCommandHistory()
// - initialise the data structure
// - read from .history if it exists

int initCommandHistory()
{
    // Works with arrays, doesn't work with files yet
    // TODO
    int i = 0;
    CommandHistory.nEntries = 0;
    while (i < MAXHIST) {
        // CommandHistory.commands[i].commandLine = malloc((MAXSTR+1)*sizeof(char));
        CommandHistory.commands[i].seqNumber = -1;
        i++;
    }
    FILE *history;
    char string[MAXSTR];
    char number[MAXSTR];
    char *file = strdup("$HOME/.mymysh_history");
    history = fopen(file,"r");
    if (history == NULL) {
        return 1;
    } else {
        i = 0;
        while (fgets(string,MAXSTR,history) != NULL) {
            sscanf(number,string);
            int j = atoi(number);
            printf("j = %d\n",j);
            CommandHistory.commands[i].seqNumber = j;
            CommandHistory.commands[i].commandLine = strdup(string);
            i++;
        }
    }

    fclose(history);

    return CommandHistory.commands[i-1].seqNumber;
}

// addToCommandHistory()
// - add a command line to the history list
// - overwrite oldest entry if buffer is full

void addToCommandHistory(char *cmdLine, int seqNo)
{
    // Works now lol
    // TODO
    int oldestSeq = seqNo - 19;
    int i = 0;
    int index = CommandHistory.nEntries;
    printf("index = %d\n",index);

    // If CommandHistory has less than 20 entries.
    if (CommandHistory.nEntries < MAXHIST) {
        CommandHistory.commands[index].commandLine = strdup(cmdLine);
        CommandHistory.commands[index].seqNumber = seqNo;
        index++;
        CommandHistory.nEntries = index;
    } // If CommandHistory has 20 entries (needs to overwrite).
    else if (CommandHistory.nEntries == MAXHIST) {
        // Shifts all entries up one.
        while (i < MAXHIST - 1) {
            CommandHistory.commands[i].seqNumber = CommandHistory.commands[i+1].seqNumber;
            CommandHistory.commands[i].commandLine = strdup(CommandHistory.commands[i+1].commandLine);
            i++;
        }
        CommandHistory.commands[index-1].commandLine = strdup(cmdLine);
        CommandHistory.commands[index-1].seqNumber = seqNo;
    }
}

// showCommandHistory()
// - display the list of

void showCommandHistory(FILE *outf)
{
    // TODO
    int i = 0;
    char *file = strdup(HISTFILE);
    outf = fopen(file,"r");
    char string[MAXSTR];
    while (fgets(string, MAXSTR, outf) != NULL && i < MAXHIST) {
        printf("%s\n",string);
        i++;
    }
    fclose(outf);
}

// getCommandFromHistory()
// - get the command line for specified command
// - returns NULL if no command with this number

char *getCommandFromHistory(int cmdNo)
{
    // TODO
    int lower = CommandHistory.commands[0].seqNumber;
    if (cmdNo < lower || cmdNo > lower + 20) {
        return NULL;
    }
    int offset = cmdNo - lower;
    if (CommandHistory.commands[offset-1].seqNumber == cmdNo) {
        return CommandHistory.commands[offset-1].commandLine;
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
    char *file = strdup("$HOME/.mymysh_history");
    int i = 0;
    printf("bugde\n");
    history = fopen(file,"a");
    printf("bugdee\n");
    while (i < CommandHistory.nEntries) {
        fprintf(history," %d ",CommandHistory.commands[i].seqNumber);
        fprintf(history,"%s\n",CommandHistory.commands[i].commandLine);
        i++;
    }
    printf("bugdeee\n");
    // fclose(history);
    printf("buhdee\n");
    /* if (getenv(file) != NULL) {
        history = fopen(file,"w");
        while (i < nEntries) {
            fprintf("%d ",CommandHistory.commands[i].seqNumber);
            fprintf("%s\n",CommandHistory.commands[i].commandLine);
            i++;
        }
    } */
}

// cleanCommandHistory
// - release all data allocated to command history

void cleanCommandHistory()
{
    // TODO
    int i = 0;
    while (i < CommandHistory.nEntries) {
        free(CommandHistory.commands[i].commandLine);
        i++;
    }
}

void debug() {
    int i = 0;
    int high = CommandHistory.nEntries;
    printf("nEntries = %d\n",high);
    while (i < high) {
        printf("seqNumber: %d\n",CommandHistory.commands[i].seqNumber);
        printf("commandLine: %s\n\n",CommandHistory.commands[i].commandLine);
        i++;
    }
}
