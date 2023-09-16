#include "Maze.h"
/* Display the maze. */
void ShowMaze(const char *maze, int width, int height) {
   int x, y;
   for(y = 0; y < height; y++) {
      for(x = 0; x < width; x++) {
         switch(maze[y * width + x]) {
         case 1:  printf("[]");  break;
         case 2:  printf("<>");  break;
         default: printf("  ");  break;
         }
      }
      printf("\n");
   }
}

static unsigned char our_memory[1024 * 1024]; //reserve 1 MB for malloc
static size_t next_index = 0;

void *selfmalloc(size_t sz)
{
    void *mem;

    if(sizeof our_memory - next_index < sz)
        return NULL;

    mem = &our_memory[next_index];
    next_index += sz;
    return mem;
}

void selffree(void *mem)
{
   //we cheat, and don't free anything.
}

static uint32_t xorshift_state = 42; // You can initialize it with any non-zero value.

// Function to generate a random 32-bit integer.
uint32_t xorshift32() {
    xorshift_state ^= (xorshift_state << 13);
    xorshift_state ^= (xorshift_state >> 17);
    xorshift_state ^= (xorshift_state << 5);
    return xorshift_state;
}

// Function to generate a random integer within a given range.
int rand_range(int min, int max) {
    // Ensure that min <= max.
    if (min > max) {
        int temp = min;
        min = max;
        max = temp;
    }

    // Calculate the range and add 1 to include max.
    int range = max - min + 1;

    // Use the xorshift32() function to get a random number and scale it to the desired range.
    uint32_t random_value = xorshift32();
    return min + (int)(random_value % range);
}


/*  Carve the maze starting at x, y. */
void CarveMaze(char *maze, int width, int height, int x, int y) {

   int x1, y1;
   int x2, y2;
   int dx, dy;
   int dir, count;

   dir = rand_range(1,100) % 4;
   count = 0;
   while(count < 4) {
      dx = 0; dy = 0;
      switch(dir) {
      case 0:  dx = 1;  break;
      case 1:  dy = 1;  break;
      case 2:  dx = -1; break;
      default: dy = -1; break;
      }
      x1 = x + dx;
      y1 = y + dy;
      x2 = x1 + dx;
      y2 = y1 + dy;
      if(   x2 > 0 && x2 < width && y2 > 0 && y2 < height
         && maze[y1 * width + x1] == 1 && maze[y2 * width + x2] == 1) {
         maze[y1 * width + x1] = 0;
         maze[y2 * width + x2] = 0;
         x = x2; y = y2;
         dir = rand_range(1,100) % 4;
         count = 0;
      } else {
         dir = (dir + 1) % 4;
         count += 1;
      }
   }

}

