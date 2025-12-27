#include "request_queue.h"

using namespace std;
RequestQueue::RequestQueue() {
   //requests.reset(new std::vector<std::unique_ptr<std::unique_ptr<Request*>>());
}

void RequestQueue::push(std::unique_ptr<Request> req) {
    std::lock_guard lock(mtx);
    requests->push(req);
}

std::unique_ptr<Request> RequestQueue::pop() {
    std::lock_guard lock(mtx);
    std::unique_ptr<Request> req_ptr = std::move(requests->front());
    requests->pop();
    return req_ptr;
}