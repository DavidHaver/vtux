#include <stdlib.h>
#include "lease_manager.h"

#ifdef DEBUG
#include <stdio.h>
#include <string.h>
#endif

static void failureCleanUp
(xcb_connection_t *connection, xcb_randr_get_screen_resources_reply_t *resourcesReply);

int createLease(lease_manager_t *lm) {
    int screen = 0;
    xcb_randr_get_screen_resources_reply_t *resourcesReply = NULL;

    //connecting to the X server
    xcb_connection_t *connection = xcb_connect(NULL, &screen);

    //checking connection error
    int err = xcb_connection_has_error(connection);
    if(err) 
    {
        #ifdef DEBUG
        printf("connection has error: %s\n", strerror(err));
        #endif
        failureCleanUp(connection, resourcesReply);
        return LEASE_MANAGER_CONNECTION_FAILED;
    }
    
    //getting root window
    xcb_screen_iterator_t screenItr = xcb_setup_roots_iterator(xcb_get_setup(connection));

    for(; screenItr.rem && screen; xcb_screen_next(&screenItr), --screen)
    {
        //empty
    }
    xcb_window_t root = screenItr.data->root;

    //getting screen resources for lease
    xcb_generic_error_t *xerr = NULL;

    xcb_randr_get_screen_resources_cookie_t resourcesCookie = 
    xcb_randr_get_screen_resources(connection, root);

    resourcesReply = 
    xcb_randr_get_screen_resources_reply(connection, resourcesCookie, &xerr);
    if(!resourcesReply) 
    {
        #ifdef DEBUG
        printf("failed to get screen resources: %s\n", strerror(xerr->error_code));
        #endif
        failureCleanUp(connection, resourcesReply);
        return LEASE_MANAGER_FAILED_TO_GET_REPLY;
    }

    xcb_randr_output_t *outputsArray = xcb_randr_get_screen_resources_outputs(resourcesReply);
    xcb_randr_output_t output = 0;
    for(int i = 0; 0 == output && i < resourcesReply->num_outputs; ++i) 
    {
        xcb_randr_get_output_info_cookie_t outputCookie = 
        xcb_randr_get_output_info(connection, outputsArray[i], resourcesReply->config_timestamp);

        xcb_randr_get_output_info_reply_t *outputReply = 
        xcb_randr_get_output_info_reply(connection, outputCookie, &xerr);
        if (!outputReply) 
        {
            #ifdef DEBUG
            printf("failed to get output from X server: %s\n", strerror(xerr->error_code));
            #endif
            failureCleanUp(connection, resourcesReply);
            return LEASE_MANAGER_FAILED_TO_GET_OUTPUT;
        }
        if (outputReply->connection == XCB_RANDR_CONNECTION_CONNECTED) 
        {
            output = outputsArray[i];
        }
        free(outputReply);
    }

    if(!output) 
    {
        #ifdef DEBUG
        printf("output not found\n");
        #endif
        failureCleanUp(connection, resourcesReply);
        return LEASE_MANAGER_FAILED_TO_GET_OUTPUT;
    }

    xcb_randr_crtc_t *crtcsArray = xcb_randr_get_screen_resources_crtcs(resourcesReply);
    xcb_randr_crtc_t crtc = 0;
    for(int i=0;  0 == crtc && i<resourcesReply->num_crtcs; ++i) 
    {
        xcb_randr_get_crtc_info_cookie_t crtcCookie = 
        xcb_randr_get_crtc_info(connection, crtcsArray[i], resourcesReply->config_timestamp);

        xcb_randr_get_crtc_info_reply_t *crtcReply = 
        xcb_randr_get_crtc_info_reply(connection, crtcCookie, &xerr);
        if(!crtcReply) 
        {
            #ifdef DEBUG
            printf("failed to get crtc from X server: %s\n", strerror(xerr->error_code));
            #endif
            failureCleanUp(connection, resourcesReply);
            return LEASE_MANAGER_FAILED_TO_GET_CRTC;
        }
        if (crtcReply->mode != 0) 
        {
            crtc = crtcsArray[i];
        }
        free(crtcReply);
    }

    if(!crtc) 
    {
        #ifdef DEBUG
        printf("crtc not found\n");
        #endif
        failureCleanUp(connection, resourcesReply);
        return LEASE_MANAGER_FAILED_TO_GET_CRTC;
    }

    free(resourcesReply);
    
    xcb_randr_lease_t lease = xcb_generate_id(connection);
    xcb_randr_create_lease_cookie_t leaseCookie = 
    xcb_randr_create_lease(connection, root, lease, 1, 1, &crtc, &output);

    xcb_randr_create_lease_reply_t *leaseReply = 
    xcb_randr_create_lease_reply(connection, leaseCookie, &xerr);
    if(!leaseReply) 
    {
        #ifdef DEBUG
        printf("failed to create lease from X server: %s\n", strerror(xerr->error_code));
        #endif
        failureCleanUp(connection, resourcesReply);
        return LEASE_MANAGER_FAILED_TO_CREATE_LEASE;
    }

    lm->connection = connection;
    lm->leaseReply = leaseReply;

    //although this function docs say return value needs to be freed
    //trying to free the pointer will result in invalid pointer
    //and no leaks all be found by valgrind
    int *leaseFD = xcb_randr_create_lease_reply_fds(connection, leaseReply);

    return leaseFD[0];
}

void revokeLease(lease_manager_t *lm) 
{
    free(lm->leaseReply);
    xcb_disconnect(lm->connection);
}

void failureCleanUp
(xcb_connection_t *connection, xcb_randr_get_screen_resources_reply_t *resourcesReply)
{
    xcb_disconnect(connection);
    free(resourcesReply);
}