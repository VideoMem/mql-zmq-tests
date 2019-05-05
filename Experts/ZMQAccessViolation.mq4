//+------------------------------------------------------------------+
//|                                           ZMQAccessViolation.mq4 |
//|                                  Copyleft 2019, swilwerth@github |
//|                                             github.com/swilwerth |
//+------------------------------------------------------------------+
// This is a bugreport code. It'll crash Metatrader Standard Terminal
// under certain conditions.
// Do not use it in any production code.

#property copyright "Copyleft 2019, swilwerth@github"
#property link      "github.com/swilwerth"
#property version   "1.00"
#property strict

#include <Zmq/Zmq.mqh>

#define REQUEST_TIMEOUT     2500    //  msecs, (> 1000!)
#define REQUEST_RETRIES     3       //  Before we abandon

class WorkerClientBase {
    protected:
        string zmq_address;
        Socket *socket;
        Context *context;
        void connect();
        void initZMQ();
    public:
        virtual string getName() { return "WorkerClientBase"; }
        void setAddr(string addr) { zmq_address = addr; }
        ZmqMsg sendTX(string payload);
        WorkerClientBase(Context &ctx);
        ~WorkerClientBase();
};

class WorkerA: public WorkerClientBase {
    public:
        WorkerA(Context &ctx, string addr);
        string getName() { return "WorkerA"; }
        void txSomething();
};

class WorkerB: public WorkerClientBase {
    public:
        WorkerB(Context &ctx, string addr);
        string getName() { return "WorkerB"; }
        void txSomething();
};

WorkerClientBase::WorkerClientBase(Context &c) {
    context = &c;
    initZMQ();
}

WorkerClientBase::~WorkerClientBase() {
    delete(socket);
}

void WorkerClientBase::initZMQ() {
    printf("Creating new socket");
    this.socket = new Socket(context, ZMQ_REQ);
}

//Start of lazy pirate pattern client TX adapted to MQL5 from
//sample code in c++ http://zguide.zeromq.org/cpp:lpclient
void WorkerClientBase::connect() {
    printf("Opening new client socket connection to %s", zmq_address);
    if(!socket.connect(zmq_address)) {
        printf("Error connecting to worker");
    }
    //  Configure socket to not wait at close time
    int linger = 0;
    socket.setLinger(linger);
}

ZmqMsg WorkerClientBase::sendTX(string payload) {
    ZmqMsg request(payload);
    ZmqMsg reply;
    PollItem item;
    PollItem items[1];

    int global_retries = REQUEST_RETRIES;
    int retries_left = global_retries;
    int request_timeout = REQUEST_TIMEOUT;
    bool expect_reply = true;
    while(retries_left) {
        printf("%d %s %s %s", retries_left, getName(), zmq_address, payload);
        printf("Sending data to worker %d bytes ...", StringLen(request.getData()));
        socket.send(request);

        printf("Receiving data from worker. Sent payload : %s", payload);

        expect_reply = true;
        socket.fillPollItem(item, ZMQ_POLLIN);
        items[0] = item;
        while(expect_reply) {
            socket.poll(items, request_timeout);
            //got reply
            if ((items[0].revents & ZMQ_POLLIN) == 1) {
                // Get the reply.
                socket.recv(reply);
                printf("Received Response, %d bytes", reply.size());
                if(reply.size() > 0) {
                        printf("reply body: %s",reply.getData());
                        return reply;
                } else
                    printf("Malformed reply from worker");
            } else if(--retries_left == 0) {
                printf("Worker seems to be offline, abandoning ...");
                expect_reply = false;
                break;
            } else {
                printf("No response from worker, retrying ... ");
                printf("Retries left: %d", retries_left);
                delete(socket);
                initZMQ();
                connect();
                socket.send(request);
            }
        }
    }
    return reply;
}
//end of lazy pirate client code

WorkerA::WorkerA(Context &c, string addr):WorkerClientBase(c) {
    setAddr(addr);
    connect();
}

WorkerB::WorkerB(Context &c, string addr):WorkerClientBase(c) {
    setAddr(addr);
    connect();
}

void WorkerA::txSomething() {
    string payload = "#this is a R script";
    sendTX(payload);
}

void WorkerB::txSomething() {
    string payload = "{ \"some_json_log\": { \"SYMBOL\": \"EURUSD\", \"MAGIC\": \"42\", \"etc ...\":\"etc\" }}";
    sendTX(payload);
}

//+------------------------------------------------------------------+
//| Expert initialization function                                   |
//+------------------------------------------------------------------+
Context *sharedContext;
WorkerA *workerA;
WorkerB *workerB;

int OnInit()  {
   sharedContext = new Context("ECHOTEST");
   workerA = new WorkerA(sharedContext, "tcp://localhost:5555");
   workerB = new WorkerB(sharedContext, "tcp://localhost:5566");

   return(INIT_SUCCEEDED);
}
//+------------------------------------------------------------------+
//| Expert deinitialization function                                 |
//+------------------------------------------------------------------+
void OnDeinit(const int reason) {
    delete(workerA);
    delete(workerB);
    delete(sharedContext);
}

//+------------------------------------------------------------------+
//| Expert tick function                                             |
//+------------------------------------------------------------------+
int tickC = 0;
void OnTick() {
    printf("On tick loop number: %d", tickC);
    workerA.txSomething();
    workerB.txSomething();
    tickC++;
}
//+------------------------------------------------------------------+
