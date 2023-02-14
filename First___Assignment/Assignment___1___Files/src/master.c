#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/inotify.h>
#include <string.h>
#include <fcntl.h> 

/**********************************************************/
#define TIMER 60 // define constant TIMER as 60

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )

int timer = TIMER; // declare and initialize variable timer to TIMER value

/**********************************************************/
pid_t pid_cmd; // declare variable pid_cmd of type pid_t
pid_t pid_insp; // declare variable pid_insp of type pid_t
pid_t pid_motorx; // declare variable pid_motorx of type pid_t
pid_t pid_motorz; // declare variable pid_motorz of type pid_t
pid_t pid_world; // declare variable pid_world of type pid_t
pid_t pid_master; // declare variable pid_master of type pid_t

/**********************************************************/
int fd, wd; // declare variables fd and wd of type int

/**********************************************************/
// Signal handler function to terminate child processes on receiving a signal
void terminate(int signal)
{
  kill(pid_world,signal); // terminate process with pid pid_world
  printf("world killed");
  kill(pid_motorx,signal); // terminate process with pid pid_motorx
  printf("motorx killed");
  kill(pid_motorz,signal); // terminate process with pid pid_motorz
  printf("motorz killed");
  kill(pid_insp,signal); // terminate process with pid pid_insp
  printf("inspection killed");
  kill(pid_cmd,signal); // terminate process with pid pid_cmd
  printf("command killed");
  kill(pid_master,signal); // terminate process with pid pid_master
  printf("master killed");
}

// Function to spawn a child process with given program and arguments
int spawn(const char * program, char * arg_list[]) {

  pid_t child_pid = fork(); // create a new process by forking the parent process

  if(child_pid < 0) { // check if fork failed
    perror("Error while forking..."); // print error message
    return 1; // return with failure status
  }

  else if(child_pid != 0) { // parent process
    return child_pid; // return child process ID
  }

  else { // child process
    if(execvp (program, arg_list) == 0); // execute program with arguments, and check for failure
    perror("Exec failed"); // print error message
    return 1; // return with failure status
  }
}

int main() {

  // Arguments for child processes
  char * arg_list_command[] = { "/usr/bin/konsole", "-e", "./bin/command", NULL };
  char * arg_list_inspection[] = { "/usr/bin/konsole", "-e", "./bin/inspection", NULL };
  char * arg_list_motorx[] = { "./bin/motorx", NULL };
  char * arg_list_motorz[] = { "./bin/motorz", NULL };
  char * arg_list_world[] = { "./bin/world", NULL };

  // Spawn child processes and get their process IDs
  pid_cmd = spawn("/usr/bin/konsole", arg_list_command);
  pid_insp = spawn("/usr/bin/konsole", arg_list_inspection);
  pid_motorx = spawn("./bin/motorx", arg_list_motorx);
  pid_motorz = spawn("./bin/motorz", arg_list_motorz);
  pid_world = spawn("./bin/world", arg_list_world);
  

  /**********************************************************/
// Initialize inotify
// Create an inotify instance and add a watch for the "log" directory with events to monitor.
fd = inotify_init();
if (fd < 0) {
  perror("inotify_init"); // Handle error
}
wd = inotify_add_watch(fd, "./log", IN_MODIFY | IN_CREATE);
if (wd < 0) {
  perror("inotify_add_watch"); // Handle error
}
// Set the file descriptor to non-blocking mode.
if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0) {
  perror("fcntl"); // Handle error
  fflush(stdout);
}
/**********************************************************/

// Continuously loop to monitor for file changes
while (1) {
  char buffer[EVENT_BUF_LEN];
  int length, i = 0;

  // Read buffer
  // Read the events that have occurred since the last read from the inotify instance.
  length = read(fd, buffer, EVENT_BUF_LEN);

  if (length > 0) {
    // If there are events, reset the timer.
    timer = TIMER;
  }

  if (timer == 0) {
    // If the timer has reached zero, terminate the program.
    timer = TIMER;
    terminate(SIGTERM);
  }

  sleep(1);
  timer--;
}
/**********************************************************/
int status; // Declare a variable to hold the process status.
terminate(SIGTERM); // Send the SIGTERM signal to terminate the processes.

// Wait for the child processes to terminate.
waitpid(pid_cmd, &status, 0);
waitpid(pid_insp, &status, 0);
waitpid(pid_motorx, &status, 0);
waitpid(pid_motorz, &status, 0);
waitpid(pid_world, &status, 0);

// Print the status of the main process.
printf ("Main program exiting with status %d\n", status);

// Return 0 to indicate successful completion.
return 0;
}

