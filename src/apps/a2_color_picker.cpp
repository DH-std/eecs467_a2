
#include <stdio.h>
#include <gtk/gtk.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <signal.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <limits>
#include <math.h>

#include <lcm/lcm.h>

#include "vx/vx.h"
#include "vx/vx_util.h"
#include "vx/vx_remote_display_source.h"
#include "vx/vxo_drawables.h"
#include "vx/gtk/vx_gtk_display_source.h"

#include "common/getopt.h"
#include "common/timestamp.h"
#include "common/zarray.h"

#include "imagesource/image_u32.h"
#include "imagesource/image_util.h"
#include "imagesource/image_source.h"
#include "imagesource/image_convert.h"

#include <string>
#include <stack>

using namespace std;

static int verbose = 0;
const uint8_t PURPLE_R = 102;
const uint8_t PURPLE_G = 51;
const uint8_t PURPLE_B = 153;
const uint32_t PURPLE_RGB = (0xff) << 24 | (PURPLE_B & 0xff) << 16 | (PURPLE_G & 0xff) << 8 | PURPLE_R;
const double scale_width = 100;

string file_name = "HSV.txt";
string imfile_name = "screenshot.pmm";

struct HSV_extreme {
    float min_H, max_H, min_S, max_S, min_V, max_V;

    HSV_extreme() : 
            min_H(numeric_limits<float>::max()), max_H(-numeric_limits<float>::max()), 
            min_S(numeric_limits<float>::max()), max_S(-numeric_limits<float>::max()),
            min_V(numeric_limits<float>::max()), max_V(-numeric_limits<float>::max()) {}

    HSV_extreme(float Hi, float Ha, float Si, float Sa, float Vi, float Va) :
            min_H(Hi), max_H(Ha), min_S(Si), max_S(Sa), min_V(Vi), max_V(Va) {}
};

struct RGB_pixel {
    int x, y;
    int R, G, B;

    RGB_pixel() : x(0), y(0), R(0), G(0), B(0) {}
    RGB_pixel(int x_in, int y_in, int R_in, int G_in, int B_in) : 
            x(x_in), y(y_in), R(R_in), G(G_in), B(B_in) {}
};

typedef struct {
    vx_application_t app;
    vx_event_handler_t veh;

    int running;

    getopt_t * gopt;
    char * url;
    image_source_t *isrc;


    pthread_t loop_thread;

    pthread_mutex_t layer_mutex;

    vx_world_t* vw;
    zhash_t* layer_map;

    vx_mouse_event_t last_mouse_event;

    int mask_left, mask_right, mask_top, mask_bot;

    image_u32_t* im;
    float* im_hsv;
    HSV_extreme cur_HSV;

    stack<image_u32_t*> past_im;
    stack<HSV_extreme> past_HSV;

} state_t;

static state_t * global_state;


// http://lolengine.net/blog/2013/01/13/fast-rgb-to-hsv
static void RGB2HSV(uint8_t r, uint8_t g, uint8_t b,
                    float &h, float &s, float &v) {
    float R = r/255.;
    float G = g/255.;
    float B = b/255.;

    float K = 0.f;

    if (G < B)
    {
        swap(G, B);
        K = -1.f;
    }

    if (R < G)
    {
        swap(R, G);
        K = -2.f / 6.f - K;
    }

    float chroma = R - std::min(G, B);
    h = fabs(K + (G - B) / (6.f * chroma + 1e-20f));
    s = chroma / (R + 1e-20f);
    v = R;
}

static void display_finished(vx_application_t * app, vx_display_t * disp)
{
    state_t* state = (state_t*) app->impl;
    pthread_mutex_lock(&state->layer_mutex);

    vx_layer_t * layer = NULL;

    // store a reference to the world and layer that we associate with each vx_display_t
    zhash_remove(state->layer_map, &disp, NULL, &layer);

    vx_layer_destroy(layer);

    pthread_mutex_unlock(&state->layer_mutex);
}

static void display_started(vx_application_t * app, vx_display_t * disp)
{
    state_t* state = (state_t*) app->impl;

    vx_layer_t * layer = vx_layer_create(state->vw);
    vx_layer_set_display(layer, disp);
    vx_layer_add_event_handler(layer, &state->veh);

    pthread_mutex_lock(&state->layer_mutex);
    // store a reference to the world and layer that we associate with each vx_display_t
    zhash_put(state->layer_map, &disp, &layer, NULL, NULL);
    pthread_mutex_unlock(&state->layer_mutex);
}

