//
//  Lazy Pirate client
//  Use zmq_poll to do a safe request-reply
//  To run, start piserver and then randomly kill/restart it
//  This example needs two workers on different addresses
//
#include "zhelpers.hpp"
#include <sstream>
using namespace std;

#define REQUEST_TIMEOUT     2500    //  msecs, (> 1000!)
#define REQUEST_RETRIES     3       //  Before we abandon
#define LPC_ERR_REXCEED     787
#define LPC_MAX_WORKERS     1024    //max workers in this api

int tickC = 0; //for OnTick();

class LazyPirate {
    protected:
        string zmq_address;
        string name;
        int request_retries;
        int request_timeout;
        int error_code;
        string error_context;
        zmq::socket_t *client;
        zmq::context_t *context;
        void init();
        void close();
        void connect();
        void setError(int code, string desc) { error_code = code; error_context = desc; }
        void safeSend(stringstream&);
        void safePoll(zmq::pollitem_t*);
        void safeRecv(string&);

   public:
        string getName() { return name; }
        bool hasError() { return error_code==0 ? true: false; }
        int getLastError() { return error_code; }
        string getLErrContext() { return error_context; }
        void setName(string n) { name = n; }
        void setAddr(string addr) { zmq_address = addr; }
        void setRequestRetries(int v) { request_retries = v; }
        void setRequestTimeout(int v) { request_timeout = v; }
        string sendTX(string payload);
        LazyPirate();
        ~LazyPirate();
};

LazyPirate::LazyPirate() {
    context = new zmq::context_t(1);
    request_retries = REQUEST_RETRIES;
    request_timeout = REQUEST_TIMEOUT;
}

void LazyPirate::init() {
    client = new zmq::socket_t (*context, ZMQ_REQ);
}

void LazyPirate::close() {
   client->close();
   delete client;
}

LazyPirate::~LazyPirate() {
    close();
    delete context;
}

void LazyPirate::connect() {
    cout << "I: connecting to server…" << endl;
    try {
        init();
        client->connect (zmq_address);

        //  Configure socket to not wait at close time
        int linger = 0;
        client->setsockopt (ZMQ_LINGER, &linger, sizeof (linger));
    } catch (int e) {
        setError(e, name + "::connect()");
    }
}

void LazyPirate::safeSend(stringstream &m) {
    try {
        s_send (*client, m.str());
    } catch (int e) {
        setError(e, name + "::safeSend()");
    }
}

void LazyPirate::safePoll(zmq::pollitem_t* items) {
    try {
        zmq::poll (&items[0], 1, request_timeout);
    } catch (int e) {
        setError(e, name + "::safePoll()");
    }
}

void LazyPirate::safeRecv(string &m) {
    try {
        m = s_recv (*client);
    } catch (int e) {
        setError(e, name + "::safeRecv()");
    }
}


string LazyPirate::sendTX(string payload) {
    int retries_left = request_retries;
    string reply = "";

    while (retries_left) {
        stringstream request;
        request << payload;
        safeSend(request);

        bool expect_reply = true;
        while (expect_reply) {
            //  Poll socket for a reply, with timeout
            zmq::pollitem_t items[] = { { *client, 0, ZMQ_POLLIN, 0 } };
            safePoll(items);
            //  If we got a reply, process it
            if (items[0].revents & ZMQ_POLLIN) {
                //  We got a reply from the server
                safeRecv(reply);
                //reply = s_recv (*client);
                if (reply.size() > 0) {
                    cout << "I: server replied OK (" << reply.size() << ") bytes" << endl;
                    retries_left = 0;
                    expect_reply = false;
                    setError(0, name + "::sendTX() sucess!");
                }
                else {
                    cout << "E: malformed reply from server: " << reply << endl;
                }
            }
            else
            if (--retries_left == 0) {
                cout << "E: server seems to be offline, abandoning" << endl;
                expect_reply = false;
                setError(LPC_ERR_REXCEED, name + "::sendTX() retries exceeded");
                close();
                connect();
                break;
            }
            else {
                cout << "W: no response from server, retrying…" << endl;
                //  Old socket will be confused; close it and open a new one
                close();
                connect();
                //  Send request again, on new socket
                safeSend(request);
            }
        }
    }
    return reply;
}

class WorkerA: public LazyPirate {
    public:
        WorkerA(string addr) { setName("WorkerA"); setAddr(addr); connect(); }
        void txSomething();
};

class WorkerB: public LazyPirate {
    public:
        WorkerB(string addr) { setName("WorkerB"); setAddr(addr); connect(); }
        void txSomething();
};


void WorkerA::txSomething() {
    string f_payload = "%d #this is a R script";
    char buf[256];
    sprintf(buf, f_payload.c_str(), tickC);
    string payload = buf;
    printf("reply body: %s\n", sendTX(payload).c_str());
    printf("Last error context: %s\n", getLErrContext().c_str());
}

void WorkerB::txSomething() {
    string f_payload = "%d { \"some_json_log\": { \"SYMBOL\": \"EURUSD\", \"MAGIC\": \"42\", \"etc ...\":\"etc\" }}";
    char buf[256];
    sprintf(buf, f_payload.c_str(), tickC);
    string payload = buf;
    printf("reply body: %s\n", sendTX(payload).c_str());
    printf("Last error context: %s\n", getLErrContext().c_str());
}

WorkerA* workerA;
WorkerB* workerB;

void OnInit() {
   workerA = new WorkerA("tcp://localhost:5555");
   workerB = new WorkerB("tcp://localhost:5566");
}

void OnTick() {
    printf("On tick loop number %d:\n", tickC);
    workerA->txSomething();
    workerB->txSomething();
    tickC++;
}

int main () {
    OnInit();
    while(true) {
        OnTick();
    }
    return 0;
}

