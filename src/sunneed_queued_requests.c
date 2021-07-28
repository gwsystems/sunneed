#include "sunneed_queued_requests.h"

void SunneedRequest_List_init(void) {
    sunneed_queued_requests.head = sunneed_queued_requests.tail = NULL;
    sunneed_queued_requests.num_active_requests = 0;
}

void insert_request(SunneedRequest *request, struct sunneed_tenant *tenant, nng_pipe tenant_pipe, uint power) {
    struct SunneedRequest_ListNode *new_req;
    new_req = (struct SunneedRequest_ListNode*) malloc(sizeof(struct SunneedRequest_ListNode));
    new_req->request = request;
    new_req->tenant = tenant;
    new_req->tenant_pipe = tenant_pipe;
    new_req->power = power;
    new_req->next = NULL;
    if (sunneed_queued_requests.head == NULL) {
        sunneed_queued_requests.head = sunneed_queued_requests.tail = new_req;
        ++sunneed_queued_requests.num_active_requests;
        return;
    }
    sunneed_queued_requests.tail->next = new_req;
    sunneed_queued_requests.tail = new_req;
    ++sunneed_queued_requests.num_active_requests;
}

/* 
 * insert tenant request with <offset> requests in front of it 
 * returns 1 on success, 0 on failure
 */
int insert_request_offset(SunneedRequest *request, struct sunneed_tenant *tenant, nng_pipe tenant_pipe, uint power, uint offset) {
    struct SunneedRequest_ListNode *ptr, *node_afterInsert, *new_req;
    new_req = (struct SunneedRequest_ListNode*) malloc(sizeof(struct SunneedRequest_ListNode));
    new_req->request = request;
    new_req->tenant = tenant;
    new_req->tenant_pipe = tenant_pipe;
    new_req->power = power;

    ptr = sunneed_queued_requests.head;
    for (uint i = 0; i < offset - 1; i++) {
        if (ptr->next == NULL) return 0;
        ptr = ptr->next;
    }

    node_afterInsert = ptr->next;
    ptr->next = new_req;
    new_req->next = node_afterInsert;
    ++sunneed_queued_requests.num_active_requests;
    return 0;
}

struct SunneedRequest_ListNode* pop_requestNode(void) {
    struct SunneedRequest_ListNode *requestNode = sunneed_queued_requests.head;
    sunneed_queued_requests.head = sunneed_queued_requests.head->next;
    --sunneed_queued_requests.num_active_requests;
    return requestNode;
}