void draw_im() {
    image_u32_t*& im = global_state->im;
    vx_world_t*& vw = global_state->vw;

    if (im != NULL) {
        vx_object_t* vim = vxo_image_from_u32(im, VXO_IMAGE_FLIPY, VX_TEX_MIN_FILTER | VX_TEX_MAG_FILTER);

        const double scale = scale_width/im->width;
        vx_buffer_add_back(vx_world_get_buffer (vw, "image"),
                            vxo_chain (vxo_mat_scale3 (scale, scale, 1.0),
                                       vxo_mat_translate3 (-im->width/2., -im->height/2., 0.),
                                       vim));
        vx_buffer_swap(vx_world_get_buffer(vw, "image"));
    }
    else {
        cout << "Image is null" << endl;
        exit(-1);
    }
}

static void write_HSV_extreme() {
    ostringstream text_string;

    text_string << "<<#ff0000,serif>>" <<
            "H min: " << global_state->cur_HSV.min_H << " H max: " << global_state->cur_HSV.max_H <<
            "\nS min: " << global_state->cur_HSV.min_S << " S max: " << global_state->cur_HSV.max_S << 
            "\nV min: " << global_state->cur_HSV.min_V << " V max: " << global_state->cur_HSV.max_V;
    vx_object_t* text = vxo_text_create(VXO_TEXT_ANCHOR_TOP_LEFT, text_string.str().c_str()); 
    vx_buffer_add_back(vx_world_get_buffer (global_state->vw, "HSV Extreme"), vxo_pix_coords(VX_ORIGIN_TOP_LEFT, text));
    vx_buffer_swap (vx_world_get_buffer (global_state->vw, "HSV Extreme"));
}

static int touch_event (vx_event_handler_t * vh, vx_layer_t * vl, vx_camera_pos_t * pos, vx_touch_event_t * mouse)
{
    return 0;
}

