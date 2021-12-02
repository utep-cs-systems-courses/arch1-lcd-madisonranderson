#include <msp430.h>
#include "libTimer.h"
#include "buzzer.h"

static int counter = 0;

void buzzer_init() {
  timerAUpmode();
  P2SEL2 &= ~(BIT6 | BIT7);
  P2SEL &= ~BIT7;
  P2SEL |= BIT6;
  P2DIR = BIT6;
}

void superMarioTheme(){
  switch(counter){
    case 0:
    case 1:
    case 2:
    case 4: buzzer_set_period(750); counter++; break;
    case 3: buzzer_set_period(950); counter++; break;
    case 5: buzzer_set_period(630); counter++; break;
    case 6: buzzer_set_period; counter = 0; break;
  }
}

void buzzer_set_period(short cycles) {
  CCR0 = cycles;
  CCR1 = cycles >> 1;
}