void GenerateMaze(char *maze, int width, int height) {
   int x,y, dir;
   int frontier = 0;
   int frontierParsed = 0;
   int *xDirArrays = (int*)selfmalloc(width * height * sizeof(int));
   int *yDirArrays = (int*)selfmalloc(width * height * sizeof(int));
   for (x = 0; x < width * height; x++) {
      maze[x] = 1;
   }

   int randomX = rand_range(0, width);
   int randomY = rand_range(0, height);
   x = randomX;
   y = randomY;

   maze[randomY * width + randomX] = 0;
   // CHECK FOR NORTH FRONTIER
   if (y *width + x - 2*width > 0) {
      *(xDirArrays + frontier) = x;
      *(yDirArrays + frontier) = y - 2;
      frontier++;
   }
   //CHECK FOR EAST FRONTIER
   if (y*width + x + 2 < (y + 1) * width){
      *(xDirArrays + frontier) = x + 2;
      *(yDirArrays + frontier) = y;
      frontier++;
   }
   //CHECK FOR SOUTH FRONTIER
   if (y* width + x + 2 *width < width * height) {
      *(xDirArrays + frontier) = x;
      *(yDirArrays + frontier) = y + 2;
      frontier++;
   }
   //CHECK FOR WEST FRONTIER
   if (y*width + x - 2 > y * width - 1) {
      *(xDirArrays + frontier) = x - 2;
      *(yDirArrays + frontier) = y;
      frontier++;
   }
   for (int i = 0; i < 199; i++) {
   while (1) {
         dir = rand_range(0, frontier - 1);
         if (   maze[*(yDirArrays + dir)*width + *(xDirArrays + dir)] == 0) {
            continue;
         }
         maze[*(yDirArrays + dir)*width + *(xDirArrays + dir)] = 0;
         frontierParsed++;
         break;
      }
      // CASE FRONTIER WAS TOP
      if (maze[(*(yDirArrays + dir) + 2)*width + *(xDirArrays + dir)] == 0 && (*(yDirArrays + dir))*width + *(xDirArrays + dir) > 0 && (*(yDirArrays + dir) + 2)*width + *(xDirArrays + dir) < width * height) {
         maze[(*(yDirArrays + dir) + 1)*width + *(xDirArrays + dir)] = 0;
      }
      // CASE FRONTIER WAS EAST
      else if (maze[*(yDirArrays + dir)*width + *(xDirArrays + dir) - 2] == 0 && (*(yDirArrays + dir)*width + *(xDirArrays + dir)) < (*(yDirArrays + dir)*width + width) && (*(yDirArrays + dir)*width + *(xDirArrays + dir) - 2) >= (*(yDirArrays + dir)*width)) {
         maze[*(yDirArrays + dir)*width + *(xDirArrays + dir) - 1] = 0;
      }
      // CASE FRONTIER WAS SOUTH
      else if (maze[(*(yDirArrays + dir) - 2)*width + *(xDirArrays + dir)] == 0 && (*(yDirArrays + dir))*width + *(xDirArrays + dir) < width * height && (*(yDirArrays + dir) - 2)*width + *(xDirArrays + dir) > 0) {
         maze[(*(yDirArrays + dir) - 1)*width + *(xDirArrays + dir)] = 0;
      }
      else if (maze[*(yDirArrays + dir)*width + *(xDirArrays + dir) + 2] == 0 && (*(yDirArrays + dir)*width + *(xDirArrays + dir)) >(*(yDirArrays + dir)*width) - 1) {
         maze[*(yDirArrays + dir)*width + *(xDirArrays + dir) + 1] = 0;
      }
      x = *(xDirArrays + dir);
      y = *(yDirArrays + dir);
      // CHECK FOR NORTH FRONTIER
      if (y *width + x - 2*width > 0 && maze[y *width + x - 2*width] != 0) {
         *(xDirArrays + frontier) = x;
         *(yDirArrays + frontier) = y - 2;
         frontier++;
      }
      //CHECK FOR EAST FRONTIER
      if (y*width + x + 2 < (y + 1) * width && maze[y*width + x + 2] != 0){
         *(xDirArrays + frontier) = x + 2;
         *(yDirArrays + frontier) = y;
         frontier++;
      }
      //CHECK FOR SOUTH FRONTIER
      if (y* width + x + 2 *width < width * height && maze[y* width + x + 2 *width] != 0) {
         *(xDirArrays + frontier) = x;
         *(yDirArrays + frontier) = y + 2;
         frontier++;
      }
      //CHECK FOR WEST FRONTIER
      if (y*width + x - 2 > y * width - 1 && maze[y*width + x - 2] != 0) {
         *(xDirArrays + frontier) = x - 2;
         *(yDirArrays + frontier) = y;
         frontier++;
      }
   }

   // while (1) {
   //       dir = rand_range(0, frontier - 1);
   //       if (   maze[*(yDirArrays + dir)*width + *(xDirArrays + dir)] == 0) {
   //          maze[*(yDirArrays + dir)*width + *(xDirArrays + dir)] = 2;
   //          break;
   //       }
   //    }
   while (1) {
      dir = rand_range(0, width * height);
      if (   maze[dir] == 0) {
         maze[dir] = 2;
         break;
      }
   }

   printf("This is amount of frontier: %d\n", frontier - frontierParsed);
   // for (int i = 0; i < frontier; i++) {
   //    maze[*(yDirArrays + i)*width + *(xDirArrays + i)] = 0;
   // }
}