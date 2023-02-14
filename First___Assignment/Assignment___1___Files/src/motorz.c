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

#define Z_MIN 0
#define Z_MAX 10

float SPEED_MINIMUM = -2;
float SPEED_MAXIMUM = 2;

float velocity_z = 0;
float freq = 1;

int update = 0;
int reset = 0;

// File descriptors
int fd_cmd, fd_world;

// End-effector coordinates
float Endeffector_z = 0;

char command_c[SIZE];
char command_z[SIZE];
char * mz_world;

FILE *log_file;


void rst(int sig) {
    // Signal handler for SIGUSR2
    // Signal from the inspection console (Reset)
    if (sig == SIGUSR2) {
        velocity_z = -1;
    }
}

void exit_handler(int sig) {
    // Signal handler for SIGTERM
    // Signal from the watchdog (Terminate)
    if (sig == SIGTERM) {
        close(fd_cmd);
        close(fd_world);
        unlink(mz_world); 
        fclose(log_file); 
        exit(EXIT_SUCCESS);
    }
}

int main(int argc, char *argv[]) {
    // Signal handlers for SIGUSR2 and SIGTERM
    if (signal(SIGUSR2, rst) == SIG_ERR)
        printf("\ncan't catch SIGUSR2\n");
    if (signal(SIGTERM, exit_handler) == SIG_ERR)
        printf("\ncan't catch SIGTERM\n");

    fd_set readfds;
    int retval;
    struct timeval tv;

    // Creating the logfile
    log_file = fopen("./log/motot_z.log", "w");
    if (log_file == NULL) {
        perror("Cannot create logfile");
        exit(EXIT_FAILURE);
    }

    // Pipes
    char * cmd_mz = "/tmp/cmd_mz";
    mz_world = "/tmp/mz_world"; 
    mkfifo(mz_world, 0666);

    while (1) {
        // Get the current time to write on the logfile
        time_t clk = time(NULL);
        char * timestamp = ctime(&clk);
        timestamp[24] = ' ';

        // Pipe open
        fd_cmd = open(cmd_mz, O_RDONLY);   

        float Endeffector_z_prev = Endeffector_z;

        // Initialize with an empty set the file descriptors set
        FD_ZERO(&readfds);
        // Add the file descriptor to the file descriptors set
        FD_SET(fd_cmd, &readfds);

        tv.tv_sec = freq;

        retval = select(fd_cmd+1, &readfds, NULL, NULL, &tv);

        switch (retval) {
            case -1:
                perror("select()");
                break;
            case 1: ;
                int rd_cmd = read(fd_cmd, command_c, SIZE);
                if (rd_cmd == 0) {
                    perror("Cannot read pipe.");
                }
                velocity_z = atof(command_c);
                close(fd_cmd);
                break;
        }    

        if (velocity_z != 0 ) {
            // End-effector moves up or down with the input speed
            Endeffector_z += velocity_z;
            // enviar señal a watchdog cuando hay un cambio?
        }
        else {
            // End-effector is stopped
        }
        
        if (Endeffector_z < Z_MIN) {
            Endeffector_z = Z_MIN;
        }  
        else if (Endeffector_z > Z_MAX) {
            Endeffector_z = Z_MAX;   
        }


              // Send the current position to the world process only if the position has changed.
        if (Endeffector_z_prev != Endeffector_z) {
            // Convert the position to a string and store it in the command buffer.
            sprintf(command_z, "%f", Endeffector_z);
            
            // Open the pipe to the world process for writing.
            fd_world = open(mz_world, O_WRONLY);
            
            // Write the position to the pipe.
            int wr_world = write(fd_world, command_z, SIZE);
            
            // Check if the write was successful, and print an error message if not.
            if (wr_world == 0) {
                perror ("Cannot write in pipe");
            }
        }

        // Close the file descriptors for the command and world pipes.
        close(fd_cmd);
        close(fd_world);
    }

    // Close the log file.
    fclose(log_file); 

    // Return 0 to indicate successful completion.
    return 0;
}
