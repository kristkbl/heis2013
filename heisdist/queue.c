#include <stdbool.h>

#include "elev.h"
#include "queue.h"
#include "car.h"


bool queue_order_above(int floor){
	for(floor+=1; floor < N_FLOORS; floor++){
		if (queue[BUTTON_CALL_UP][floor] || queue[BUTTON_CALL_DOWN][floor] || queue[BUTTON_COMMAND][floor]){
			return true;
		}
	}	
	return false;
}


bool queue_order_below(int floor){
	for(floor-=1; floor >= 0; floor--){
		if (queue[BUTTON_CALL_DOWN][floor] || queue[BUTTON_CALL_UP][floor] || queue[BUTTON_COMMAND][floor]){
			return true;
		}
	}	
	return false;
}


direction_t queue_get_next_direction(direction_t direction, int floor){
    switch(direction){
    case UP: 
        if (queue_order_above(floor)){
            return UP;
            break;
        }
        if (queue_order_below(floor)){
            return DOWN;
            break;
        }

    case DOWN:
        if(queue_order_below(floor)){
            return DOWN;
            break;
        } 
        if(queue_order_above(floor)){
            return UP;
            break;
        }

    case STANDSTILL:
        if(queue_order_below(floor)){
            return DOWN;
            break;
        } 
        if(queue_order_above(floor)){
            return UP;
            break;
        }
    }
    return STANDSTILL;
}


bool queue_stop_at_floor(direction_t direction, int floor){
    // if not at end floor 
	if((direction == UP) && (floor != 3)){  
	    // if no orders left in direction, stop at floor!   
        if(!queue_order_above(floor)){       
            return true;
        }
		return (queue[BUTTON_CALL_UP][floor] || queue[BUTTON_COMMAND][floor]);
	}
	// if not at end floor 
	else if ((direction == DOWN) && (floor != 0)){  
	    // if no orders left in direction, stop at floor!    
        if(!queue_order_below(floor)) {       
            return true;
        }
        else{   
		    return (queue[BUTTON_CALL_DOWN][floor] || queue[BUTTON_COMMAND][floor]);
        }
	}
	// always stop at end floor 
	return true;                  
}


void queue_clear_queue(void){	   
	int floor = 0; 		
	int button;
	for(; floor < N_FLOORS; floor++){
		for(button = 0; button < N_BUTTON_TYPES; button++){
		    // check for buttons that don't exist 
            if(((floor == 0) && (button == DOWN)) || ((floor == 3) && (button == UP))){
                // do nothing
            }
            else{
			    queue[button][floor] = 0;
			    elev_set_button_lamp(button, floor, false);
		    }
        }
	}
}

bool queue_check_order(elev_button_type_t button, int floor){
	return queue[button][floor];
}

void queue_place_order(elev_button_type_t button,int floor){
	queue[button][floor] = 1;
	elev_set_button_lamp(button, floor, true);
}

// checks buttons and updates queue[][]
void queue_check_buttons(car_state_t state){	
	int floor = 0;
	elev_button_type_t button = 0;

    // listen to all buttons as long as car is not in emergency state
	if(state != CAR_EMG_STOP){
		for(; floor < N_FLOORS; floor++){
			for(button = 0; button < N_BUTTON_TYPES; button++){
			    //only check buttons that actually exict
				if(!((button == BUTTON_CALL_DOWN && floor == 0) || (button == BUTTON_CALL_UP && floor == 3))){
					if (elev_get_button_signal(button, floor)){	
						queue_place_order(button, floor);	
					}
				}
			}
		}
	}
	// if car is in emergency state, only listen to calls from inside the car
	else{
		for(;floor < N_FLOORS; floor++){
			if (elev_get_button_signal(BUTTON_COMMAND, floor)){		
				queue_place_order(BUTTON_COMMAND, floor);	
			}
		}
	}
}


void queue_delete_order(direction_t direction, int floor){
    if(floor == 0){
        queue[BUTTON_CALL_UP][floor] = false;
        elev_set_button_lamp(UP, floor, false); 
    }
    else if(floor == 3){
        queue[BUTTON_CALL_DOWN][floor] = false;
        elev_set_button_lamp(DOWN, floor, false); 
    }
    else	{
    	int button = 0;
    	for(; button < N_BUTTON_TYPES; button++){
    		queue[button][floor] = false;
    		elev_set_button_lamp(button, floor, false);
    	}
    }
    queue[BUTTON_COMMAND][floor] = false;
    elev_set_button_lamp(BUTTON_COMMAND, floor, false);
}





