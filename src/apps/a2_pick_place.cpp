#include "a2_pick_place.h"

void a2_pick_place::pick(double x, double y) {
    ik.move(x, y, 7, BIG_OPEN_HAND);
    usleep(1000000);
    ik.move(x, y, 1.5, BIG_OPEN_HAND);
    usleep(250000);
    ik.move(x, y, 1.5, CLOSE_HAND);
    usleep(1000000);
    ik.move_origin(CLOSE_HAND);
    usleep(1000000);
}

void a2_pick_place::place(double x, double y) {
    ik.move(x, y, 7, CLOSE_HAND);
    usleep(1000000);
    ik.move(x, y, 3, CLOSE_HAND);
    usleep(250000);
    ik.move(x, y, 3, SMALL_OPEN_HAND);
    usleep(250000);
    ik.move(x, y, 7, SMALL_OPEN_HAND);
    usleep(250000);
    ik.move_origin(BIG_OPEN_HAND);
    usleep(1000000);
}

void a2_pick_place::move_origin() {
    ik.move_origin(CLOSE_HAND);
}

a2_pick_place::a2_pick_place() {
    // NUM_SERVOS = 6;
    // HAND_SERVO = 5;

    // command_channel = "ARM_COMMAND";
    // status_channel = "ARM_STATUS";    

    // CLOSE_HAND = 1.5;
    // BIG_OPEN_HAND = 0.8;
    // SMALL_OPEN_HAND = 1.3;

    // SPEED = 0.05;
    // TORQUE = 0.35;

    // arm_pos.resize(NUM_SERVOS, 0);


    // lcm.subscribeFunction(status_channel, (void (*)(lcm::ReceiveBuffer*, string&, dynamixel_status_list_t*, void*) )status_handler, (void*) NULL);
    // lcm.subscribeFunction(status_channel, status_handler, (void*) this);

    // pthread_create(&lcm_handler_thread, NULL, lcm_handler, (void*) this);

}

/*
void a2_pick_place::move_hand(double angle) {
    
    dynamixel_command_list_t cmds;
    cmds.len = NUM_SERVOS;
    cmds.commands = vector< dynamixel_command_t >(NUM_SERVOS);

    // Send LCM commands to arm. Normally, you would update positions, etc,
    // but here, we will just home the arm.

    for (int id = 0; id < NUM_SERVOS; id++) {
        if (id != HAND_SERVO) {
            cmds.commands[id].utime = utime_now();
            cmds.commands[id].position_radians = arm_pos[id];
            cmds.commands[id].speed = SPEED;
            cmds.commands[id].max_torque = TORQUE;
        }
        else {
            cmds.commands[id].utime = utime_now();
            cmds.commands[id].position_radians = angle;
            cmds.commands[id].speed = SPEED;
            cmds.commands[id].max_torque = TORQUE;
        }
    }
    // cmds.commands[HAND_SERVO].utime = utime_now();
    // cmds.commands[HAND_SERVO].position_radians = angle;
    // cmds.commands[HAND_SERVO].speed = SPEED;
    // cmds.commands[HAND_SERVO].max_torque = TORQUE;

cout << "before arm_pos[HAND_SERVO] " << arm_pos[HAND_SERVO] << " angle " << angle << endl;

    lcm.publish(command_channel, &cmds);

    
    // check every 1/2 second for 10 times
    int count = 0;
    int hz = 2;
    double error = 0.05;
    // wait till gripper get to position or at most 5 second
    while (count < 10 && (arm_pos[HAND_SERVO] < (1-error)*angle || arm_pos[HAND_SERVO] > (1+error)*angle)) {
        // cout << "##########while arm_pos[HAND_SERVO] " << arm_pos[HAND_SERVO] << " angle " << angle << " count: " << count << endl;
        usleep(1000000/hz);
        count++;
    }

    cout << "after arm_pos[HAND_SERVO] " << arm_pos[HAND_SERVO] << " angle " << angle << " count: " << count << endl;
}


void status_handler(const lcm::ReceiveBuffer* rbuf, const std::string& channel, 
                                   const dynamixel_status_list_t *scan, void *user) {
    vector<double>& arm_pos = ((a2_pick_place*) user)->arm_pos;

    for (int id = 0; id < scan->len; ++id) {
        arm_pos[id] = scan->statuses[id].position_radians;

// cout << "arm_pos[id:" << id << "]=" << arm_pos[id] << " | ";
    }

// cout << endl;
}

void* lcm_handler(void* args) {
    lcm::LCM& lcm = ((a2_pick_place*) args)->lcm;
    int hz = 15;
    while(1){
        lcm.handle();
        usleep(1000000/hz);
    }
    return NULL;
}
*/

/*
void* a2_pick_place::status_loop(void *data) {
    state_t* state = (state_t*) data;
    dynamixel_status_list_t_subscribe (state->lcm,
                                       state->status_channel,
                                       status_handler,
                                       state);
    const int hz = 1;
    while (1) {
        // Set up the LCM file descriptor for waiting. This lets us monitor it
        // until something is "ready" to happen. In this case, we are ready to
        // receive a message.
        int status = lcm_handle_timeout (state->lcm, 1000/hz);
        if (status <= 0)
            continue;

        // LCM has events ready to be processed
    }

    return NULL;
}
*/
/*
void* command_loop(void *user) {
    state_t* state = (state_t*) user;
    const int hz = 30;

    dynamixel_command_list_t cmds;
    cmds.len = NUM_SERVOS;
    cmds.commands = calloc (NUM_SERVOS, sizeof(dynamixel_command_t));

    while (1) {
        // Send LCM commands to arm. Normally, you would update positions, etc,
        // but here, we will just home the arm.
        for (int id = 0; id < NUM_SERVOS; id++) {
            if (getopt_get_bool (state->gopt, "idle")) {
                cmds.commands[id].utime = utime_now ();
                cmds.commands[id].position_radians = 0.0;
                cmds.commands[id].speed = 0.0;
                cmds.commands[id].max_torque = 0.0;
            }
            else {
                // home servos slowly
                cmds.commands[id].utime = utime_now ();
                cmds.commands[id].position_radians = 0.0;
                cmds.commands[id].speed = 0.05;
                cmds.commands[id].max_torque = 0.35;
            }
        }
        dynamixel_command_list_t_publish (state->lcm, state->command_channel, &cmds);

        usleep (1000000/hz);
    }

    free (cmds.commands);

    return NULL;
}
*/