static int mouse_event (vx_event_handler_t * vh, vx_layer_t * vl, vx_camera_pos_t * pos, vx_mouse_event_t * mouse)
{
    state_t* state = (state_t*) vh->impl;
    image_u32_t*& im = state->im;
    float*& im_hsv = state->im_hsv;
    HSV_extreme& HSV = state->cur_HSV;

    const int stride = state->im->stride;

    if ((mouse->button_mask & VX_BUTTON1_MASK) &&
        !(state->last_mouse_event.button_mask & VX_BUTTON1_MASK)) {

        vx_ray3_t ray;
        vx_camera_pos_compute_ray (pos, mouse->x, mouse->y, &ray);

        double ground[3];
        vx_ray3_intersect_xy (&ray, 0, ground);

        int click_x = ground[0]*im->width/scale_width + im->width/2;
        int click_y = -ground[1]*im->width/scale_width + im->height/2;

printf ("\nMouse clicked at coords: [%8.3f, %8.3f] \n"
        "Ground clicked at coords: [%6.3f, %6.3f] \n"
        "Pixel clicked at coords: [%i, %i] \n", 
        mouse->x, mouse->y, ground[0], ground[1], click_x, click_y);

        // check if click in the bound
        if (click_y < state->mask_top || click_y > state->mask_bot || 
                click_x < state->mask_left || click_x > state->mask_right) {
            
            cout << "Please click within the bound" << endl;
            return -1;
        }

        if (im->buf[click_y*stride+click_x] == PURPLE_RGB) {
            cout << "Please don't click on purple" << endl;
            return -1;
        }


        float H = im_hsv[3*(click_y*stride+click_x) + 0];
        float S = im_hsv[3*(click_y*stride+click_x) + 1];
        float V = im_hsv[3*(click_y*stride+click_x) + 2];

cout << "H : " << H << " S : " << S << " V : " << V << endl;
cout << "R : " << (im->buf[click_y*stride+click_x] & 0xff) << " G : " << (im->buf[click_y*stride+click_x] >> 8 & 0xff) << " B : " << (im->buf[click_y*stride+click_x] >> 16 & 0xff) << endl;

        const float range = 0.15;
        float H_u = (1+range)*H;
        float S_u = (1+range)*S;
        float V_u = (1+range)*V;

        float H_l = (1-range)*H;
        float S_l = (1-range)*S;
        float V_l = (1-range)*V;

        // save current image and HSV extremes
        state->past_im.push(image_u32_copy(im));
        state->past_HSV.push(HSV);

        // loop through image
        for (int y = state->mask_top; y <= state->mask_bot; ++y) {
            for (int x = state->mask_left; x <= state->mask_right; ++x) {
                int idx = y*stride+x;

                float cur_H = im_hsv[3*idx + 0];
                float cur_S = im_hsv[3*idx + 1];
                float cur_V = im_hsv[3*idx + 2];

                uint32_t cur_RGB = im->buf[idx + 0];
                // uint8_t cur_R = (uint8_t) (im->buf[idx + 0] & 0xff);
                // uint8_t cur_G = (uint8_t) ((im->buf[idx + 0] >> 8) & 0xff);
                // uint8_t cur_B = (uint8_t) ((im->buf[idx + 0] >> 16) & 0xff);

                // if similar rgb to clicked HSV
                if (cur_H <= H_u && cur_H >= H_l  
                        && cur_S <= S_u && cur_S >= S_l 
                        && cur_V <= V_u && cur_V >= V_l
                        && cur_RGB != PURPLE_RGB) {
                        // !(cur_R == PURPLE_R && cur_G == PURPLE_G && cur_B == PURPLE_B)) {

                    im->buf[idx + 0] = PURPLE_RGB;
                    // im->buf[idx + 0] = PURPLE_R;
                    // im->buf[idx + 1] = PURPLE_G;
                    // im->buf[idx + 2] = PURPLE_B;

                    if (cur_H < HSV.min_H) {
                        HSV.min_H = cur_H;
                    }
                    if (cur_H > HSV.max_H) {
                        HSV.max_H = cur_H;
                    }
                    if (cur_S < HSV.min_S) {
                        HSV.min_S = cur_S;
                    }
                    if (cur_S > HSV.max_S) {
                        HSV.max_S = cur_S;
                    }
                    if (cur_V < HSV.min_V) {
                        HSV.min_V = cur_V;
                    }
                    if (cur_V > HSV.max_V) {
                        HSV.max_V = cur_V;
                    }
                }
            }
        }

        write_HSV_extreme();
        draw_im();
    }
    // else {
    //     cout << "nth happen cur mouse: " << (mouse->button_mask & VX_BUTTON1_MASK) << " last mouse " << (state->last_mouse_event.button_mask & VX_BUTTON1_MASK)  << endl;
    // }

    // store previous mouse event to see if the user *just* clicked or released
    state->last_mouse_event = *mouse;

    return 0;
}

static int key_event (vx_event_handler_t * vh, vx_layer_t * vl, vx_key_event_t * key)
{
    state_t* state = (state_t*) vh->impl;

    if (!key->released) {
        if (key->key_code == 's' || key->key_code == 'S') {
            if (!state->past_HSV.empty()){
                HSV_extreme& cur_HSV_extremes = state->cur_HSV;

                ofstream HSV_file(file_name);

                HSV_file << cur_HSV_extremes.min_H << " " << cur_HSV_extremes.max_H << " "
                        << cur_HSV_extremes.min_S << " " << cur_HSV_extremes.max_S << " "
                        << cur_HSV_extremes.min_V << " " << cur_HSV_extremes.max_V << " " << endl;;

                HSV_file.close();
                cout << "file saved to " + file_name << endl;
            }
            else {
                cout << "Nothing to save" << endl;
            }

        } 
        else if (key->key_code == VX_KEY_DEL) {

            if (!state->past_im.empty()) {
                cout << "delete last click" << endl;
                image_u32_destroy(state->im);

                state->im = state->past_im.top();
                state->cur_HSV = state->past_HSV.top();

                state->past_im.pop();
                state->past_HSV.pop();

                write_HSV_extreme();
                draw_im();
            }
            else {
                cout << "Nothing to delete" << endl;
            }

        }
        else if (key->key_code == 'p' || key->key_code == 'P') {
            if (!image_u32_write_pnm (state->im, imfile_name.c_str())) {
                cout << "image saved to " + imfile_name << endl;
            }
            else {
                cout << "error saving image to " + imfile_name << endl;
            }

        } 
        else if (key->key_code == VX_KEY_ESC) {
            exit(0);
        }
    }

    return 0;
}

