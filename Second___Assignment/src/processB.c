#include "./../include/processB_utilities.h"

#include <bmpfile.h>
/*
In this project we are going to use the bmpfile library to read a bitmap image.
*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>

// Define the size of the shared memory object.
const int SIZE = 1600*600*sizeof(int);

// Declare a pointer to a bmpfile_t.
bmpfile_t *bmp;

// Declare variables for the coordinates of the center of the image.
int center_x;
int center_y;

// Declare the name of the shared memory object.
const char* name = "NM";

// Declare the shared memory file descriptor.
int shm_fd;

// Declare a pointer to the shared memory object.
int* ptr;

// Declare the functions used in this program.
void load_shared_memory(void);
void find_center(void);

int main(int argc, char const *argv[])
{

    sleep(1);

    // Open the shared memory object.
    shm_fd = shm_open(name, O_RDONLY, 0666);
 
    // Memory map the shared memory object.
    ptr = mmap(0, SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);
    
    // Utility variable to avoid trigger resize event on launch
    int first_resize = TRUE;

    // Initialize UI
    init_console_ui();

    // Infinite loop
    while (TRUE) {

        // Get input in non-blocking mode
        int cmd = getch();

        // If user resizes screen, re-draw UI...
        if(cmd == KEY_RESIZE) {
            if(first_resize) {
                first_resize = FALSE;
            }
            else {
                reset_console_ui();
            }
        }

        else {

                load_shared_memory();
                find_center();
               bmp_destroy(bmp);

        }
    }

    endwin();

    // Return 0 to indicate successful completion.
    return 0;
}

void find_center(void){

    // Initialize the variables for finding the center of the image.
    int y_min=600;
    int y_max=0;
    int x_min=1600;
    int x_max=0;    
    int center_x;
    int center_y;

    // Loop over every pixel in the image to find the center.
    for(int x=0;x<1600;x++)
    {
      for(int y=0;y<600;y++)
      {

        // Get the color of the current pixel.
        rgb_pixel_t *pixel = bmp_get_pixel(bmp ,x, y);

        // If the pixel is blue, update the bounding box.
        if(pixel->blue == 80)
        {
          if(x<x_min) {x_min=x;};
          if(x>x_max) {x_max=x;};
          if(y<y_min) {y_min=y;};
          if(y>y_max) {y_max=y;};
        }

      }
    }

    // Calculate the coordinates of the center of the image.
    center_x=(x_min+x_max)/2;
    center_y=(y_min+y_max)/2;

    // Convert the center coordinates to match the console size.
    center_x=center_x/(1600/COLS);
    center_y=center_y/(600/LINES);

    // Print the center coordinates to the console.
    mvprintw(LINES - 1, 1, "center_x: %d  ", center_x);
    mvprintw(LINES - 2, 1, "center_y: %d  ");
    
    
mvaddch(center_y, center_x, '0'); // Move the cursor to the specified location and add the character '0'.
refresh(); // Refresh the screen.

// Refresh the screen again.
refresh();

void load_shared_memory(void){

    bmp = bmp_create(1600, 600, 4); // Create a bitmap with the specified width, height, and depth.

    // Loop through each pixel in the bitmap.
    for(int x = 0; x < 1600; x++)
    {
        for(int y = 0; y < 600; y++)
        {
            // Check if the value of the corresponding memory location is 80.
            if (ptr[x*599+y]==80)
            {
                // Create a red pixel.
                rgb_pixel_t pixel = {ptr[x*599+y], 0, 0, 0};
                bmp_set_pixel(bmp,x,y, pixel); // Set the pixel at the specified location to the red pixel.
            }
            else{
                // Create a white pixel.
                rgb_pixel_t pixel = {255, 255, 255, 0};
                bmp_set_pixel(bmp,x,y, pixel); // Set the pixel at the specified location to the white pixel.
            }
        }
    }

}

