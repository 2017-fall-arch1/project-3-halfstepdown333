#include <msp430.h>
#include <libTimer.h>
#include <lcdutils.h>
#include <lcddraw.h>
#include <p2switches.h>
#include <shape.h>
#include <abCircle.h>

#define GREEN_LED BIT6

char pong1=0;
char pong2=0;

AbRect left = {abRectGetBounds, abRectCheck,{2,15}}; 
AbRect right = {abRectGetBounds, abRectCheck,{2,15}};

AbRectOutline fieldOutline = {	/* playing field */
  abRectOutlineGetBounds, abRectOutlineCheck,   
  {screenWidth/2 - 10, screenHeight/2 - 10}
};

Layer layer4 = {
  (AbShape *) &circle5,
  {(screenWidth/2)+10, (screenHeight/2)+5}, /**< bit below & right of center */
  {0,0}, {0,0},				    /* last & next pos */
  COLOR_PINK,
  0
};


Layer fieldLayer = {		/* playing field as a layer */
  (AbShape *) &fieldOutline,
  {screenWidth/2, screenHeight/2},/**< center */
  {0,0}, {0,0},				    /* last & next pos */
  COLOR_BLUE,
  &layer4
};

Layer layer1 = {	       
  (AbShape *)&left,
  {(screenWidth/2)-51, (screenHeight/2)-51}, /**< center */
  {0,0}, {0,0},				    /* last & next pos */
  COLOR_RED,
  &fieldLayer,
};

Layer layer0 = {		
  (AbShape *)&right,
  {(screenWidth/2)+51, (screenHeight/2)+51}, /**< bit below & right of center */
  {0,0}, {0,0},				    /* last & next pos */
  COLOR_RED,
  &layer1,
};

/** Moving Layer
 *  Linked list of layer references
 *  Velocity represents one iteration of change (direction & magnitude)
 */
typedef struct MovLayer_s {
  Layer *layer;
  Vec2 velocity;
  struct MovLayer_s *next;
} MovLayer;

/* initial value of {0,0} will be overwritten */
MovLayer ml0 = { &layer4, {3,3}, 0 }; /**< not all layers move */
MovLayer mll = { &layer1, {0,0}, &ml0 }; 
MovLayer ml2 = { &layer0, {0,0}, &mll }; 


int returnBall(Vec2 *new, u_int x){
  int velocity3 =0;
    if(abShapeCheck(mll.layer->abShape,&mll.layer->posNext, new) ||
       (abShapeCheck(ml2.layer->abShape,&ml2.layer->posNext,new))){
	velocity3 = ml0.velocity.axes[x]= -ml0.velocity.axes[x];
	return velocity3;
}
}



void movLayerDraw(MovLayer *movLayers, Layer *layers)
{
  int row, col;
  MovLayer *movLayer;

  and_sr(~8);			/**< disable interrupts (GIE off) */
  for (movLayer = movLayers; movLayer; movLayer = movLayer->next) { /* for each moving layer */
    Layer *l = movLayer->layer;
    l->posLast = l->pos;
    l->pos = l->posNext;
  }
  or_sr(8);			/**< disable interrupts (GIE on) */


  for (movLayer = movLayers; movLayer; movLayer = movLayer->next) { /* for each moving layer */
    Region bounds;
    layerGetBounds(movLayer->layer, &bounds);
    lcd_setArea(bounds.topLeft.axes[0], bounds.topLeft.axes[1], 
		bounds.botRight.axes[0], bounds.botRight.axes[1]);
    for (row = bounds.topLeft.axes[1]; row <= bounds.botRight.axes[1]; row++) {
      for (col = bounds.topLeft.axes[0]; col <= bounds.botRight.axes[0]; col++) {
	Vec2 pixelPos = {col, row};
	u_int color = bgColor;
	Layer *probeLayer;
	for (probeLayer = layers; probeLayer; 
	     probeLayer = probeLayer->next) { /* probe all layers, in order */
	  if (abShapeCheck(probeLayer->abShape, &probeLayer->pos, &pixelPos)) {
	    color = probeLayer->color;
	    break; 
	  } /* if probe check */
	} // for checking all layers at col, row
	lcd_writeColor(color); 
      } // for col
    } // for row
  } // for moving layer being updated
}	  



