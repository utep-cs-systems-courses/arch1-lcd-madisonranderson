#include <msp430.h>
#include <libTimer.h>
#include "lcdutils.h"
#include "lcddraw.h"

// WARNING: LCD DISPLAY USES P1.0.  Do not touch!!! 

#define LED BIT6		/* note that bit zero req'd for display */

#define SW1 1
#define SW2 2
#define SW3 4
#define SW4 8

#define SWITCHES 15

static char 
switch_update_interrupt_sense()
{
  char p2val = P2IN;
  /* update switch interrupt to detect changes from current buttons */
  P2IES |= (p2val & SWITCHES);	/* if switch up, sense down */
  P2IES &= (p2val | ~SWITCHES);	/* if switch down, sense up */
  return p2val;
}

void 
switch_init()			/* setup switch */
{  
  P2REN |= SWITCHES;		/* enables resistors for switches */
  P2IE |= SWITCHES;		/* enable interrupts from switches */
  P2OUT |= SWITCHES;		/* pull-ups for switches */
  P2DIR &= ~SWITCHES;		/* set switches' bits for input */
  switch_update_interrupt_sense();
}

int switches = 0;

void
switch_interrupt_handler()
{
  char p2val = switch_update_interrupt_sense();
  switches = ~p2val & SWITCHES;
}


// axis zero for col, axis 1 for row
short drawPos[2] = {10,10}, controlPos[2] = {10,10};
short velocityArray[2] = {3,8}, limits[2] = {screenWidth-36, screenHeight-8};

short refresh = 1;
u_int controlFontColor = COLOR_GREEN;
u_char currentRow = 10;
u_char nextRow = 10;
signed char velocity = 1;

void wdt_c_handler()
{
  static int secCount = 0, secondCount =0;

  secCount++;
  secondCount++;
  //if (secCount >= 25) {		/* 10/sec */
  //  secCount = 0;
  //  redrawScreen = 1;
  //}
  if(secCount == 250) {
    secCount = 0;
    refresh = 1;
  }
  if(secondCount == 25) {
    secondCount = 0;
    nextRow += velocity;
    if(nextRow > 30 || nextRow < 10) {
      velocity = -velocity;
      nextRow += velocity;
    }
    refresh = 1;
  }
}

void fillTriangle(u_char positionCol, u_char positionRow, u_char size, u_int color)
{
  for(int row = 0; row <= size; row++) {
    for(int col = 0; col <= size - row; col++) {
      drawPixel(positionCol + col, positionRow + row, color);
      drawPixel(positionCol - col, positionRow + row, color);
    }
  }
}

void fillHeart(u_char positionCol, u_char positionRow, u_char size, u_int color)
{
  int i, j;
  for(i = size/2; i <= size; i+=2){
    for(j = 1; j < i; j++){
      drawPixel(positionCol, positionRow + i, color);      // connect left and right upprt half 
      drawPixel(positionCol + j, positionRow + i, color);  // right side of upper half of <3
      drawPixel(positionCol - j, positionRow + i, color);  // left side of upper half of <3
    }
  }
  fillTriangle(positionCol, positionRow+i, size, color);
}

void fillDiamond(u_char positionCol, u_char positionRow, u_char size, u_int color)
{
  for(int row = 0; row <= size; row++) {
    for(int col = 0; col <= size - row; col++) {
      drawPixel(positionCol + col, positionRow - row, color);
      drawPixel(positionCol - col, positionRow - row, color);
    }
  }
  fillTriangle(positionCol, positionRow, size, color);
}

void state_button1() {
  if(refresh) {
    refresh = 0;
    fillTriangle(60, nextRow, 30, COLOR_BLUE);
    fillTriangle(60, nextRow, 30, COLOR_WHITE);
    currentRow = nextRow;
  }
};

void state_button2() {
  if(refresh) {
    refresh = 0;
    int heartRow = nextRow + 60;
    fillHeart(60, heartRow, 20, COLOR_BLUE);
    fillHeart(60, heartRow, 20, COLOR_WHITE);
    currentRow = nextRow;
  }
};

void state_button3() {
  if(refresh) {
    refresh = 0;
    int diamondCol = nextRow + 45;
    fillDiamond(30, diamondCol, 15, COLOR_BLUE);
    fillDiamond(30, diamondCol, 15, COLOR_WHITE);
    
    fillDiamond(90, diamondCol, 15, COLOR_BLUE);
    fillDiamond(90, diamondCol, 15, COLOR_WHITE);
    currentRow = nextRow;
  }
};

void state_button4() { clearScreen(COLOR_BLUE); };

void default_state() {
  if(refresh) {
    refresh = 0;
    currentRow = nextRow;
  }
};

void selectState()
{
  if (switches & SW1) state_button1();
  else if (switches & SW2) state_button2();
  else if (switches & SW3) state_button3();
  else if (switches & SW4) state_button4();
  else default_state();
}
  
void update_shape();

void main()
{
  
  P1DIR |= LED;		/**< Green led on when CPU on */
  P1OUT |= LED;
  configureClocks();
  lcd_init();
  switch_init();
  
  enableWDTInterrupts();      /**< enable periodic interrupt */
  or_sr(0x8);	              /**< GIE (enable interrupts) */
  
  clearScreen(COLOR_BLUE);
  while (1) {			/* forever */
    selectState();
    P1OUT &= ~LED;	/* led off */
    or_sr(0x10);	/**< CPU OFF */
    P1OUT |= LED;	/* led on */
  }
}

    
    
void
update_shape()
{
  static unsigned char row = screenHeight / 2, col = screenWidth / 2;
  static char blue = 31, green = 0, red = 31;
  static unsigned char step = 0;
  if (switches & SW4) return;
  if (step <= 60) {
    int startCol = col - step;
    int endCol = col + step;
    int width = 1 + endCol - startCol;
    // a color in this BGR encoding is BBBB BGGG GGGR RRRR
    unsigned int color = (blue << 11) | (green << 5) | red;
    fillRectangle(startCol, row+step, width, 1, color);
    fillRectangle(startCol, row-step, width, 1, color);
    if (switches & SW3) green = (green + 10) % 64;
    if (switches & SW2) blue = (blue + 2) % 32;
    if (switches & SW1) red = (red - 3) % 32;
    step ++;
  } else {
     clearScreen(COLOR_BLUE);
     step = 0;
  }
}


/* Switch on S2 */
void
__interrupt_vec(PORT2_VECTOR) Port_2(){
  if (P2IFG & SWITCHES) {	      /* did a button cause this interrupt? */
    P2IFG &= ~SWITCHES;		      /* clear pending sw interrupts */
    switch_interrupt_handler();	/* single handler for all switches */
  }
}
