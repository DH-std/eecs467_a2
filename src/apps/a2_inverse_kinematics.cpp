#include "a2_inverse_kinematics.h"

void status_handler(const lcm::ReceiveBuffer* rbuf, const std::string& channel, 
                                   const dynamixel_status_list_t *scan, void *user) {
    vector<double>& arm_pos = ((a2_inverse_kinematics*) user)->arm_pos;
    mutex& lock = ((a2_inverse_kinematics*) user)->lock;
    lock.lock();
    for (int id = 0; id < scan->len; ++id) {
        arm_pos[id] = scan->statuses[id].position_radians;
    }
    lock.unlock();
}

void* lcm_handler(void* args) {
    lcm::LCM& lcm = ((a2_inverse_kinematics*) args)->lcm;
    int hz = 15;
    while(1){
        lcm.handle();
        usleep(1000000/hz);
    }
    return NULL;
}

a2_inverse_kinematics::a2_inverse_kinematics() {
    arm_pos.resize(NUM_SERVOS, 0);

    // lcm.subscribeFunction(status_channel, (void (*)(lcm::ReceiveBuffer*, string&, dynamixel_status_list_t*, void*) )status_handler, (void*) NULL);
    lcm.subscribeFunction(status_channel, status_handler, (void*) this);

    pthread_create(&lcm_handler_thread, NULL, lcm_handler, (void*) this);

}

void a2_inverse_kinematics::move(double x, double y, double z, double tilt, double rotate, double grip) {
    if (z >= 15) {
        cout << "z can't go that high z = " << z << endl;
        return;
    }
    if (x >= 25 || y >= 25) {
        cout << "x or y can't go that far, x = " << x << ", y = " << y << endl;
        return;
    }

    dynamixel_command_list_t cmds;
    cmds.len = NUM_SERVOS;
    cmds.commands = vector<dynamixel_command_t>(NUM_SERVOS); 
    
    double error = 0.15; // about 8.5 degree error
    int count = 0;
    double hz = 2;
    int num_count = 20; // default wait at most 10 sec

    double theta[NUM_SERVOS];

    if (x == 0 && y == 0) {
        theta[0] = 0;
        theta[1] = 0;
        theta[2] = 0;
        theta[3] = tilt;
    }
    else {
        fasttrig_init();

        double h = d5 + z;
        double R = sqrt(x*x+y*y)-1;
        double M2 = (x*x+y*y)+(d4+h-d1)*(d4+h-d1); 

        theta[0] = atan2(y, x); 

        theta[1] = M_PI/2.0 
                - atan2(d4+h-d1,R) 
                - facos((-d3*d3+d2*d2+M2)/(2*d2*sqrt(M2)));

        theta[2] = M_PI
                - facos((-M2+d2*d2+d3*d3)/(2*d2*d3));

        theta[3] = M_PI 
                - theta[1] 
                - theta[2]
                + tilt;
    }

    theta[4] = rotate;
    theta[5] = grip;

cout << "target ";
    for (int id = 0; id < NUM_SERVOS; ++id) {
        cmds.commands[id].utime = utime_now();
        cmds.commands[id].position_radians = theta[id];
        cmds.commands[id].speed = SPEED;
        cmds.commands[id].max_torque = TORQUE;

cout << "theta[" << id << "]=" << theta[id] << " | ";
    }

cout << endl;
  
    lcm.publish(command_channel, &cmds);

    // wait at most 10 sec or until arm in position
    lock.lock();
    while (count < num_count && 
                        (   fabs(arm_pos[0] - theta[0]) > error || 
                            fabs(arm_pos[1] - theta[1]) > error ||  
                            fabs(arm_pos[2] - theta[2]) > error || 
                            fabs(arm_pos[3] - theta[3]) > error || 
                            fabs(arm_pos[4] - theta[4]) > error ||
                            fabs(arm_pos[5] - theta[5]) > error )) {
        lock.unlock();
        usleep(1000000/hz);
        lock.lock();
        count++;
        cout << "reached ";
        for (int id = 0; id < NUM_SERVOS; ++id) {
    cout << "arm[" << id << "]=" << arm_pos[id] << " | ";
        }
        cout << endl;
    }
    lock.unlock();

}

void a2_inverse_kinematics::move(double x, double y, double z, double grip) {
    move(x, y, z, 0, -M_PI/2.0, grip);
}

void a2_inverse_kinematics::move_origin(double grip) {
    move(0, 0, 0, 0, -M_PI/2.0, grip);
}

void a2_inverse_kinematics::limp() {
// cout << " limp" << endl;

    dynamixel_command_list_t cmds;
    cmds.len = NUM_SERVOS;
    cmds.commands = vector<dynamixel_command_t>(NUM_SERVOS); 

// cout << " goign to set commands" << endl;

    for (int id = 0; id < NUM_SERVOS; ++id) {
// cout << "b4 u time" << endl;


        cmds.commands[id].utime = utime_now();

// cout << "after u time" << endl;

        cmds.commands[id].position_radians = 0;
        cmds.commands[id].speed = 0;
        cmds.commands[id].max_torque = 0;
    }

// cout << " limp going to pub" << endl;

    lcm.publish(command_channel, &cmds);

  // cout << " limp pub" << endl;  

}