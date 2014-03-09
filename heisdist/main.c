// REPLACE THIS FILE WITH YOUR OWN CODE.
// READ ELEV.H FOR INFORMATION ON HOW TO USE THE ELEVATOR FUNCTIONS.

#include <stdio.h>

#include "elev.h"
#include "queue.h"
#include "car.h"




int main(){

    // Initialize hardware
    if (!elev_init()){
        printf(__FILE__ ": Unable to initialize elevator hardware\n");
        return 1;
    }
	
	car_state_t state = car_init();
	    
	while(1){
		queue_check_buttons(state);
        state = car_update_state();		
	}
 
    return 0;
}
