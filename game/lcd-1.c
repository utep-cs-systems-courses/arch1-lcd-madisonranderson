/** \file lcddemo.c
 *  \brief A simple demo that draws a string and square
 */

#include <libTimer.h>
#include "lcdutils.h"
#include "lcddraw.h"

/** Initializes everything, clears the screen, draws "hello" and a square */
int
main()
{
  configureClocks();
  lcd_init();
  u_char width = screenWidth, height = screenHeight;

  clearScreen(COLOR_BLUE);

  short centerR = LARGE_EDGE_PIXELS/2;
  short centerC = SHORT_EDGE_PIXELS/2;
  short col = 30;
  short startc = 30, startr = 30;
  for(col = 0; col < 30;){
    short row = col;
    short leftEdge = -col;
    short rightEdge = col;
    //drawPixel(startc+leftEdge, startr+row, COLOR_PINK);
    for (short c = leftEdge; c <= rightEdge; c++)
      drawPixel(startc+c, startr+row, COLOR_PINK);
    col += 1;
  }
  drawString5x7(20,20, "hello", COLOR_GREEN, COLOR_RED);

  //  fillRectangle(30,30, 60, 60, COLOR_ORANGE);
  
}