static void nodestroy (vx_event_handler_t * vh)
{
    // do nothing, since this event handler is statically allocated.
}

// static void sig_handler(int signum)
// {
//     switch (signum)
//     {
//         case SIGINT:
//         case SIGQUIT:
//             global_state->running = 0;
//             break;
//         default:
//             break;
//     }
// }

image_u32_t* get_camera_frame() {

    state_t* state = global_state;

    image_u32_t* im = NULL;

    zarray_t *urls = image_source_enumerate();

    printf("Cameras:\n");
    for (int i = 0; i < zarray_size(urls); i++) {
        char *url;
        zarray_get(urls, i, &url);
        printf("  %3d: %s\n", i, url);
    }

    if (zarray_size(urls) == 0) {
        printf("No cameras found.\n");
        exit(-1);
    }
    zarray_get(urls, 0, &state->url);

    state->isrc = image_source_open(state->url);

    if (state->isrc == NULL) {
        printf("Unable to open device %s\n", state->url);
        exit(-1);
    }

    image_source_t *isrc = state->isrc;

    if (isrc->start(isrc)) {
        cout << "can't start image source" << endl;
        exit(-1);
    }

    image_source_data_t isdata;

    int res = isrc->get_frame(isrc, &isdata);
    if (!res) {
        im = image_convert_u32(&isdata);
    }
    else {
        isrc->stop(isrc);
        cout << "can't get frame " << res << endl;
        exit(-1);
    }

    isrc->release_frame(isrc, &isdata);
    return im;
}

static void init_state(int argc, char ** argv) {
    gdk_threads_init();
    gdk_threads_enter();

    gtk_init(&argc, &argv);

    vx_global_init();

    state_t* state = new state_t;
    global_state = state;

    state->running = 1;

    state->gopt = getopt_create();

    state->app.display_finished = display_finished;
    state->app.display_started = display_started;
    state->app.impl = state;

    state->veh.dispatch_order = -1; // what is this??
    state->veh.touch_event = touch_event;
    state->veh.mouse_event = mouse_event;
    state->veh.key_event = key_event;
    state->veh.destroy = nodestroy;
    state->veh.impl = state;

    state->vw = vx_world_create();
    state->layer_map = zhash_create(sizeof(vx_display_t*), sizeof(vx_layer_t*), zhash_ptr_hash, zhash_ptr_equals);

    pthread_mutex_init(&state->layer_mutex, NULL);

    // signal(SIGINT, sig_handler); // what does this do?

    getopt_add_bool(state->gopt, 'h', "help", 0, "Show this help");
    getopt_add_bool(state->gopt, 'v', "verbose", 0, "Show extra debugging output");
    getopt_add_string(state->gopt, 'f', "file", "NULL", "Input file to pick color");
    getopt_add_string(state->gopt, 'o', "outfile", "NULL", "Output HSV filename");
    getopt_add_string(state->gopt, 'i', "outimage", "NULL", "Output image filename");
    getopt_add_string(state->gopt, 'm', "mask", "NULL", "Mask file");
    getopt_add_bool (state->gopt, '\0', "stay-open", 0, "Stay open after gtk exits to continue handling remote connections");

    if (!getopt_parse(state->gopt, argc, argv, 0) ||
        getopt_get_bool(state->gopt,"help")) {
        getopt_do_usage(state->gopt);
        exit(-1);
    }

    if (!strcmp(getopt_get_string(state->gopt, "mask"), "NULL")) {
        cout << "Please input the mask file with -m" << endl;
        exit(-1);
    }
    
    if (!strcmp(getopt_get_string(state->gopt, "outfile"), "NULL")) {
        file_name = getopt_get_string(state->gopt, "outfile");
    }

    if (!strcmp(getopt_get_string(state->gopt, "outimage"), "NULL")) {
        imfile_name = getopt_get_string(state->gopt, "outimage");
    }


    verbose = getopt_get_bool(state->gopt, "verbose");
}

void loop_thread() {
    const int fps = 1;
    while (global_state->running) {
        usleep (1000000/fps);
    }
}

