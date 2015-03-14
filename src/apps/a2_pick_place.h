#ifndef A2_PICK_PLACE_H
#define A2_PICK_PLACE_H

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <math.h>
#include <sys/select.h>
#include <sys/time.h>
#include <pthread.h>

#include <lcm/lcm-cpp.hpp>
#include <lcm/lcm.h>
#include "lcmtypes/dynamixel_command_list_t.hpp"
#include "lcmtypes/dynamixel_command_t.hpp"
#include "lcmtypes/dynamixel_status_list_t.hpp"
#include "lcmtypes/dynamixel_status_t.hpp"

#include "common/getopt.h"
#include "common/timestamp.h"
// #include "math/math_util.h"

#include <iostream>

#include <string>
#include <vector>

using namespace std;

const int NUM_SERVOS = 6;
const int HAND_SERVO = 5;

const string command_channel = "ARM_COMMAND";
const string status_channel = "ARM_STATUS";    

const double CLOSE_HAND = 1.5;
const double BIG_OPEN_HAND = 0.8;
const double SMALL_OPEN_HAND = 1.3;

const double SPEED = 0.05;
const double TORQUE = 0.35;

class a2_pick_place {
public:

    lcm::LCM lcm;

    pthread_t lcm_handler_thread;
    // pthread_mutex_t lock;

    // static int NUM_SERVOS;
    // static int HAND_SERVO;

    // static string command_channel;
    // static string status_channel;    

    // static double CLOSE_HAND;
    // static double BIG_OPEN_HAND;
    // static double SMALL_OPEN_HAND;

    // static double SPEED;
    // static double TORQUE;

    vector<double> arm_pos;

    // inverse kinematics

    void pick(double x, double y, double z);
    void place(double x, double y, double z);

    void move_hand(double angle);

    // void status_handler(const lcm::ReceiveBuffer* rbuf, const std::string& channel, 
    //                                const dynamixel_status_list_t *scan, void *user);
    // void* lcm_handler(void* args);

    a2_pick_place();
};

#endif // A2_PICK_PLACE_H
