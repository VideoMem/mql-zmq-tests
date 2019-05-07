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

class WorkerClientBase {
    protected:
        string zmq_address; 
        zmq::socket_t *client;
        zmq::context_t *context;
        void init();
        void close();
        void connect();
   public: 
        virtual string getName() { return "WorkerClientBase"; }
        void setAddr(string addr) { zmq_address = addr; }
        string sendTX(string payload);
        ~WorkerClientBase();
};


class WorkerA: public WorkerClientBase {
    public:
        WorkerA(string addr);
        string getName() { return "WorkerA"; }
        void txSomething();
};

class WorkerB: public WorkerClientBase {
    public:
        WorkerB(string addr);
        string getName() { return "WorkerB"; }
        void txSomething();

};

void WorkerClientBase::init() {
    context = new zmq::context_t(1);
    client = new zmq::socket_t (*context, ZMQ_REQ);
}

void WorkerClientBase::close() {
    delete client;
    delete context;
}

WorkerClientBase::~WorkerClientBase() {
    close();
};

void WorkerClientBase::connect() {
    cout << "I: connecting to server…" << endl;
    init();
    client->connect (zmq_address);

    //  Configure socket to not wait at close time
    int linger = 0;
    client->setsockopt (ZMQ_LINGER, &linger, sizeof (linger));
}

string WorkerClientBase::sendTX(string payload) {
    int retries_left = REQUEST_RETRIES;
    string reply = "";

    while (retries_left) {
        stringstream request;
        request << payload;
        s_send (*client, request.str());
        sleep (1);

        bool expect_reply = true;
        while (expect_reply) {
            //  Poll socket for a reply, with timeout
            zmq::pollitem_t items[] = { { *client, 0, ZMQ_POLLIN, 0 } };
            zmq::poll (&items[0], 1, REQUEST_TIMEOUT);

            //  If we got a reply, process it
            if (items[0].revents & ZMQ_POLLIN) {
                //  We got a reply from the server, must match sequence
                reply = s_recv (*client);
                if (reply.size() > 0) {
                    cout << "I: server replied OK (" << reply.size() << ") bytes" << endl;
                    retries_left = 0;
                    expect_reply = false;
                }
                else {
                    cout << "E: malformed reply from server: " << reply << endl;
                }
            }
            else
            if (--retries_left == 0) {
                cout << "E: server seems to be offline, abandoning" << endl;
                expect_reply = false;
                break;
            }
            else {
                cout << "W: no response from server, retrying…" << endl;
                //  Old socket will be confused; close it and open a new one
                close();
                connect();
                //  Send request again, on new socket
                s_send (*client, request.str());
            }
        }
    }
    return reply;
}

WorkerA::WorkerA(string addr) {
    setAddr(addr);
    connect();
}

WorkerB::WorkerB(string addr) {
    setAddr(addr);
    connect();
}

void WorkerA::txSomething() {
    string payload = "#this is a R script";
    printf("reply body: %s\n", sendTX(payload).c_str());
}

void WorkerB::txSomething() {
    string payload = "{ \"some_json_log\": { \"SYMBOL\": \"EURUSD\", \"MAGIC\": \"42\", \"etc ...\":\"etc\" }}";
    printf("reply body: %s\n", sendTX(payload).c_str());
}

WorkerA* workerA;
WorkerB* workerB;

void OnInit() {
   workerA = new WorkerA("tcp://localhost:5555");
   workerB = new WorkerB("tcp://localhost:5566");
}

int tickC = 0;
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

