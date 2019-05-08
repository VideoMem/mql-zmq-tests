//+------------------------------------------------------------------+
//|                                           multiworker_lpclient.mq4 |
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
        void init();
        void close();
        void connect();
    public:
        virtual string getName() { return "WorkerClientBase"; }
        void setAddr(string addr) { zmq_address = addr; }
        ZmqMsg sendTX(string payload);
        WorkerClientBase() { context = new Context(); }
        ~WorkerClientBase();
};

class WorkerA: public WorkerClientBase {
    public:
        WorkerA(string addr) { setAddr(addr); }
        string getName() { return "WorkerA"; }
        void txSomething();
};

class WorkerB: public WorkerClientBase {
    public:
        WorkerB(string addr) { setAddr(addr); }
        string getName() { return "WorkerB"; }
        void txSomething();
};

void WorkerClientBase::init() {
    socket = new Socket(context, ZMQ_REQ);
}

void WorkerClientBase::close() {
    socket.close();
    //delete socket;
}

WorkerClientBase::~WorkerClientBase() {
    close();
    delete socket;
    delete context;
}

//Start of lazy pirate pattern client TX adapted to MQL5 from
//sample code in c++ http://zguide.zeromq.org/cpp:lpclient
void WorkerClientBase::connect() {
    printf("I: connecting to server…");
    init();
    socket.connect(zmq_address);

    //  Configure socket to not wait at close time
    int linger = 0;
    socket.setLinger(linger);
}

ZmqMsg WorkerClientBase::sendTX(string payload) {
    ZmqMsg request(payload);
    ZmqMsg reply;
    int retries_left = REQUEST_RETRIES;

    while(retries_left) {
        connect();
        socket.send(request);

        bool expect_reply = true;
        while(expect_reply) {
            //  Poll socket for a reply, with timeout
            PollItem item;
            PollItem items[1];
            socket.fillPollItem(item, ZMQ_POLLIN);
            items[0] = item;
            socket.poll(items, REQUEST_TIMEOUT);

            //  If we got a reply, process it
            if ((items[0].revents & ZMQ_POLLIN) == 1) {
                //  We got a reply from the server
                socket.recv(reply);
                if(reply.size() > 0) {
                    printf("I: server replied OK (%d) bytes", reply.size());
                    retries_left = 0;
                    expect_reply = false;
                } else 
                    printf("Malformed reply from worker");
            }
            else
            if(--retries_left == 0) {
                printf("E: server seems to be offline, abandoning");
                expect_reply = false;
                break;
            } else {
                printf("No response from worker, retrying ... ");
                close();
                connect();
                socket.send(request);
            }
        }
        close();
    }
    return reply;
}
//end of lazy pirate client code

void WorkerA::txSomething() {
    string payload = "#this is a R script";
    printf("reply body: %s\n", sendTX(payload).getData());
}

void WorkerB::txSomething() {
    string payload = "{ \"some_json_log\": { \"SYMBOL\": \"EURUSD\", \"MAGIC\": \"42\", \"etc ...\":\"etc\" }}";
    printf("reply body: %s\n", sendTX(payload).getData());
}

//+------------------------------------------------------------------+
//| Expert initialization function                                   |
//+------------------------------------------------------------------+
WorkerA *workerA;
WorkerB *workerB;

int OnInit()  {
   workerA = new WorkerA("tcp://localhost:5555");
   workerB = new WorkerB("tcp://localhost:5566");

   return(INIT_SUCCEEDED);
}
//+------------------------------------------------------------------+
//| Expert deinitialization function                                 |
//+------------------------------------------------------------------+
void OnDeinit(const int reason) {
    delete(workerA);
    delete(workerB);
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
