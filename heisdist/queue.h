#ifndef __INCLUDE_QUEUE_H__
#define __INCLUDE_QUEUE_H__

#include <stdbool.h>

#include "car.h"

/** queue system, 1 for order in effect, 0 if no order is placed
    @param N_BUTTON_TYPES is 3 (UP, DOWN og COMMAND), N_FLOORS is 4 (floors 0 to 3)
    used by all queue functions*/
bool queue[N_BUTTON_TYPES][N_FLOORS];					

/** clears queue if in EMG_STOP */
void queue_clear_queue(void);

/** sets queue parametres to 1 if order is placed */
void queue_place_order(elev_button_type_t button, int floor);

/** checks queue at param, returnrs 1 if order in effect, else 0*/
bool queue_check_order(elev_button_type_t button, int floor);

/** uses queue to find next direction
    @param current_direction, and last_floor */
direction_t queue_get_next_direction(direction_t direction, int floor);

/** sets queue parametres to 0 if order is handled */
void queue_delete_order(direction_t direction, int floor);

/** uses direction and queue to check if car should stop at current floor */
bool queue_stop_at_floor(direction_t direction, int floor);

/** checks for orders, and uses queue_place_order(...) to place order in queue. */
void queue_check_buttons(car_state_t state);

/** uses last_floor to check for orders above */
bool queue_order_above(int floor);

/** uses last_floor to check for orders below */
bool queue_order_below(int floor);


#endif // #ifndef __INCLUDE_QUEUE_H__




