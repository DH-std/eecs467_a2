
#include <string>
#include <iostream>
#include "a2_inverse_kinematics.h"

int main() {

    a2_inverse_kinematics ik;
    cout << "going to move origin" << endl;
    ik.move_origin(0);
    cout << "moved to origin" << endl;
    usleep(1000000);
    cout << "going to move 10,0,5" << endl;
    ik.move(10, 0, 5, 0, 0, 0, false);
    cout << "moved 10,0,5" << endl;
    usleep(1000000);
}