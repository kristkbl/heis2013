#ifndef __INCLUDE_CAR_H__
#define __INCLUDE_CAR_H__


#define SPEED_UP 300
#define SPEED_DOWN -300
#define DOOR_OPEN_TIME 3		
#define BRAKE_TIME 10000



typedef enum {	
	CAR_INVALID, 
	CAR_MOVING,
	CAR_STANDSTILL,
	CAR_EMG_STOPPED,
	N_CAR_STATES
} car_state_t;


typedef enum {				
	CAR_NO_EVENT,
	CAR_MOVE,
	CAR_HALT, 
	CAR_EMG_STOP,
	N_CAR_EVENTS
} car_event_t; 

typedef enum { 
  	STANDSTILL = -1,		
   	UP = 0, 
  	DOWN = 1, 
   	N_DIRECTIONS = 2
} direction_t;


void car_stop();

void door_close();

void door_open();

/** initializes elevator car
    @return STANDSTILL */
car_state_t car_init();

/** updates states
    @return current_state */
car_state_t car_update_state();

/** Checks current state and returns next event
    @param current_state and current floor
    @return next event based on state and floor */
car_event_t car_get_next_event(car_state_t state, int floor);

/** Checks for obstruction and pauses car if obstruction
    @return CAR_NO_EVENT if EMG_BUTTON is not pressed */
car_event_t car_handle_obstruction();


#endif // #ifndef __INCLUDE_CAR_H__
