/*
 * Carlos's Shell
 * Parts of the project were completed in collaboration with:
 *    Shereen Haji
 *    Geethica
 *
 * Similar google resources may or may not have been used between us.
 * Resources:
 *    https://www.tutorialspoint.com/cprogramming/c_array_of_pointers.htm
 *    https://www.educative.io/answers/splitting-a-string-using-strtok-in-c
 */

#include "cwd.h" // for using cwd()
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define HIST_SIZE 50
#define CMD_SIZE 512
#define MAX_ARGS 512

/*
 * Global Variables
 */

extern char **environ;
char *hist[HIST_SIZE]; // History array
int end = 0;
int histFull = 1;
/*
 * Function Declarations
 */
void cmdHandler(char cmd[], char **path);
void runRelativeProgram(char *args[]);
void put(char item[]);

/*
 * put= Puts command into history array
 */
void put(char item[]) {
  char histCpy[CMD_SIZE];
  strncpy(histCpy, item, CMD_SIZE);
  // char *histCmd = strtok(histCpy, "");
  if (end == HIST_SIZE) {
    end = 0;
    histFull = 0;
  }
  hist[end] = strdup(histCpy);
  end += 1;
}

/*
 * runRelativeProgram= Runs a program given:
 *  prgrm[]= Path of the program to run
 *  args[]= Arguments for program
 * Output= None, runs a program
 */
void runRelativeProgram(char *args[]) {
  // Executes program
  pid_t pid = fork();
  // Handle error
  if (pid == -1) {
    perror("fork");
  } else if (pid == 0) {
    // Child process task
    if (execve(args[0], args, environ) == -1) {
      perror("execve");
      exit(-1);
    }
  }
  // Makes parent wait for child
  else {
    wait(NULL);
  }
}

/*
 *cmdHandler= Handles commands given by user
 *  Input:
 *    cmd[]= command string given by user
 *  Output= None, executes commands or calls functions to execute commands
 */
void cmdHandler(char cmd[], char **path) {

  put(cmd);

  // Stuff for command manipulation & processing
  char *cmdPointers[MAX_ARGS]; // Create array of pointers for splitting command
  memset(cmdPointers, 0, CMD_SIZE * sizeof(char *)); // Set all to null
  char fstTwo[3]; // Stores first two chars of cmd
  char fst[2];    // Stores first char
  fst[0] = cmd[0];
  fst[1] = '\0';

  // Split command
  char cmdCpy[CMD_SIZE];
  strncpy(cmdCpy, cmd, CMD_SIZE);
  char *cmdToken = strtok(cmdCpy, " ");
  int cmdSize = 0;

  while (cmdToken != NULL) {
    cmdPointers[cmdSize] = cmdToken;
    cmdSize += 1;
    cmdToken = strtok(NULL, " ");
  }

  // Preps cmd for running programs
  if (strlen(cmd) >= 2 && cmdSize >= 1) {
    // Populates fstTwo with first two chars of cmd
    for (int i = 0; i < 2; i++) {
      fstTwo[i] = cmd[i];
    }
    fstTwo[2] = '\0';
  }

  // Handle no command
  if (strncmp(cmdPointers[0], "\n", CMD_SIZE) == 0) {
    printf("no command specified\n");
  }

  // Handle pwd
  else if (strncmp(cmdPointers[0], "pwd", CMD_SIZE) == 0) {
    char wd[MAX_ARGS];
    getcwd(wd, MAX_ARGS);
    printf("%s\n", wd);
  }

  // Handle cd
  else if (strncmp(cmdPointers[0], "cd", CMD_SIZE) == 0) {

    int err; // Initialize error number

    // Handle shortcuts
    if (strncmp(cmdPointers[1], "~", CMD_SIZE) == 0 ||
        strncmp(cmdPointers[1], "-", CMD_SIZE) == 0) {
      err = chdir(getenv("HOME"));
    } else {
      err = chdir(cmdPointers[1]);
    }
    // Throw up an error message
    if (err != 0) {
      perror("error");
    }
  }

  // Handle executing programs
  else if (strncmp(fstTwo, "./", CMD_SIZE) == 0) {
    runRelativeProgram(cmdPointers);
  }

  // Handles 'history' command
  else if (strncmp(cmdPointers[0], "history", CMD_SIZE) == 0) {
    printf("History:\n");
    // Prints history if full
    if (histFull == 0) {
      for (int i = 0; i < HIST_SIZE; i++) {
        printf("%s\n", hist[i]);
      }
    }
    // else
    else {
      for (int j = 0; j < end; j++) {
        printf("%s\n", hist[j]);
      }
    }
  }

  // Handles '!'
  else if (strncmp(fst, "!", CMD_SIZE) == 0) {
    int histIndex;
    if (strlen(cmdPointers[0]) <= 2) {
      char indx[2];
      indx[0] = cmd[1];
      indx[1] = '\0';
      histIndex = atoi(indx);

    } else {
      char indx[3];
      indx[0] = cmd[1];
      indx[1] = cmd[2];
      indx[2] = '\0';
      histIndex = atoi(indx);
    }
    cmdHandler(hist[histIndex], path);
  }
  // Execute non-realtive programs
  else {
    int index = 0;
    char prgrmPath[MAX_ARGS];

    // Scans directories of path for program
    while (path[index] != 0) {
      // char prgrmPath[MAX_ARGS];
      strncpy(prgrmPath, path[index], MAX_ARGS);
      strcat(prgrmPath, "/");
      strcat(prgrmPath, cmdPointers[0]);
      if (access(prgrmPath, X_OK) == 0) {
        break;
      }
      index++;
    }
    cmdPointers[0] = prgrmPath;      // First arg replaces with program path
    runRelativeProgram(cmdPointers); // Runs program
  }
}

/*
 *  Main Function
 */
int main(int argc, char **argv) {
  // Maybe use dynamic memory allocation for this...
  char pathCpy[MAX_ARGS];       // Create empty string as char array for 'path'
                                // manipulation
  char *pathPointers[MAX_ARGS]; // Create empty array of char pointers
  memset(pathPointers, 0, MAX_ARGS * sizeof(char *));
  strncpy(pathCpy, getenv("PATH"), MAX_ARGS); // Copy path into empty string
  int size = 0;

  // Split string on the colon
  char *token =
      strtok(pathCpy, ":"); // Initializes token to first split of path

  while (token != NULL) {
    pathPointers[size] = token;
    size += 1;
    token = strtok(NULL, ":");
  }

  // Print the shiz
  printf("Welcome to MyShell!\nPath: %s\n", pathPointers[0]);

  for (int i = 1; i < size; i++) {
    printf("      %s\n", pathPointers[i]);
  }

  // Get first shell command
  char command[CMD_SIZE]; // array for command
  printf("$ ");
  fgets(command, CMD_SIZE, stdin);

  // Implement input loop
  while (strncmp(command, "exit\n", CMD_SIZE) != 0) {
    // Strip command only if user put in more than just an empty command
    if (strncmp(command, "\n", CMD_SIZE) != 0) {
      command[strlen(command) - 1] = '\0';
    }

    // Call command handler to execute commands
    cmdHandler(command, pathPointers);

    // Get new command
    printf("$ ");
    fgets(command, CMD_SIZE, stdin);
  }

  return 0;
}
