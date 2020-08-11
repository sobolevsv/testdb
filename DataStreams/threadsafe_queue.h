#pragma once

#include <mutex>
#include <condition_variable>
#include <queue>
#include <Common/Exception.h>


template<class T>
class threadsafe_queue {
public:

    int size() { return m_msg_queue.size(); }

    void close() {
        closed = true;
        m_not_empty.notify_all();
    }

    void push(T item) {
        std::unique_lock<std::mutex> l(m_lock);
        m_msg_queue.push(item);
        m_not_empty.notify_one();
    }

    T pop() {
        if(closed && m_msg_queue.empty()) {
            throw  Exception("stream is closed and empty");
        }

        std::unique_lock<std::mutex> l(m_lock);
        m_not_empty.wait(l, [this](){return this->m_msg_queue.size() != 0; });
        T item  = m_msg_queue.front();
        m_msg_queue.pop();
        return item;
    }

    bool pop(T &res) {
        std::unique_lock<std::mutex> l(m_lock);
        if(!m_msg_queue.empty()){
            res = m_msg_queue.front();
            m_msg_queue.pop();
            return true;
        }
        return false;
    }

    bool isClosed() const {
        return closed;
    }

    operator bool() const{
        return !isClosed() || !empty();
    }

    bool empty() const {
        return m_msg_queue.empty();
    }

private:
    std::queue<T>           m_msg_queue;
    std::mutex              m_lock;
    std::condition_variable m_not_empty;
    bool closed = false;
};
