
#include <string>
#include <iostream>
#include "a2_pick_place.h"


#include <lcm/lcm-cpp.hpp>
#include <lcm/lcm.h>
#include "lcmtypes/dynamixel_command_list_t.hpp"
#include "lcmtypes/dynamixel_command_t.hpp"
#include "lcmtypes/dynamixel_status_list_t.hpp"
#include "lcmtypes/dynamixel_status_t.hpp"

lcm::LCM lcm_main;

// pthread_t lcm_handler_thread;

int main() {

    a2_pick_place pp;
    pp.move_origin();
    
    pp.pick(5, 5);
    pp.place(-5,-5);

}