int main(int argc, char ** argv) {
    init_state(argc, argv);
    state_t* state = global_state;
    
    // vx_remote_display_source_attr_t remote_attr;
    // vx_remote_display_source_attr_init(&remote_attr);
    // remote_attr.advertise_name = "color picker";
    // vx_remote_display_source_t * cxn = vx_remote_display_source_create_attr(&state->app, &remote_attr);  

    ifstream mask(getopt_get_string(state->gopt, "mask"));

    mask >> state->mask_left >> state->mask_right >> state->mask_top >> state->mask_bot;

cout << "mask: " << state->mask_left << "," << state->mask_right << "," << state->mask_top << "," << state->mask_bot << endl;

    mask.close();

    image_u32_t*& im = state->im;
    float*& im_hsv = state->im_hsv;

    if (strcmp(getopt_get_string(state->gopt, "file"), "NULL")) {
        im = image_u32_create_from_pnm (getopt_get_string(state->gopt, "file"));

cout << "read from file " << getopt_get_string(state->gopt, "file") << endl;
    } 
    else { // no input file, use camera
        im = get_camera_frame();

cout << "read from camera " << endl;
    }

    assert(state->mask_left >= 0 && state->mask_left < im->width);
    assert(state->mask_right >= 0 && state->mask_right < im->width);
    assert(state->mask_top >= 0 && state->mask_top < im->height);
    assert(state->mask_bot >= 0 && state->mask_bot < im->height);

    im_hsv = (float*) calloc(1, 3*im->height*im->stride*sizeof(float));

cout << "im->height " << im->height << " im->width " << im->width << " stride " << im->stride << endl;
cout << "im_hsv size " << 3*(im->height*im->stride*sizeof(float)) << " im size " << (im->height*im->stride*sizeof(uint32_t)) << endl;
    
    uint8_t* buf = (uint8_t*) im->buf;
    
    // black out parts out side mask and make HSV of im
    for (int y = 0; y < im->height; ++y) {
        for (int x = 0; x < im->width; ++x) {
            int idx = y*im->stride+x;

            if (y < state->mask_top || y > state->mask_bot || x < state->mask_left || x > state->mask_right) {
                im->buf[idx] = 0xff000000;
                // im->buf[idx + 0] = 0x00;
                // im->buf[idx + 1] = 0x00;
                // im->buf[idx + 2] = 0x00;
                // im->buf[idx + 3] = 0x00;
            }
            else {
                // RGB2HSV((uint8_t) ((im->buf[idx] >> 0) & 0xff), 
                //         (uint8_t) ((im->buf[idx] >> 8) & 0xff),
                //         (uint8_t) ((im->buf[idx] >> 16) & 0xff),
                //         im_hsv[3*idx + 0], 
                //         im_hsv[3*idx + 1],
                //         im_hsv[3*idx + 2]);
                RGB2HSV(buf[4*idx + 0], 
                        buf[4*idx + 1],
                        buf[4*idx + 2],
                        im_hsv[3*idx + 0], 
                        im_hsv[3*idx + 1],
                        im_hsv[3*idx + 2]);
            }
        }
    }

    // draw image
    write_HSV_extreme();
    draw_im();

    // pthread_create (&state->loop_thread, NULL, (void* (*)(void*)) loop_thread, (void*) state);

    vx_gtk_display_source_t * appwrap = vx_gtk_display_source_create(&state->app);
    GtkWidget * window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    GtkWidget * canvas = vx_gtk_display_source_get_widget(appwrap);
    gtk_window_set_default_size (GTK_WINDOW (window), 1024, 768);
    gtk_container_add(GTK_CONTAINER(window), canvas);
    gtk_widget_show (window);
    gtk_widget_show (canvas);

    g_signal_connect_swapped(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_main (); // Blocks as long as GTK window is open
    gdk_threads_leave ();

    // quit when gtk closes? Or wait for remote displays/Ctrl-C
    if (!getopt_get_bool(state->gopt, "stay-open")) {
        state->running = 0;
    }

    image_u32_destroy(im);
    getopt_destroy (state->gopt);
    free (im_hsv);
    if (state->isrc != NULL) {
        state->isrc->stop(state->isrc);
    }
    // state->isrc->close(state->isrc);
    vx_gtk_display_source_destroy(appwrap);
    // vx_remote_display_source_destroy(cxn);
    vx_global_destroy();
    delete state;
}
