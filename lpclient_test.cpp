//
//  Lazy Pirate client
//  Use zmq_poll to do a safe request-reply
//
//
//
#include "lpcapi.hpp"

string rPay() {
    string payload = "#this is a R script";
    return payload;
}

string jsonPay() {
    string payload = "{ \"some_json_log\": { \"SYMBOL\": \"EURUSD\", \"MAGIC\": \"42\", \"etc ...\":\"etc\" }}";
    return payload;
}

void OnInit() {
    widptr_t handle;
    worker_add("WorkerA", "tcp://localhost:5555", handle);
    worker_setRequestTimeout(handle, 5000);
    worker_add("WorkerB", "tcp://localhost:5566", handle);
}

void OnTick() {
    printf("Ontick() loop number %d:\n", __tickC);
    widptr_t pos = __tickC % wid;
    if(pos % 2 == 0)
        worker_echo(pos, rPay());
    else
        worker_echo(pos, jsonPay());
    __tickC++;
}

int main () {
    OnInit();
    while(true) {
        OnTick();
    }
    return 0;
}

