#include "./../include/processA_utilities.h"
#include <bmpfile.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>

/* the size (in bytes) of shared memory object */
const int SIZE = 1600 * 600 * sizeof(int);

bmpfile_t *bmp;
rgb_pixel_t pixel = {0, 80, 0, 0};
const char* name = "NM"; /* name of the shared memory object */
int shm_fd; /* shared memory file descriptor */
int* ptr; /* pointer to shared memory object */
bool print_flag = false;
int width = 1600;
int height = 600;
int depth = 4;

void bitmap_creat(int circle_x, int circle_y);
void update_shared_memory(bmpfile_t *bmp);

int main(int argc, char *argv[])
{
    /* create the shared memory object */
    shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);

    /* configure the size of the shared memory object */
    ftruncate(shm_fd, SIZE);

    /* memory map the shared memory object */
    ptr = mmap(0, SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0);

    /* Initialize bitmap and console UI */
    init_console_ui();
    bitmap_creat(circle.x, circle.y);

    /* Infinite loop for user input */
    while (TRUE)
    {
        int cmd = getch(); /* get input in non-blocking mode */

        /* If user resizes screen, re-draw UI... */
        if (cmd == KEY_RESIZE)
        {
            if (first_resize)
            {
                first_resize = FALSE;
            }
            else
            {
                reset_console_ui();
            }
        }

        /* Else, if user presses print button... */
        else if (cmd == KEY_MOUSE)
        {
            if (getmouse(&event) == OK)
            {
                if (check_button_pressed(print_btn, &event))
                {
                    print_flag = true;
                    bitmap_creat(circle.x, circle.y);
                    mvprintw(LINES - 1, 1, "Print button pressed");
                    refresh();
                    sleep(1);
                    for (int j = 0; j < COLS - BTN_SIZE_X - 2; j++)
                    {
                        mvaddch(LINES - 1, j, ' ');
                    }
                }
            }
        }

        /* If input is an arrow key, move circle accordingly... */
        else if (cmd == KEY_LEFT || cmd == KEY_RIGHT || cmd == KEY_UP || cmd == KEY_DOWN)
        {
            move_circle(cmd);
            draw_circle();
            bitmap_creat(circle.x, circle.y);
        }
    }

    endwin();
    return 0;
}

/* Update shared memory with green value of each pixel in bitmap */


// Declare a pointer to hold the shared memory


void update_shared_memory(bmpfile_t *bmp)
{
    // Iterate over each pixel in the bitmap and store the green value in shared memory
    for(int x = 0; x < bmp->width; x++)
    {
        for(int y = 0; y < bmp->height; y++)
        {
            // Get the pixel at position (x, y)
            rgb_pixel_t *pixel = bmp_get_pixel(bmp, x, y);

            // Store the green value of the pixel in the shared memory
            ptr[x * bmp->height + y] = pixel->green;
        }
    }

    // Print information about a specific pixel (for debugging purposes)
    // rgb_pixel_t *pixel=bmp_get_pixel(bmp ,20, 20);
    // mvprintw(LINES - 1, 1, "blue=%d , red=%d , green=%d, alpha=%d",pixel->blue,pixel->red,pixel->green,pixel->alpha);
    // sleep(5);
}

void bitmap_create(int circle_x, int circle_y)
{
    // Create a new bitmap with the specified width, height, and color depth
    bmp = bmp_create(width, height, depth);

    // Set the color of the circle to green
    rgb_pixel_t pixel = {0, 80, 0, 0};

    // Iterate over each pixel in a circle around the center of the bitmap
    int radius = 20;
    for(int x = -radius; x <= radius; x++) 
    {
        for(int y = -radius; y <= radius; y++) 
        {
            // Check if the pixel is within the circle
            if(sqrt(x * x + y * y) < radius) {
                // Color the pixel with the green color
                bmp_set_pixel(bmp, circle_x * (width / (COLS - BTN_SIZE_X - 2)) + x, circle_y * (height / LINES) + y, pixel);
            }
        }
    }

    // Save the bitmap to a file if the print flag is set
    if(print_flag == true)
    {
        bmp_save(bmp, "NMRESULT");
        print_flag = false;
    }

    // Update the shared memory with the pixel data from the bitmap
    update_shared_memory(bmp);

    // Free the memory used by the bitmap
    bmp_destroy(bmp);
}

