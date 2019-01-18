// COMP1521 18s2 mysh ... command history
// Implements an abstract data object
// Completed by Jeremy Lim (z5209627), September/October 2018
// Version 4 (30/09)

// if it reads from a file, & is needed
// if there's no file, & is NOT needed

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

int initCommandHistory() {
    int i = 0;
    CommandHistory.nEntries = 0;

    // Allocates memory to commandLine and initialises all seqNumbers to -1.
    while (i < MAXHIST) {
        CommandHistory.commands[i].commandLine = malloc((MAXSTR+1)*sizeof(char));
        CommandHistory.commands[i].seqNumber = -1;
        i++;
    }
    FILE *history;
    char string[MAXSTR];
    char number[MAXSTR];

    char *file = malloc(MAXSTR*sizeof(char));
    strcpy(file,getenv("HOME"));
    strcat(file,"/.mymysh_history");
    history = fopen(file,"r");

    // If the file doesn't exist, seqNumber must start at 1.
    if (history == NULL) {
        return 1;
    } else {
        i = 0;
        // Reads the file for the seqNumber and commandLine and initialises the array.
        while (fgets(string,MAXSTR,history) != NULL && i < MAXHIST) {
            sscanf(string," %3d %s\n",&CommandHistory.commands[i].seqNumber,CommandHistory.commands[i].commandLine);
            i++;
        }
        CommandHistory.nEntries = i;
        fclose(history);
    }

    return CommandHistory.commands[i-1].seqNumber + 1;
}

// addToCommandHistory()
// - add a command line to the history list
// - overwrite oldest entry if buffer is full

void addToCommandHistory(char *cmdLine, int seqNo) {
    int i = 0;
    int index = CommandHistory.nEntries;

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
            free(CommandHistory.commands[i].commandLine);
            CommandHistory.commands[i].commandLine = strdup(CommandHistory.commands[i+1].commandLine);
            CommandHistory.commands[i].seqNumber = CommandHistory.commands[i+1].seqNumber;
            i++;
        }
        free(CommandHistory.commands[MAXHIST-1].commandLine);
        CommandHistory.commands[MAXHIST-1].commandLine = strdup(cmdLine);
        CommandHistory.commands[MAXHIST-1].seqNumber = seqNo;
    }
}

// showCommandHistory()
// - display the list of

void showCommandHistory() {
    int i = 0;

    // Iterates through the array of command history to show up to the last 20 commands.
    while (i < CommandHistory.nEntries) {
        printf(" %3d  %s\n",CommandHistory.commands[i].seqNumber,CommandHistory.commands[i].commandLine);
        i++;
    }
}

// getCommandFromHistory()
// - get the command line for specified command
// - returns NULL if no command with this number

char *getCommandFromHistory(int cmdNo) {
    // Finds the lower range of the seqNumber.
    int lower = CommandHistory.commands[0].seqNumber;
    if (cmdNo < lower || cmdNo > lower + 20) {
        return NULL;
    }
    // Calculates array index offset to find the sequence number.
    int offset = cmdNo - lower;
    if (CommandHistory.commands[offset].seqNumber == cmdNo) {
        return CommandHistory.commands[offset].commandLine;
    } else {
        return NULL;
    }
}

// saveCommandHistory()
// - write history to $HOME/.mymysh_history

void saveCommandHistory() {
    // TODO
    FILE *history;
    char *file = malloc(MAXSTR*sizeof(char));
    strcpy(file,getenv("HOME"));
    strcat(file,"/.mymysh_history");
    int i = 0;
    history = fopen(file,"w");
    while (i < CommandHistory.nEntries) {
        i++;
    }
    i = 0;
    while (i < CommandHistory.nEntries) {
        fprintf(history," %3d %s\n",CommandHistory.commands[i].seqNumber,CommandHistory.commands[i].commandLine);
        i++;
    }
    fclose(history);
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

void cleanCommandHistory() {
    // Frees all allocated memory.
    int i = 0;
    while (i < CommandHistory.nEntries) {
        free(CommandHistory.commands[i].commandLine);
        i++;
    }
}
