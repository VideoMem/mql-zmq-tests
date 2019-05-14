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
    worker_add(name.c_str(), name.size(), addr.c_str(), addr.size(), handle);
    worker_setRequestTimeout(handle, 5000);

    name = "WorkerB";
    addr = "tcp://localhost:5566";
    worker_add(name.c_str(), name.size(), addr.c_str(), addr.size(), handle);
}

void OnTick() {
    string rpay = rPay();
    string jsonpay = jsonPay();
    worker_echo(0, rpay.c_str(), rpay.size());
    worker_echo(1, jsonpay.c_str(), jsonpay.size());
}

int main(int argc, char** argv) {

  printf("%d\n", int_echo(42));
  wchar_t reply[1024];
  wchar_t source[] = L"String echo test\n";
  size_t size = string_echo(source, reply);
  wprintf(reply);

  char hello[] = "Char echo test\n";
  string hell = hello;
  int strlen = hell.size();
  char creply[1024] = {0};
  char_echo(hello, creply, strlen);
  printf("%s", creply);

  printf("Init Sucess!\n");
  printf("reply len: %d\n", size);
  OnInit();
  while(true) {
    OnTick();
  }
  return EXIT_SUCCESS;
}
