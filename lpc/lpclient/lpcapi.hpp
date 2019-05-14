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

//safe null strcpy
void string_copy(char* src, string &ret, int_t len) {
    int dstsize = len + 1;
    char dst[dstsize] = {0};
    strncpy(dst, src, len);
    dst[dstsize] = 0;
    ret = dst;
}

Worker *workers[LPC_MAX_WORKERS];
widptr_t wid = 0;

bool check_bounds(widptr_t w) { return (w >= 0 && w < wid && w < LPC_MAX_WORKERS)? true: false;}

LPCCALL void worker_add(char* name, int_t nlen, char* address, int_t alen, widptr_t& nid) {
    string stdname;
    string stdaddr;
    string_copy(name, stdname, nlen);
    string_copy(address, stdaddr, alen);

    if(wid >= 0 || wid < LPC_MAX_WORKERS) {
        workers[wid] = new Worker(stdaddr);
        workers[wid]->setName(stdname);
        widptr_t id = wid;
        wid++;
        nid = id;
    } else
        nid = -1;
}

LPCCALL size_t worker_send(widptr_t id, char* payload, int_t psize) {
    string stdpayload;
    string_copy(payload, stdpayload, psize);
    string reply = "";
    if(check_bounds(id))
        reply = workers[id]->sendTX(stdpayload);
    else
        reply = "";

    __tickC++;
    return (size_t) reply.size();
}

LPCCALL size_t worker_getreplysize(widptr_t id) {
    size_t replysize = 0;
    if(check_bounds(id))
        workers[id]->getLastReplySize(replysize);
    return replysize;
}

LPCCALL void worker_getreply(widptr_t id, char *ret, int_t size) {
    string reply = "";
    ret[0] = 0;
    if(check_bounds(id) && size > 0) {
        workers[id]->getLastReply(reply);
        strncpy(ret, reply.c_str(), size);
        ret[size-1]=0;
    }
}

LPCCALL void worker_echo(widptr_t id, char* payload, int_t len) {
    string stdpayload;
    string_copy(payload, stdpayload, len);
    if(check_bounds(id))
        workers[id]->echo(stdpayload);
    __tickC++;
}

LPCCALL void worker_getname(widptr_t id, char* name, int_t &size) {
    string stdname = "";
    if(check_bounds(id))
        stdname = workers[id]->getName();
    else {
        char buf[256];
        sprintf(buf ,"%d: Worker non initialized", id);
        stdname = buf;
    }
    size = strlen(stdname.c_str());
    strncpy(name, stdname.c_str(), size);
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

LPCCALL void worker_getLErrContext(widptr_t id, char* err, int_t &size) {
    string errctx = "";
    if(check_bounds(id))
        errctx = workers[id]->getLErrContext();
    else  {
        char buf[256];
        sprintf(buf ,"%d: Worker non initialized", id);
        errctx = buf;
    }
    size = strlen(errctx.c_str());
    strncpy(err, errctx.c_str(), size);
}

LPCCALL void worker_setRequestTimeout(widptr_t id, int_t value) {
    if(check_bounds(id))
        workers[id]->setRequestTimeout(value);
}

LPCCALL void worker_setRequestRetries(widptr_t id, int_t value) {
    if(check_bounds(id))
        workers[id]->setRequestRetries(value);
}

LPCCALL void worker_setname(widptr_t id, char* name, int_t size) {
    string Name;
    string_copy(name, Name, size);
    if(check_bounds(id))
        workers[id]->setName(Name);
}

LPCCALL void worker_setaddr(widptr_t id, char* addr, int_t size) {
    string Addr;
    string_copy(addr, Addr, size);
    if(check_bounds(id))
        workers[id]->setName(Addr);
}

LPCCALL void worker_deinit(void) {
    for(int i = 0; i < wid; i++)
        delete(workers[i]);
    wid = 0;
}