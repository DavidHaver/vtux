#ifndef _LEASE_MANAGER_H_
#define _LEASE_MANAGER_H_

#include <stdio.h>
#include <stdlib.h>

#include <errno.h>
#include <string.h>

/* #include <xcb/xcb.h> */
#include <xcb/randr.h>

struct leaseManager {
    xcb_connection_t *connection;
    xcb_randr_create_lease_reply_t *leaseReply;
};

int createLease(struct leaseManager *lm);

void revokeLease(struct leaseManager *lm);

#endif /*_LEASE_MANAGER_H_*/
