#include "lpc.hpp"

#define widptr_t uint32_t
#define int_t uint32_t

int __tickC = 0;

void Worker::echo(string pay) {
    string f_payload = "%d " + pay;
    char buf[256];
    sprintf(buf, f_payload.c_str(), __tickC);
    string payload = buf;
    printf("reply body: %s\n", sendTX(payload).c_str());
    printf("Last error context: %s\n", getLErrContext().c_str());
}

Worker *workers[LPC_MAX_WORKERS];
widptr_t wid = 0;

bool check_bounds(widptr_t w) { return (w >= 0 && w < wid && w < LPC_MAX_WORKERS)? true: false;}

widptr_t worker_add(string name, string address) {
    if(wid >= 0 || wid < LPC_MAX_WORKERS) {
        workers[wid] = new Worker(address);
        workers[wid]->setName(name);
        widptr_t id = wid;
        wid++;
        return id;
    } else
        return -1;
}

//TODO: string must be wstring
string worker_tx(widptr_t id, string payload) {
    string reply = "";
    if(check_bounds(id))
        reply = workers[id]->sendTX(payload);
    else
        reply = "";
    return reply;
}

//TODO: string must be wstring
void worker_echo(widptr_t id, string payload) {
    if(check_bounds(id))
        workers[id]->echo(payload);
}

//TODO: string must be wstring?
string worker_name(widptr_t id) {
    string name = "";
    if(check_bounds(id))
        name = workers[id]->getName();
    else {
        char buf[256];
        sprintf(buf ,"%d: Worker non initialized", id);
        name = buf;
    }
    return name;
}

int_t worker_getLastError(widptr_t id) {
    int error = 0;
    if(check_bounds(id))
        error = workers[id]->getLastError();
    else {
        error = LPC_ERR_INVALIDHANDLE;
    }
    return error;
}

string worker_getLErrContext(widptr_t id) {
    string errctx = "";
    if(check_bounds(id))
        errctx = workers[id]->getLErrContext();
    else  {
        char buf[256];
        sprintf(buf ,"%d: Worker non initialized", id);
        errctx = buf;
    }
}

void worker_setRequestTimeout(widptr_t id, int_t value) {
    if(check_bounds(id))
        workers[id]->setRequestTimeout(value);
}

void worker_setRequestRetries(widptr_t id, int_t value) {
    if(check_bounds(id))
        workers[id]->setRequestRetries(value);
}
