#ifndef A2_INVERSE_KINEMATICS_H
#define A2_INVERSE_KINEMATICS_H

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <math.h>
#include <sys/select.h>
#include <sys/time.h>
#include <pthread.h>
#include <mutex>

#include <lcm/lcm-cpp.hpp>
#include <lcm/lcm.h>
#include "lcmtypes/dynamixel_command_list_t.hpp"
#include "lcmtypes/dynamixel_command_t.hpp"
#include "lcmtypes/dynamixel_status_list_t.hpp"
#include "lcmtypes/dynamixel_status_t.hpp"

#include "common/getopt.h"
#include "common/timestamp.h"
// #include "math/math_util.h"
#include "math/fasttrig.h"

#include <iostream>

#include <string>
#include <vector>

using namespace std;

const int NUM_SERVOS = 6;

const string command_channel = "ARM_COMMAND";
const string status_channel = "ARM_STATUS";    

const double d1 = 11.7; // unit = cm
const double d2 = 10;
const double d3 = 10;
const double d4 = 10;
const double d5 = 8.2;

const double SPEED = 0.07;
const double TORQUE = 0.35;

class a2_inverse_kinematics {
public:

    lcm::LCM lcm;

    pthread_t lcm_handler_thread;
    mutex lock;

    vector<double> arm_pos;

    a2_inverse_kinematics();
    void move(double x, double y, double z, double tilt, double rotate, double grip);
    void move(double x, double y, double z, double grip);
    void move_origin(double grip);
};

#endif // A2_INVERSE_KINEMATICS_H
