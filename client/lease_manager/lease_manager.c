#include "lease_manager.h"

int createLease(struct leaseManager *lm) {
    int ret = 0;
    int screen, err;
    xcb_generic_error_t *xerr;
    xcb_connection_t *connection;

    //connecting to the X server
    connection = xcb_connect(NULL, &screen);
    if (!connection) {
        printf("failed to connect to X server\n");
        ret = -1;
        goto failed_x_con;
    }

    //checking connection error
    err = xcb_connection_has_error(connection);
    if (err) {
        printf("connection has error: %s\n", strerror(err));
        ret = -2;
        goto failed_x_con;
    }
    
    //getting root window
    const xcb_setup_t *setup = xcb_get_setup(connection);
    xcb_screen_iterator_t screenItr = xcb_setup_roots_iterator(setup);
    int screenIndex = 0;
    while (screenItr.rem) {
        if (screenIndex == screen) {
            break;
        }
        xcb_screen_next(&screenItr);
        screenIndex++;
    }
    xcb_window_t root = screenItr.data->root;

    //getting screen resources
    xcb_randr_get_screen_resources_cookie_t resourcesCookie = xcb_randr_get_screen_resources(connection, root);
    xcb_randr_get_screen_resources_reply_t *resourcesReply = xcb_randr_get_screen_resources_reply(connection, resourcesCookie, &xerr);
    if (!resourcesReply) {
        printf("failed to get screen resources: %s\n", strerror(xerr->error_code));
        ret = -3;
        goto failed_x_con;
    }

    xcb_randr_output_t *outputs = xcb_randr_get_screen_resources_outputs(resourcesReply);
    xcb_randr_output_t output = 0;
    for (int o=0; output==0 && o<resourcesReply->num_outputs; o++) {
        xcb_randr_get_output_info_cookie_t outputCookie = xcb_randr_get_output_info(connection, outputs[o], resourcesReply->config_timestamp);
        xcb_randr_get_output_info_reply_t *outputReply = xcb_randr_get_output_info_reply(connection, outputCookie, &xerr);
        if (!outputReply) {
            printf("failed to get output from X server: %s\n", strerror(xerr->error_code));
            ret = -4;
            goto failed_output;
        }
        if (outputReply->connection == XCB_RANDR_CONNECTION_CONNECTED) {
            output = outputs[o];
        }
        free(outputReply);
    }
    if (!output) {
        printf("output not found\n");
        ret = -5;
        goto failed_output;
    }

    xcb_randr_crtc_t *crtcs = xcb_randr_get_screen_resources_crtcs(resourcesReply);
    xcb_randr_crtc_t crtc = 0;
    for (int c=0; crtc==0 && c<resourcesReply->num_crtcs; c++) {
        xcb_randr_get_crtc_info_cookie_t crtcCookie = xcb_randr_get_crtc_info(connection, crtcs[c], resourcesReply->config_timestamp);
        xcb_randr_get_crtc_info_reply_t *crtcReply = xcb_randr_get_crtc_info_reply(connection, crtcCookie, &xerr);
        if (!crtcReply) {
            printf("failed to get crtc from X server: %s\n", strerror(xerr->error_code));
            ret = -6;
            goto failed_crtc;
        }
        if (crtcReply->mode != 0) {
            crtc = crtcs[c];
        }
        free(crtcReply);
    }
    if (!crtc) {
        printf("crtc not found\n");
        ret = -7;
        goto failed_crtc;
    }

    free(resourcesReply);
    
    xcb_randr_lease_t lease = xcb_generate_id(connection);
    xcb_randr_create_lease_cookie_t leaseCookie = xcb_randr_create_lease(connection, root, lease, 1, 1, &crtc, &output);
    xcb_randr_create_lease_reply_t *leaseReply = xcb_randr_create_lease_reply(connection, leaseCookie, &xerr);
    if (!leaseReply) {
        printf("failed to create lease from X server: %s\n", strerror(xerr->error_code));
        ret = -8;
        goto failed_lease;
    }

    //although this function docs say return value needs to be freed
    //trying to free the pointer will result in invalid pointer
    //and no leaks all be found by valgrind
    int *leaseFD = xcb_randr_create_lease_reply_fds(connection, leaseReply);
    int fd = leaseFD[0];

    lm->connection = connection;
    lm->leaseReply = leaseReply;

    return fd;

failed_lease:
failed_crtc:
failed_output:
    free(resourcesReply);
failed_x_con:
    xcb_disconnect(connection);
    return ret;
}

void revokeLease(struct leaseManager *lm) {
    free(lm->leaseReply);
    xcb_disconnect(lm->connection);
}
