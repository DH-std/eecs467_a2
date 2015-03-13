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

#include <lcm/lcm.h>
#include "lcmtypes/dynamixel_command_list_t.h"
#include "lcmtypes/dynamixel_command_t.h"
#include "lcmtypes/dynamixel_status_list_t.h"
#include "lcmtypes/dynamixel_status_t.h"

#include "common/getopt.h"
#include "common/timestamp.h"
#include "math/math_util.h"

#define NUM_SERVOS 6

typedef struct state state_t;
struct state
{
    getopt_t *gopt;

    // LCM
    lcm_t *lcm;
    const char *command_channel;
    const char *status_channel;

    pthread_t status_thread;
    pthread_t command_thread;

    double x;
    double y;
    double z;
    double tilt;
};


static void
status_handler (const lcm_recv_buf_t *rbuf,
                const char *channel,
                const dynamixel_status_list_t *msg,
                void *user)
{
    // Print out servo positions
    for (int id = 0; id < msg->len; id++) {
        dynamixel_status_t stat = msg->statuses[id];
        printf ("[id %d]=%6.3f ",id, stat.position_radians);
    }
    printf ("\n");
}

void *
status_loop (void *data)
{
    state_t *state = data;
    dynamixel_status_list_t_subscribe (state->lcm,
                                       state->status_channel,
                                       status_handler,
                                       state);
    const int hz = 15;
    while (1) {
        int status = lcm_handle_timeout (state->lcm, 1000/hz);
        if (status <= 0)
            continue;
    }

    return NULL;
}

void *
command_loop (void *user)
{
    state_t *state = user;
    const int hz = 30;

    dynamixel_command_list_t cmds;
    cmds.len = NUM_SERVOS;
    cmds.commands = (dynamixel_command_t *) calloc (NUM_SERVOS, sizeof(dynamixel_command_t));
 
    cmds.commands[0].position_radians = atan2(state->y, state->x); 
    cmds.commands[1].position_radians = M_PI/2-atan2(state->y, state->x);
    cmds.commands[2].position_radians = atan2(state->y, state->x);
    cmds.commands[3].position_radians = atan2(state->y, state->x);
    cmds.commands[4].position_radians = atan2(state->y, state->x);
    cmds.commands[5].position_radians = atan2(state->y, state->x);

    for (int id = 0; id < NUM_SERVOS; id++) {
        cmds.commands[id].speed = 0.1;
        cmds.commands[id].max_torque = 0.5;
    }

    while (1) {
        for (int id = 0; id < NUM_SERVOS; id++)
            cmds.commands[id].utime = utime_now ();

        dynamixel_command_list_t_publish (state->lcm, state->command_channel, &cmds);
        usleep (1000000/hz);
    }

    free (cmds.commands);
    return NULL;
}

int
main (int argc, char *argv[])
{
    state_t *state = new state_t;
    state->lcm = lcm_create (NULL);
    state->command_channel = "ARM_STATUS";
    state->status_channel = "ARM_COMMAND";

    cout << "x, y, z, tilt: ";
    cin >> state->x >> state->y >> state->z >> state->tilt;

    pthread_create (&state->status_thread, NULL, status_loop, state);
    pthread_create (&state->command_thread, NULL, command_loop, state);

    pthread_join (state->status_thread, NULL);
    pthread_join (state->command_thread, NULL);

    lcm_destroy (state->lcm);
    free (state);

    return 0;
}
