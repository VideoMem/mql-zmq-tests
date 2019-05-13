#include "lpclient.hpp"

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
    string name = "WorkerA";
    string addr = "tcp://localhost:5555";
    string_t n[string_size(name)] = {0};
    string_t a[string_size(addr)] = {0};
    string_unmarshall(name,n);
    string_unmarshall(addr,a);
    printf("workerAdd\n");
    worker_add(n, a, handle);
    worker_setRequestTimeout(handle, 5000);

    name = "WorkerB";
    addr = "tcp://localhost:5566";
    string_unmarshall(name,n);
    string_unmarshall(addr,a);
    worker_add(n,a, handle);
}

void OnTick() {
    string r = rPay();
    string m = jsonPay();
    string_t rpay[string_size(r)] = {0};
    string_t jsonpay[string_size(m)] = {0};
    string_unmarshall(r, rpay);
    string_unmarshall(m, jsonpay);
    worker_echo(0, rpay);
    worker_echo(1, jsonpay);
}

int main(int argc, char** argv) {
  printf("%d\n", int_echo(42));
  string_t reply[1024];
  string_t source[] = L"Echo Test\n";
  size_t size = string_echo(source, reply);
  printf("Init Sucess!\n");
  wprintf(reply);
  printf("reply len: %d\n", size);
  OnInit();
  while(true) {
    OnTick();
  }
  return EXIT_SUCCESS;
}
