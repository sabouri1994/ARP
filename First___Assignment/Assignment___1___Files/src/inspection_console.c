#include "./../include/inspection_utilities.h"
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
/**********************************************************/
#define SIZE 80

int fd_ins;

char msg[SIZE];
char line[SIZE];
char real_pos[SIZE];
char * world_ins;

int val=0;
/**********************************************************/
// End-effector coordinates
float Endeffector_x, Endeffector_z;

long cm, mx, mz;

FILE *log_file;
FILE *get_pid;

void stp(int sig) {

    if (sig == SIGUSR1) {

        kill(cm,SIGUSR2);
    }
}

void rst(int sig) {

    if (sig == SIGUSR2) {
            kill(mx, SIGUSR2);
            kill(mz, SIGUSR2);
            kill(cm,SIGUSR1);

    }
}

void exit_handler(int sig) {
    // Signal from the watchdog
    if (sig == SIGTERM) {
        close(fd_ins);
        unlink(world_ins);
        fclose(log_file); 
        exit(EXIT_SUCCESS);
    }
}
/**********************************************************/

int main(int argc, char const *argv[])
{
    // Utility variable to avoid trigger resize event on launch
    int first_resize = TRUE;

    // End-effector coordinates
    //float Endeffector_x, ee_y;

    // Initialize User Interface 
    init_console_ui();

    /**********************************************************/
    // Signal from the watchdog (Terminate)
    if (signal(SIGTERM, exit_handler) == SIG_ERR)
        printf("\ncan't catch SIGTERM\n");

    if (signal(SIGUSR1, stp) == SIG_ERR)
        printf("\ncan't catch SIGUSR1\n");

    if (signal(SIGUSR2, rst) == SIG_ERR)
        printf("\ncan't catch SIGUSR1\n");
    /**********************************************************/
    // Getting the pid of process "command"
    get_pid = popen("pidof -s command", "r");
    fgets(line, SIZE, get_pid);
    cm = strtoul(line, NULL, 10);

    // Getting the pid of process "motor_x"
    get_pid = popen("pidof -s motor_x", "r");
    fgets(line, SIZE, get_pid);
    mx = strtoul(line, NULL, 10);
    
    // Getting the pid of process "motot_z"
    get_pid = popen("pidof -s motot_z", "r");
    fgets(line, SIZE, get_pid);
    mz = strtoul(line, NULL, 10);
    /**********************************************************/
    // Creating the logfile
    log_file = fopen("./log/inspection_console.log", "w");
    if (log_file == NULL) { /* Something is wrong   */}
    /**********************************************************/
    // Pipe
    world_ins = "/tmp/world_ins";
    mkfifo(world_ins, 0666); 
    /**********************************************************/

    // Infinite loop
    while(TRUE)
	{	
        // Get mouse/resize commands in non-blocking mode...
        int cmd = getch();
        /**********************************************************/
        // Get the current time to write on the logfile
        time_t clk = time(NULL);
        char * timestamp = ctime(&clk);
        timestamp[24] = ' ';
        /**********************************************************/
        // If user resizes screen, re-draw UI
        if(cmd == KEY_RESIZE) {
            if(first_resize) {
                first_resize = FALSE;
            }
            else {
                reset_console_ui();
            }
        }
        // Else if mouse has been pressed
        else if(cmd == KEY_MOUSE) {

            // Check which button has been pressed...
            if(getmouse(&event) == OK) {

                // STOP button pressed
                if(check_button_pressed(stp_button, &event)) {
                    mvprintw(LINES - 1, 1, "STP button pressed");
                    refresh();
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }
                    /**********************************************************/
                    fprintf(log_file,"%s- STP button pressed\n", timestamp);
                    fflush(log_file); 

                    // Stop signal
                    if (signal(SIGUSR1, stp) == SIG_ERR)
                        printf("\ncan't catch SIGUSR1\n");
                        kill(getpid(), SIGUSR1);
                    /**********************************************************/
                }

                // RESET button pressed
                else if(check_button_pressed(rst_button, &event)) {
                    mvprintw(LINES - 1, 1, "RST button pressed");
                    refresh();
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }
                    /**********************************************************/
                    fprintf(log_file,"%s- RST button pressed\n", timestamp);
                    fflush(log_file); 

                    // Reset signal
                    if (signal(SIGUSR2, rst) == SIG_ERR)
                        printf("\ncan't catch SIGUSR1\n");     
                        kill(getpid(), SIGUSR2);
                    /**********************************************************/
                }
            }
        }
        /**********************************************************/
        // Get updated position from world
        fd_ins = open(world_ins, O_RDWR);
        if (fd_ins == 0){
            perror("Cannot open pipe");
        }
        int rd_ins = read(fd_ins, real_pos, SIZE);
        if (rd_ins== 0){
            perror("Cannot read pipe.");
        }
        sscanf(real_pos, "%f,%f", &Endeffector_x, &Endeffector_z);
        close(fd_ins);
        /**********************************************************/
        //To put in 0 velocities when the position returns to (0,0) or goes to (39,9)
        if (Endeffector_x==0.0 && Endeffector_z == 0.0 && val==1){
            kill(getpid(), SIGUSR1);
            val=0;
        }
        if (Endeffector_x!=0.0 || Endeffector_z != 0.0){
            val=1;
        }
   
        // Update UI
        update_console_ui(&Endeffector_x, &Endeffector_z);
	}

    fclose(log_file);
    // Terminate
    endwin();
    return 0;
}

