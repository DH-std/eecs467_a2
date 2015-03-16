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
#include <gsl/gsl_blas.h>

#include "lcmtypes/dynamixel_command_list_t.hpp"
#include "lcmtypes/dynamixel_command_t.hpp"
#include "lcmtypes/dynamixel_status_list_t.hpp"
#include "lcmtypes/dynamixel_status_t.hpp"

#include "common/getopt.h"
#include "common/timestamp.h"
#include "math/gsl_util_matrix.h"
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
    const char *status_channel;

    pthread_t status_thread;

    double x;
    double y;
    double z;

    double base_height;
    double upper_arm;
    double lower_arm;
    double palm_length;
    double finger_length;
};

void radiansToXYZ(double pos[], state_t *state) {
    fasttrig_init();
    double d = state->palm_length+state->finger_length;
    double a_f2w[] = {fcos(pos[3]), -1.*fsin(pos[3]), 0.,  d*fsin(pos[3]),
                      fsin(pos[3]),     fcos(pos[3]), 0., -d*fcos(pos[3]),
                                0.,               0., 1.,              0.,
                                0.,               0., 0.,              1.};

    d = state->lower_arm;
    double a_w2e[] = {fcos(pos[2]), -1.*fsin(pos[2]), 0.,  d*fsin(pos[2]),
                      fsin(pos[2]),     fcos(pos[2]), 0., -d*fcos(pos[2]),
                                0.,              0.,  1.,              0.,
                                0.,              0.,  0.,              1.};

    d = state->upper_arm;
    double a_e2s[] = {fcos(pos[1]), -1.*fsin(pos[1]), 0.,  d*fsin(pos[1]),
                      fsin(pos[1]),     fcos(pos[1]), 0., -d*fcos(pos[1]),
                                0.,               0., 1.,              0.,
                                0.,               0., 0.,              1.};

    d = state->base_height;
    double a_s2b[] = {fcos(pos[0]),  0., -1.*fsin(pos[0]), 0.,
                      fsin(pos[0]),  0.,     fcos(pos[0]), 0.,
                                0., -1.,               0.,  d,
                                0.,  0.,               0., 1.};

    double pt_f[] = {0., 0., 0., 1.};
    double pt_b[] = {0., 0., 0., 0.};
    double temp[] = {0., 0., 0., 0.,
                     0., 0., 0., 0.,
                     0., 0., 0., 0.,
                     0., 0., 0., 0.};
    double tem2[] = {0., 0., 0., 0.,
                     0., 0., 0., 0.,
                     0., 0., 0., 0.,
                     0., 0., 0., 0.};
 
    gsl_matrix_view Afw = gsl_matrix_view_array(a_f2w, 4, 4);
    gsl_matrix_view Awe = gsl_matrix_view_array(a_w2e, 4, 4);
    gsl_matrix_view Aes = gsl_matrix_view_array(a_e2s, 4, 4);
    gsl_matrix_view Asb = gsl_matrix_view_array(a_s2b, 4, 4);
    gsl_matrix_view Pf  = gsl_matrix_view_array(pt_f,  4, 1);
    gsl_matrix_view Pb  = gsl_matrix_view_array(pt_b,  4, 1);
    gsl_matrix_view tmp = gsl_matrix_view_array(temp,  4, 4);
    gsl_matrix_view tm2 = gsl_matrix_view_array(tem2,  4, 4);

    gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.0, &Asb.matrix, 
                   &Aes.matrix, 0.0, &tmp.matrix);
    gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.0, &tmp.matrix, 
                   &Awe.matrix, 0.0, &tm2.matrix);
    gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.0, &tm2.matrix, 
                   &Afw.matrix, 0.0, &tmp.matrix);
    gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.0, &tmp.matrix, 
                   &Pf.matrix, 0.0, &Pb.matrix);

    state->x = pt_b[0];
    state->y = pt_b[1];
    state->z = pt_b[2];

    cout << pt_b[0] << " " << pt_b[1] << " " << pt_b[2] << " " << pt_b[3] << endl;
}

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
            assert(msg->len == NUM_SERVOS);

            double pos[NUM_SERVOS];
            for (int id = 0; id < msg->len; id++) {
                pos[id] = msg->statuses[id].position_radians;
            }
            radiansToXYZ(pos, state);
        }
};

void *
status_loop (void *user)
{
    state_t *state = (state_t *)user;
    while(state->lcm->handle() == 0);
    return NULL;
}

int
main (int argc, char *argv[])
{
    state_t *state = new state_t;
    state->lcm = new lcm::LCM;
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

    pthread_create (&state->status_thread, NULL, status_loop, state);
    pthread_join (state->status_thread, NULL);

    delete state->lcm;
    free (state);

    return 0;
}
