#ifndef _QUEUED_REQUESTS_H_
#define _QUEUED_REQUESTS_H_

#include <stdlib.h>
#include "log.h"
#include <nng/nng.h>
#include <nng/protocol/reqrep0/rep.h>
#include <nng/protocol/reqrep0/req.h>
#include "protobuf/c/server.pb-c.h"

#define SUB_RESPONSE_BUF_SZ 4096

struct SunneedRequest_ListNode{
    SunneedRequest *request;
    struct sunneed_tenant *tenant;
    nng_pipe tenant_pipe;
    struct SunneedRequest_ListNode *next;
    uint power;
};

struct SunneedRequest_List{
    struct SunneedRequest_ListNode *head, *tail;
    uint num_active_requests;
} sunneed_queued_requests;

void SunneedRequest_List_init(void);
void insert_request(SunneedRequest *request, struct sunneed_tenant *tenant, nng_pipe tenant_pipe, uint power);
int  insert_request_offset(SunneedRequest *request, struct sunneed_tenant *tenant, nng_pipe tenant_pipe, uint power, uint offset);
struct SunneedRequest_ListNode* pop_requestNode(void);
#endif