#include "lpc.hpp"

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

LPCCALL void worker_add(string_t* name, string_t* address, widptr_t& nid) {
    string stdname, stdaddr;
    string_marshall(name, stdname);
    string_marshall(address, stdaddr);

    if(wid >= 0 || wid < LPC_MAX_WORKERS) {
        workers[wid] = new Worker(stdaddr);
        workers[wid]->setName(stdname);
        widptr_t id = wid;
        wid++;
        nid = id;
    } else
        nid = -1;
}

LPCCALL size_t worker_tx(widptr_t id, string_t* payload, string_t* rx) {
    string stdpayload;
    string_marshall(payload, stdpayload);
    string reply = "";
    if(check_bounds(id))
        reply = workers[id]->sendTX(stdpayload);
    else
        reply = "";
    string_unmarshall(reply,rx);

    __tickC++;
    return (size_t) reply.size();
}

LPCCALL void worker_echo(widptr_t id, string_t* payload) {
    string stdpayload;
    string_marshall(payload, stdpayload);
    if(check_bounds(id))
        workers[id]->echo(stdpayload);
    __tickC++;
}

LPCCALL void worker_getname(widptr_t id, string_t* name) {
    string stdname = "";
    if(check_bounds(id))
        stdname = workers[id]->getName();
    else {
        char buf[256];
        sprintf(buf ,"%d: Worker non initialized", id);
        stdname = buf;
    }
    string_unmarshall(stdname, name);
}

LPCCALL void worker_getLastError(widptr_t id, int_t &err) {
    int error = 0;
    if(check_bounds(id))
        error = workers[id]->getLastError();
    else {
        error = LPC_ERR_INVALIDHANDLE;
    }
    err = (int_t) error;
}

LPCCALL void worker_getLErrContext(widptr_t id, string_t* err) {
    string errctx = "";
    if(check_bounds(id))
        errctx = workers[id]->getLErrContext();
    else  {
        char buf[256];
        sprintf(buf ,"%d: Worker non initialized", id);
        errctx = buf;
    }
    string_unmarshall(errctx, err);
}

LPCCALL void worker_setRequestTimeout(widptr_t id, int_t value) {
    if(check_bounds(id))
        workers[id]->setRequestTimeout(value);
}

LPCCALL void worker_setRequestRetries(widptr_t id, int_t value) {
    if(check_bounds(id))
        workers[id]->setRequestRetries(value);
}

LPCCALL void worker_setname(widptr_t id, string_t* name) {
    string Name;
    string_marshall(name, Name);
    if(check_bounds(id))
        workers[id]->setName(Name);
}

LPCCALL void worker_setaddr(widptr_t id, string_t* addr) {
    string Addr;
    string_marshall(addr, Addr);
    if(check_bounds(id))
        workers[id]->setName(Addr);
}

LPCCALL void worker_deinit(void) {
    for(int i = 0; i < wid; i++)
        delete(workers[i]);
    Sleep(3);
    wid = 0;
}