//Region fence = {{10,30}, {SHORT_EDGE_PIXELS-10, LONG_EDGE_PIXELS-10}}; /**< Create a fence region */

/** Advances a moving shape within a fence
 *  
 *  \param ml The moving shape to be advanced
 *  \param fence The region which will serve as a boundary for ml
 */
void mlAdvance(MovLayer *ml, Region *fence)
{
  u_char c,c2;
  Vec2 newPos;
  u_char axis;
  Region shapeBoundary;
  for (; ml; ml = ml->next) {
    vec2Add(&newPos, &ml->layer->posNext, &ml->velocity);
    abShapeGetBounds(ml->layer->abShape, &newPos, &shapeBoundary);
    c =0;
    c2=0;
    for (axis = 0; axis < 2; axis ++) {
      if (shapeBoundary.topLeft.axes[axis] < fence->topLeft.axes[axis]) {
	  int velocity = ml->velocity.axes[axis] = -ml->velocity.axes[axis];
	newPos.axes[axis] += (2*velocity);
	
	if(ml->layer->abShape == ml0.layer->abShape & (c < 1)){
	pong1 +=1;
	c +=1;
      }
	}
      else if(shapeBoundary.botRight.axes[axis] > fence->botRight.axes[axis]){
	int velocity = ml->velocity.axes[axis]= -ml->velocity.axes[axis];
	newPos.axes[axis] +=(2*velocity);
	
	if(ml->layer->abShape == ml0.layer->abShape & (c2 < 1)){
	pong2 +=1;
	c2 +=1;
      }
      }
      if(ml->layer->abShape ==ml0.layer->abShape){
	int velocity = returnBall(&newPos,axis);
	newPos.axes[axis] +=(2*velocity);
      }
      ml->layer->posNext = newPos;
    } /**< for ml */
  }
}

u_int bgColor = COLOR_BLACK;     /**< The background color */
int redrawScreen = 1;           /**< Boolean for whether screen needs to be redrawn */

Region fieldFence;		/**< fence around playing field  */


/** Initializes everything, enables interrupts and green LED, 
 *  and handles the rendering for the screen
 */
void main()
{
  P1DIR |= GREEN_LED;		/**< Green led on when CPU on */		
  P1OUT |= GREEN_LED;

  configureClocks();
  lcd_init();
  shapeInit();
  p2sw_init(15);
  shapeInit();
  layerInit(&layer0);
  layerDraw(&layer0);
  layerGetBounds(&fieldLayer, &fieldFence);
  enableWDTInterrupts();      /**< enable periodic interrupt */
  or_sr(0x8);	              /**< GIE (enable interrupts) */

  
  

  for(;;) { 
    while (!redrawScreen) { /**< Pause CPU if screen doesn't need updating */
      P1OUT &= ~GREEN_LED;    /**< Green led off witHo CPU */
      or_sr(0x10);	      /**< CPU OFF */
    }
    if(pong1)
      drawChar5x7(60,0,'0'+ pong2,COLOR_WHITE, COLOR_BLACK);

    if(pong2)
      drawChar5x7(63,152,'0'+ pong1,COLOR_WHITE, COLOR_BLACK);

    P1OUT |= GREEN_LED;       /**< Green led on when CPU on */
    redrawScreen = 0;
    movLayerDraw(&ml2, &layer0);
  }
}

/** Watchdog timer interrupt handler. 15 interrupts/sec */
void wdt_c_handler()
{
  static short count = 0;
  P1OUT |= GREEN_LED;		      /**< Green LED on when cpu on */
  count ++;
  if (count == 15) {
    mlAdvance(&ml2, &fieldFence);
    int buttons=~p2sw_read();

    if(buttons & BIT0){
      mll.velocity.axes[1]=3;
    }
    else if(buttons & BIT1){
      mll.velocity.axes[1]=-3;
    } 
    else if(buttons & BIT2){
      ml2.velocity.axes[1]=3;
    }
    else if(buttons & BIT3){
      ml2.velocity.axes[1]=-3;
    }
    else{
      mll.velocity.axes[1]=0;
      ml2.velocity.axes[1]=0;
    }

    redrawScreen = 1;
    count = 0;
  } 
  P1OUT &= ~GREEN_LED;		    /**< Green LED off when cpu off */
}
