#include "buzzer.h"


int switchBeep(int x)
{
  switch (x){
  case 1:
    buzzer_set_period(1000);
    break;
  case 2:
    buzzer_set_period(2000);
      break;
  }
}
