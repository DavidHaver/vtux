#ifndef _LEASE_MANAGER_H_
#define _LEASE_MANAGER_H_

#include <xcb/randr.h>

#define LEASE_MANAGER_CONNECTION_FAILED (-1)
#define LEASE_MANAGER_FAILED_TO_GET_REPLY (-2)
#define LEASE_MANAGER_FAILED_TO_GET_OUTPUT (-3)
#define LEASE_MANAGER_FAILED_TO_GET_CRTC (-4)
#define LEASE_MANAGER_FAILED_TO_CREATE_LEASE (-5)

typedef struct lease_manager 
{
    xcb_connection_t *connection;
    xcb_randr_create_lease_reply_t *leaseReply;
}lease_manager_t;

int createLease(lease_manager_t *lm);

void revokeLease(lease_manager_t *lm);

#endif /*_LEASE_MANAGER_H_*/
