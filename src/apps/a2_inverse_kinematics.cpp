#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <math.h>
#include <sys/select.h>
#include <sys/time.h>
#include <pthread.h>
#include <cstdlib>
#include <iostream>
#include <string>
#include <lcm/lcm-cpp.hpp>
#include <vector>

#include "lcmtypes/dynamixel_command_list_t.hpp"
#include "lcmtypes/dynamixel_command_t.hpp"
#include "lcmtypes/dynamixel_status_list_t.hpp"
#include "lcmtypes/dynamixel_status_t.hpp"

#include "common/getopt.h"
#include "common/timestamp.h"
//#include "math/math_util.h"
#include "math/fasttrig.h"

#define NUM_SERVOS 6

using namespace std;

typedef struct state state_t;
struct state
{
    getopt_t *gopt;

    // LCM
    lcm::LCM *lcm;
    const char *command_channel;
    const char *status_channel;

    pthread_t status_thread;
    pthread_t command_thread;

    double x;
    double y;
    double z;
    double tilt;

    double base_height;
    double upper_arm;
    double lower_arm;
    double palm_length;
    double finger_length;
};

class lcmHandler{
    private: 
        state_t *state;

    public:
        lcmHandler(state_t *s) : state(s) {}

        void handleStatus(const lcm::ReceiveBuffer *rbuf,
                          const std::string& channel,
                          const dynamixel_status_list_t *msg)
        {
            assert(channel == "ARM_STATUS");
            for (int id = 0; id < msg->len; id++) {
                dynamixel_status_t stat = msg->statuses[id];
              //  printf ("[id %d]=%6.3f ",id, stat.position_radians);
            }
            //printf ("\n");
        }
};

void *
status_loop (void *user)
{
    state_t *state = (state_t *)user;
    cout << "status looping maybe" << endl;
    while(state->lcm->handle() == 0);
    return NULL;
}

void *
command_loop (void *user)
{
    state_t *state = (state_t *)user;
    const double hz = 0.2;

    dynamixel_command_list_t cmds;
    cmds.len = NUM_SERVOS;
    cmds.commands = vector<dynamixel_command_t>(NUM_SERVOS); 
    fasttrig_init();

    double d1 = state->base_height;
    double d2 = state->upper_arm;
    double d3 = state->lower_arm;
    double d4 = state->palm_length;
    double h = state->finger_length+state->z;
    double x = state->x;
    double y = state->y;
    double r = sqrt(x*x+y*y);
    double m2 = x*x+y*y+(d4+h-d1)*(d4+h-d1); 

    cmds.commands[0].position_radians = atan2(y, x); 
    cmds.commands[1].position_radians = M_PI/2 
        - atan2(d4+h-d1,r) - facos((-d3*d3+d2*d2+m2)/(2*d2*sqrt(m2)));
    cmds.commands[2].position_radians = M_PI/2
        - facos((-m2+d2*d2+d3*d3)/(2*d2*d3));
    cmds.commands[3].position_radians = M_PI 
        - cmds.commands[1].position_radians 
        - cmds.commands[2].position_radians
        - state->tilt;
    cmds.commands[4].position_radians = 0;
    cmds.commands[5].position_radians = 0;

    for (int id = 0; id < NUM_SERVOS; id++) {
        cmds.commands[id].speed = 0.2;
        cmds.commands[id].max_torque = 0.7;
        cout << cmds.commands[id].position_radians << " ";
    }
    cout << endl;

    while (1) {

        cout << "lol" << endl; 
        for (int id = 0; id < NUM_SERVOS; id++)
            cmds.commands[id].utime = utime_now ();

        state->lcm->publish(state->command_channel, &cmds);
        usleep (1000000/hz);
    }

    return NULL;
}

int
main (int argc, char *argv[])
{
    state_t *state = new state_t;
    state->lcm = new lcm::LCM;
    state->command_channel = "ARM_COMMAND";
    state->status_channel = "ARM_STATUS";
    state->base_height = 11.7; // unit = cm
    state->upper_arm = 10;
    state->lower_arm = 10;
    state->palm_length = 10;
    state->finger_length = 8.2;

    lcmHandler handler(state);
    state->lcm->subscribe(state->status_channel,
                          &lcmHandler::handleStatus,
                          &handler);

    cout << "x, y, z, tilt: ";
    cin >> state->x >> state->y >> state->z >> state->tilt;

    pthread_create (&state->status_thread, NULL, status_loop, state);
    pthread_create (&state->command_thread, NULL, command_loop, state);

    pthread_join (state->command_thread, NULL);
    pthread_join (state->status_thread, NULL);

    delete state->lcm;
    free (state);

    return 0;
}
