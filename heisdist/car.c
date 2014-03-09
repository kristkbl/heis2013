#include <time.h>	
#include <unistd.h>
#include <stdio.h>

#include "elev.h"
#include "queue.h" 	
#include "car.h"

//private car variables
unsigned int last_floor;        // holds the last known floor, can be 0-3
direction_t car_direction;      // holds the car's moving direction
unsigned int timer_start;       // used as checksum for the 3 seconds timer for the car door
car_state_t current_state;      // the car's current state
car_event_t event;              // the car's next event
bool EMG_last_state;		// true if FSM comes from EMG, false else.


void car_move(direction_t direction){	
	if(!elev_get_obstruction_signal()){
		if(direction == UP){
			elev_set_speed(SPEED_UP);
		}
	
		else if(direction == DOWN){
			elev_set_speed(SPEED_DOWN);
		}
	}
}

// Stops the car by reversing the motor for BRAKE_TIME microseconds before setting the speed to 0
void car_stop(){
    	//only reverse motor if car is moving	
	if(car_direction != STANDSTILL){ 
			unsigned int current_speed;
			if (car_direction == DOWN){
				current_speed = SPEED_DOWN;	
			}	
			else if (car_direction == UP){
				current_speed = SPEED_UP;
			}
	
			elev_set_speed(-current_speed);
			usleep(BRAKE_TIME);					
			elev_set_speed(0);
	}
}


void door_close(){	
		elev_set_door_open_lamp(false);
}


void door_open(){
	elev_set_door_open_lamp(true);
}

// update Finite State Machine for elevator car, 
car_state_t car_update_state(){
	event = car_get_next_event(current_state, last_floor);  
	switch(current_state){
		case CAR_INVALID: 
			break;
		case CAR_MOVING:
			switch(event){
				case CAR_MOVE:
					//Do nothing, keep moving
					break;

				case CAR_HALT:
					last_floor = elev_get_floor_sensor_signal();
					car_stop();
					door_open();
					timer_start = time(0);	
					elev_set_floor_indicator(last_floor);	
					EMG_last_state = false;					
					queue_delete_order(car_direction, last_floor);		
                 			current_state = CAR_STANDSTILL;				
					break;
					
				case CAR_EMG_STOP:
				    	// only stop, if not already paused due to obstruction
					if(!elev_get_obstruction_signal()){	
						car_stop();
					}
					queue_clear_queue();
					elev_set_stop_lamp(true);
					EMG_last_state = true;
					current_state = CAR_EMG_STOPPED; 				
					break;
				case CAR_NO_EVENT:
					//do nothing
					EMG_last_state = false;
					break;
			}

		case CAR_STANDSTILL:						
			switch(event){
				case CAR_MOVE:
					car_move(car_direction);		
					current_state = CAR_MOVING;
					break;

				case CAR_HALT:
					door_open();
				 	timer_start = time(0); 	
				 	queue_delete_order(car_direction, last_floor);						
					break;
				
				case CAR_EMG_STOP:
					queue_clear_queue();
					elev_set_stop_lamp(true);
					EMG_last_state = true;							
					current_state = CAR_EMG_STOPPED;
					break;
				case CAR_NO_EVENT:
				    	// check for obstruction and restart timer if obstruction detected
                    			if (elev_get_obstruction_signal()){	
						timer_start = time(0); 	
					}		
					// check if DOOR_OPEN_TIME seconds have passed before closing the door			
                    			if (time(0) >= (timer_start + DOOR_OPEN_TIME)){     
						door_close();
					}
					break;
			}
		
		case CAR_EMG_STOPPED:
			switch(event){
				case CAR_MOVE:
					elev_set_stop_lamp(false);
					// only move is no obstruction
					if (!elev_get_obstruction_signal()){
						// close door if open
						door_close();				            
						car_move(car_direction);
					}
					current_state = CAR_MOVING;										
					break;
				case CAR_HALT:
					elev_set_stop_lamp(false);
					door_open();				
					timer_start = time(0);	
					last_floor = elev_get_floor_sensor_signal();
					elev_set_floor_indicator(last_floor);
					EMG_last_state = false;	
					current_state = CAR_STANDSTILL;			
					break;
				case CAR_NO_EVENT:
					//do nothing
					break;
			}
	}
	return current_state;
}
// get next event for elevator car
car_event_t car_get_next_event(car_state_t state, int floor){			
	if (elev_get_stop_signal()){
		return CAR_EMG_STOP;
	}
	switch(state){
	case CAR_MOVING:
	    //if car at defined floor
		if(elev_get_floor_sensor_signal() != -1){       
			floor = elev_get_floor_sensor_signal();  
			//if order in affect at floor   
			if(queue_stop_at_floor(car_direction, floor)){
				return CAR_HALT;
			}
		}
		if (car_handle_obstruction() != CAR_NO_EVENT){
			return car_handle_obstruction();
		}
		
		break;
	case CAR_STANDSTILL:
	    // if orders in other floors, and door closed
        if((queue_get_next_direction(car_direction, floor) != STANDSTILL) && (time(0) >= timer_start + 3)){	
			car_direction = queue_get_next_direction(car_direction, floor);
			return CAR_MOVE;
		}
		// if orders at same floor
		if(queue_check_order(BUTTON_CALL_UP,floor) || queue_check_order(BUTTON_CALL_DOWN,floor) || queue_check_order(BUTTON_COMMAND,floor)){	
			return CAR_HALT;
		}
		return CAR_NO_EVENT;
		break;

    case CAR_EMG_STOPPED:
            // check if any orders placed in queue
    		if((queue_get_next_direction(STANDSTILL, floor) != STANDSTILL)){			
				car_direction = queue_get_next_direction(car_direction, floor);		
				//door_close();				
				return CAR_MOVE;
            }
            // if already at desired floor
            else if(queue_check_order(BUTTON_COMMAND, floor)){	
            		if(elev_get_floor_sensor_signal() != -1){	
            			return CAR_HALT;
            		}
            		if(car_direction == UP){
            			car_direction = DOWN;
            			return CAR_MOVE;
            		}
            		if(car_direction == DOWN){
            			car_direction = UP;
            			return CAR_MOVE;
            		}
            	}

	default:							
		return CAR_NO_EVENT;
		break;
	}
	return CAR_NO_EVENT;
}

// Initialize elevator car
car_state_t car_init(){
	printf("CAR: initializing... \n");
	    if(elev_get_floor_sensor_signal() == -1){	// undefined floor
		elev_set_speed(SPEED_DOWN);
		car_direction = DOWN;
		
		while(elev_get_floor_sensor_signal() == -1){
			// do nothing
		}
		car_stop();
	}
	last_floor = elev_get_floor_sensor_signal();						
	current_state = CAR_STANDSTILL;	
	timer_start = time(0);
	elev_set_floor_indicator(last_floor);							
	return current_state;																						
}

// Check for obstruction and pause car if obstruction
car_event_t car_handle_obstruction(){
	if (elev_get_obstruction_signal()){
		if(!EMG_last_state){
			car_stop();
		}
		while (elev_get_obstruction_signal()){
			if (elev_get_stop_signal()){
				return CAR_EMG_STOP;
			}
			queue_check_buttons(current_state);
		}
		
		if (car_direction == UP){
			elev_set_speed(SPEED_UP);
		}
		if (car_direction == DOWN){
			elev_set_speed(SPEED_DOWN);
		}
	}
	return CAR_NO_EVENT;
}




















	
