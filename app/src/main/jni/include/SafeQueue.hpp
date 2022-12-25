#ifndef _SAFE_QUEUE_H_
#define _SAFE_QUEUE_H_

#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>

using namespace std;

template <typename T>
class SafeQueue {
    
    using ReleaseHandle = function<void(T&)>;
    using SyncHandle = function<void(queue<T>&)>;

public:
    SafeQueue(): work_flag(0) {}
    ~SafeQueue() {}

public:
    void push(T element) {
        unique_lock<mutex> lock(_mutex);
        if(work_flag) {
            que.push(element);
            _cond.notify_one();
        }else {
            release_handle(element);
        }
    }

    int pop(T & value) {
        int ret = 0;
        unique_lock<mutex> lock(_mutex);
        while(work_flag && que.empty()) {
            _cond.wait(lock);
        }
        if(!que.empty()) {
            value = que.front();
            que.pop();
            ret = 1;
        }
        return ret;
    }
    
    void setWork(int flag) {
        unique_lock<mutex> lock(_mutex);
        work_flag = flag;
        _cond.notify_one();
    }

    void setReleaseHandle(ReleaseHandle handle) {
        release_handle = handle;
    }

    void setSyncHandle(SyncHandle handle) {
        sync_handle = handle;
    }

    int empty() {
        return que.empty();
    }

    int size() {
        return que.size();
    }

    void clear() {
        unique_lock<mutex> lock(_mutex);
        int size = que.size();
        for(int i=0; i < size; ++i) {
            T value = que.front();
            que.pop();
            release_handle(value);
        }
    }
    
    void sync() {
        unique_lock<mutex> lock(_mutex);
        sync_handle(que);
    }

private:
    condition_variable _cond;
    mutex _mutex;
    queue<T> que;
    int work_flag;
    ReleaseHandle release_handle;
    SyncHandle sync_handle;

};

#endif // _SAFE_QUEUE_H_