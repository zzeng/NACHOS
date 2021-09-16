#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_TOKEN_LENGTH 50
#define MAX_TOKEN_COUNT 100
#define MAX_LINE_LENGTH 512

// Simple implementation for Shell command
// Assume all arguments are seperated by space
// Erros are ignored when executing fgets(), fork(), and waitpid().

/**
 * Sample session
 *  shell: echo hello
 *   hello
 *   shell: ls
 *   Makefile  simple  simple_shell.c
 *   shell: exit
**/

// Run commands with pipe
// This function only handles piping and forking
void runcommandwithpipe(char* command1, char* command2, char** args1, char** args2, int pfd[]);
//The general run command function
// This function handles input and output redirection
// If pipes are needed, this function will redirect to runcommandwithpipe
void runcommand(char* command, char** args);

void runcommandwithpipe(char* command1, char* command2, char** args1, char** args2, int pfd[]) {
  // Forks the parent, into two duplicate processes with different pids (child will have a pid of 0)
  pid_t pid = fork();
  
  //If the pid is anything but 0, it must be a parent and will run this code
  if(pid) {
    // Waits for the child's completion
    waitpid(pid, NULL, 0);
    //sets the file descriptor 0 (standard input) to pfd[0] (the output of the proper file descriptor)
    dup2(pfd[0], 0);
    // Closes standard output and standard input for the process (still saved in 0)
    close(pfd[0]);
    close(pfd[1]);
    // Continues running the rest of the commands with appropriately modified input/outputs
    runcommand(command2, args2);
  }
  //If the pid is 0, it's a child and will run this block 
  else {
    // sets the file descriptor 1 (standard input) to pfd[1] (input of the proper file descriptor)
    dup2(pfd[1], 1);
    // Closes standard output and standard input for the process (still saved in 1)
    close(pfd[0]);
    close(pfd[1]);
    // Runs the child command
    runcommand(command1, args1);
    // Closes the child's standard input
    exit(0);
  }
}

void runcommand(char* command, char** args) {
  // Flag - 0 no pipe
  //      - 1 pipe
  int flag = 0;
  // First - 0 
  int first = 0;

  // Set variables to default values
  // These variables below are used in the case of a pipe
  // Command 1 and args1 are used to handle the children 
  // while command2 and args2 handle the parents
  char* command1 = NULL;
  char* command2 = NULL;
  char* args1[MAX_TOKEN_COUNT];
  char* args2[MAX_TOKEN_COUNT];
  bzero(args1, MAX_TOKEN_COUNT);
  bzero(args2, MAX_TOKEN_COUNT);


  for(int i = 0; args[i] != 0; i++) {
    // If pipe is found...
    if(strcmp(args[i], "|") == 0) {
      // And if the first flag isn't set
      if(first == 0) {
	// Take the characters from the beginning of the args array
	// up to the point j where there is a "|" and put that in args1
        for(int j = 0; j < i; j++) {
          args1[j] = args[j];
        }
	// Put the remaining arguments in args2
        for(int k = i + 1; args[k] != 0; k++) {
          args2[k - i - 1] = args[k];
        }
	// The command variables are then set to the first command in 
	// each of the args arrays
        command1 = args1[0];
        command2 = args2[0];
	// And the flag and first flags are set
        flag = 1;
        first = 1;
      }
    }
  }
  // If the flag flag has been set, this means there is a pipe
  if(flag) {
    // If pipe exists we run pipe on a file descriptor to get file descriptors for input and output
    // Fork so the child runs the helper function for commands with pipes and the parent will wait for the child to finish
    pid_t pipeid = fork();

    int pipefd[2];
    // Stores the file descriptors for the pipe
    // This will allow us to change/access them later
    pipe(pipefd);
    // If pipeid != 0, it's a parent and it will wait for the child to terminate
    if(pipeid) { //parent
      waitpid(pipeid, NULL, 0);
    }
    // If pipeid == 0, it's a child and will use our runcommand function to process the line of commands, then exit
    else { // child
      runcommandwithpipe(command1, command2, args1, args2, pipefd);
      exit(0);
    }
  }
  // Otherwise, if there is no pipe...
  else {
    // Fork the process
    pid_t pid = fork();
    if(pid) { // parent
      // Wait for the child to complete
      waitpid(pid, NULL, 0);
    } 
    else { // child
      int newfd;
      // For every argument in the argument list
      for(int i = 0; args[i] != 0; i++) {
	// If input redirection is found
        if(strcmp(args[i], "<") == 0) {
	  // Set the new file descriptor to open the file to direct input from
	  // if it fails then exit with an error
	  if((newfd = open(args[i+1], O_CREAT|O_RDONLY, 0644)) < 0) {
	    perror(args[i+1]);
	    exit(1);
	  }
	  // Close standard input for the process
	  close(0);
	  // Set the file to be standard input for the process
	  dup2(newfd, 0);
	  // and then close the file
	  close(newfd);
	  // Shift the arguments array forward by 2
	  for(int j = i; args[j] != 0; j++)
	    args[j] = args[j+2];
	  // And set the index position back by 1
	  i--;
        }
	// if output redirection is found
        else if(strcmp(args[i], ">") == 0) {
	  // Create/open a file to direct input to
	  // if it fails exit with an error
	  if((newfd = open(args[i+1], O_CREAT|O_WRONLY, 0644)) < 0) {
	    perror(args[i+1]);
	    exit(1);
	  }
	  // Close standard ouput
	  close(1);
	  // Set the file to be standard output for the process
	  dup2(newfd, 1);
	  // Close the file
	  close(newfd);
	  // Shift the arguments array forward by 2
	  for(int j = i; args[j] != 0; j++)
	    args[j] = args[j+2];
	  // And set the index position back by 1
	  i--;
        }
      }
      // Execute the command after the input/output redirection has been done
      execvp(command, args);
    }
  }
}

// Exit signal handler
// Takes in signal, increments count
// When CRTL+Z is pressed twice, the shell will terminate
void handler(int sig) {
  static int count = 0;
  printf("Enter CTRL-Z again to exit\n");
  count++;
  if(count == 1)
    signal(SIGTSTP, SIG_DFL);
}

int main(){
  // Allows the handler function to take care of the exit signal
  signal(SIGTSTP, handler);

  // Everything below was implemented prior

  char line[MAX_LINE_LENGTH];
  //printf("shell: ");
  while(fgets(line, MAX_LINE_LENGTH, stdin)) {
    // Build the command and arguments, using execv conventions.
    line[strlen(line)-1] = '\0'; // get rid of the new line
    char* command = NULL;
    char* arguments[MAX_TOKEN_COUNT];
    int argument_count = 0;

    char* token = strtok(line, " ");
    while(token) {
      if(!command) command = token;
      arguments[argument_count] = token;
      argument_count++;
      token = strtok(NULL, " ");
    }
    arguments[argument_count] = NULL;
    if(argument_count>0){
      if (strcmp(arguments[0], "exit") == 0)
	       exit(0);
      runcommand(command, arguments);
    }
    //printf("shell: ");
  }
  return 0;
}
