//
//  Lazy Pirate client
//  Use zmq_poll to do a safe request-reply
//  To run, start piserver and then randomly kill/restart it
//
#include "zhelpers.hpp"
#include <sstream>

#define REQUEST_TIMEOUT     2500    //  msecs, (> 1000!)
#define REQUEST_RETRIES     3       //  Before we abandon

class WorkerClientBase {
    protected:
        std::string zmq_address; 
        zmq::socket_t *client;
        zmq::context_t *context;
        void initZMQ();
        void connect();
   public: 
        virtual std::string getName() { return "WorkerClientBase"; }
        void setAddr(std::string addr) { zmq_address = addr; }
        std::string sendTX(std::string payload);
        WorkerClientBase(zmq::context_t &ctx);
        ~WorkerClientBase();
};

void WorkerClientBase::initZMQ() {
    client = new zmq::socket_t (*context, ZMQ_REQ);
}


WorkerClientBase::WorkerClientBase(zmq::context_t &ctx) {
    context = &ctx;
    initZMQ();
}

WorkerClientBase::~WorkerClientBase() {
    delete client;
};

void WorkerClientBase::connect() {
    std::cout << "I: connecting to server…" << std::endl;
    client->connect (zmq_address);

    //  Configure socket to not wait at close time
    int linger = 0;
    client->setsockopt (ZMQ_LINGER, &linger, sizeof (linger));
}

std::string WorkerClientBase::sendTX(std::string payload) {
    int sequence = 0;
    int retries_left = REQUEST_RETRIES;
    std::string reply = "";

    while (retries_left) {
        std::stringstream request;
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
                if (atoi (reply.c_str ()) == sequence) {
                    std::cout << "I: server replied OK (" << reply << ")" << std::endl;
                    retries_left = REQUEST_RETRIES;
                    expect_reply = false;
                    return reply;
                }
                else {
                    std::cout << "E: malformed reply from server: " << reply << std::endl;
                }
            }
            else
            if (--retries_left == 0) {
                std::cout << "E: server seems to be offline, abandoning" << std::endl;
                expect_reply = false;
                break;
            }
            else {
                std::cout << "W: no response from server, retrying…" << std::endl;
                //  Old socket will be confused; close it and open a new one
                delete client;
                initZMQ();
                connect();
                //  Send request again, on new socket
                s_send (*client, request.str());
            }
        }
    }
    return reply;
}

class WorkerA: public WorkerClientBase {
    public:
        WorkerA(zmq::context_t &ctx, std::string addr);
        std::string getName() { return "WorkerA"; }
        void txSomething();
};

class WorkerB: public WorkerClientBase {
    public:
        WorkerB(zmq::context_t &ctx, std::string addr);
        std::string getName() { return "WorkerB"; }
        void txSomething();

};

WorkerA::WorkerA(zmq::context_t &ctx, std::string addr):WorkerClientBase(ctx) {
    setAddr(addr);
    connect();
}

WorkerB::WorkerB(zmq::context_t &ctx, std::string addr):WorkerClientBase(ctx) {
    setAddr(addr);
    connect();
}

void WorkerA::txSomething() {
    std::string payload = "#this is a R script";
    printf("reply body: %s\n", sendTX(payload).c_str());
}

void WorkerB::txSomething() {
    std::string payload = "{ \"some_json_log\": { \"SYMBOL\": \"EURUSD\", \"MAGIC\": \"42\", \"etc ...\":\"etc\" }}";
    printf("reply body: %s\n", sendTX(payload).c_str());
}

zmq::context_t sharedContext (1);
WorkerA* workerA;
WorkerB* workerB;
void onInit() {
   workerA = new WorkerA(sharedContext, "tcp://localhost:5555");
   workerB = new WorkerB(sharedContext, "tcp://localhost:5566");
}

int tickC = 0;
void onTick() {
    printf("On tick loop number: %d", tickC);
    workerA->txSomething();
    workerB->txSomething();
    tickC++;
}

int main () {
    onInit();
    while(true) {
        onTick();    
    }
    return 0;
}

