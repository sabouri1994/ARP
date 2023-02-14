#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

// Function to spawn a new process with given program and arguments
int spawn(const char * program, char * arg_list[]) {

  // Fork the current process to create a new child process
  pid_t child_pid = fork();

  // Check for any errors while forking
  if(child_pid < 0) {
    perror("Error while forking...");
    return 1;
  }

  // If the child process is successfully created, return its PID to the parent process
  else if(child_pid != 0) {
    return child_pid;
  }

  // If the current process is the child process, execute the new program
  else {
    if(execvp (program, arg_list) == 0); // Execute the new program with the given arguments
    perror("Exec failed"); // Print an error message if the execution failed
    return 1;
  }
}

int main() {

  // Define the arguments for the two programs to be executed
  char * arg_list_A[] = { "/usr/bin/konsole", "-e", "./bin/processA", NULL };
  char * arg_list_B[] = { "/usr/bin/konsole", "-e", "./bin/processB", NULL };

  // Spawn two new child processes to execute the two programs
  pid_t pid_procA = spawn("/usr/bin/konsole", arg_list_A);
  pid_t pid_procB = spawn("/usr/bin/konsole", arg_list_B);

  // Wait for both child processes to terminate
  int status;
  waitpid(pid_procA, &status, 0);
  waitpid(pid_procB, &status, 0);

  // Print the status of the main process
  printf ("Main program exiting with status %d\n", status);

  // Return 0 to indicate successful completion
  return 0;
}

