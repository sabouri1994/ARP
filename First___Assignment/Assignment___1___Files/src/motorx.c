#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>

#define SIZE 80

#define X_MIN 0
#define X_MAX 39

float SPEED_MINIMUM = -2;
float SPEED_MAXIMUM = 2;

float velocity_x = 0;
float freq = 1;

int update = 0;

// File descriptors
int fd_cmd, fd_world;

// End-effector coordinates
float Endeffector_x = 0;

char command_c[SIZE];
char command_x[SIZE];
char * mx_world;

FILE *log_file;

void rst(int sig) {
    // Signal from the inspection console (Reset)
    if (sig == SIGUSR2) {
            velocity_x=-1;
    }
}

void exit_handler(int sig) {
    // Signal from the watchdog (Terminate)
    if (sig == SIGTERM) {
        // Close the file descriptors and delete the FIFOs
        close(fd_cmd);
        close(fd_world);
        unlink(mx_world); 
        // Close the log file and exit
        fclose(log_file); 
        exit(EXIT_SUCCESS);
    }
}

int main(int argc, char *argv[]) {

    // Register the signal handlers
    if (signal(SIGUSR2, rst) == SIG_ERR)
        printf("\ncan't catch SIGUSR2\n");
    if (signal(SIGTERM, exit_handler) == SIG_ERR)
        printf("\ncan't catch SIGTERM\n");

    fd_set readfds;
    int retval;
    struct timeval tv;

    // Creating the logfile
    log_file = fopen("./log/motor_x.log", "w");
    if (log_file == NULL) {
        printf("Cannot create logfile\n");
        exit(EXIT_FAILURE);
    }

    // Create the named pipe
    char * cmd_mx = "/tmp/cmd_mx";
    mx_world = "/tmp/mx_world"; 
    mkfifo(mx_world, 0666);

    while (1) {
        // Get the current time to write on the logfile
        time_t clk = time(NULL);
        char * timestamp = ctime(&clk);
        timestamp[24] = ' ';

        // Open the named pipe for reading
        fd_cmd = open(cmd_mx, O_RDONLY);

        // Store the current value of Endeffector_x
        float Endeffector_x_prev = Endeffector_x;

        // Initialize the file descriptor set for the select() call
        FD_ZERO(&readfds);
        FD_SET(fd_cmd, &readfds);

        // Set the timeout value for select()
        tv.tv_sec = freq;

        // Wait for data to be available on the named pipe
        retval = select(fd_cmd+1, &readfds, NULL, NULL, &tv);

        switch (retval) {
            case -1:
                // Handle error in select()
                perror("select()");
                break;
            case 1: ;
                // Read data from the named pipe
                int rd_cmd = read(fd_cmd, command_c, SIZE);
                if (rd_cmd == 0){
                    perror("Cannot read pipe.");
                }
                // Convert the received data to a float
                velocity_x = atof(command_c);
                close(fd_cmd);
                break;
        }    

        if(velocity_x != 0 ) {
            // Update the position of the end-effector
            Endeffector_x += velocity_x;
        }
        
        if (Endeffector_x < X_MIN) {
            // Limit the position of the end-effector to the minimum value
            Endeffector_x = X_MIN;
        }  
	else if (Endeffector_x > X_MAX) { // If Endeffector_x is greater than X_MAX,
	    Endeffector_x = X_MAX; // Set Endeffector_x to X_MAX.
	}

// We only send to world when we update the position
	if (Endeffector_x_prev != Endeffector_x) { // If the previous Endeffector_x value is different from the current Endeffector_x value,
	    sprintf(command_x, "%f", Endeffector_x); // Convert Endeffector_x to a string and store it in the buffer command_x.
	    fd_world = open(mx_world, O_WRONLY); // Open the named pipe for writing.
	    int wr_world = write(fd_world, command_x, SIZE); // Write the buffer to the named pipe.
	    if (wr_world==0){
		perror ("Cannot write in pipe"); // If the write operation failed, print an error message.
	    }
	}	

	close(fd_cmd); // Close the command pipe.
	close(fd_world); // Close the world pipe.
	}

fclose(log_file); // Close the log file.

// Return 0 to indicate successful completion.
return 0;
}
