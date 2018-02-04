/*
 * Copyright © 2017-2018 Steven Beyermann, Markus Blechschmidt, Moritz Höwer,
 * Lasse Lüder, Andre Radtke
 *
 * This software is licensed by MIT License.
 * See LICENSE for details.
 */
/** 
 * @file
 * \addtogroup Broking
 * @{
 */

#ifndef BROKING_CHANNEL_H_
#define BROKING_CHANNEL_H_

// Save LOG_MODULE if it was defined before this header
#pragma push_macro("LOG_MODULE")
#undef LOG_MODULE

#define LOG_MODULE "broking"
#include "util/logging/logging.h"

#include "broking/AbstractChannelBase.h"
#include "broking/BufferedSubscription.h"
#include "broking/ThreadSafeQueue.h"
#include <functional>
#include <mutex>
#include <thread>
#include <tuple>
#include <condition_variable>
#include <map>
#include <stdexcept>

namespace broking {

/**
 * Default size for subscriber buffers
 */
constexpr int DEFAULT_BUFFERSIZE = 5;

/**
 * size of queue used for publishing.
 */
constexpr int PUBLISHING_QUEUE_SIZE = 5;

/**
 * Describes the severity of a message drop
 */
enum class Severity {
    WARNING, ERROR
};
std::ostream& operator<<(std::ostream& os, const Severity& s);

/**
 * Generic asynchronous channel for message passing.
 *
 * @author  Moritz Höwer (Moitz.Hoewer@haw-hamburg.de)
 * @version 1.0
 */
template<typename T> class Channel: public AbstractChannelBase {
private:
    bool run; ///< flag for the processing loop
    std::thread processingThread; ///< handle for the processing thread
    std::mutex mtxProcessingWait; ///< mutex to coordinate blocking
    std::mutex mtxSubscribers; ///< mutex to coordinate access to the subscribers
    std::condition_variable cvProcessingWait; ///< condition variable to wait on
    ThreadSafeQueue<std::tuple<T, Severity>> publishingQueue; ///< buffers published messages
    std::map<int, std::function<bool(T)>> subscribers; ///< stores the subscribers
    std::string name; ///< stores the name of the channel
public:
    Channel(std::string name);

    // Prevent moving and copying
    /**
     * Delete Copy-Constructor
     */
    Channel(const Channel&) = delete;

    /**
     * Delete Move-Constructor
     */
    Channel(Channel&&) = delete;

    /**
     * Delete Copy-Assignment
     */
    Channel& operator=(const Channel&) = delete;

    /**
     * Delete Move-Assignment
     */
    Channel& operator=(Channel&&) = delete;

    virtual ~Channel();

    void processingLoop();

    void publish(T message, Severity severity = Severity::ERROR);
    Subscription subscribe(std::function<void(T)> callback, bool persistent = false);
    BufferedSubscription<T> subscribe(int buffersize = DEFAULT_BUFFERSIZE);
    void unsubscribe(const Subscription& subscription) override;
    std::string getName();

};

/**
 * Kind of hacky "singleton" - defined in Broker.cpp
 */
extern Channel<std::string> WARNING_CHANNEL;

/**
 * Constructs a Channel<T>.
 *
 * @param name the name of the channel.
 */
template<typename T>
inline Channel<T>::Channel(std::string name) :
        run(true), publishingQueue(PUBLISHING_QUEUE_SIZE), name(name) {
    LOG_TRACE<< "Constructing Channel with T=" << typeid(T).name() << std::endl;
    processingThread = std::thread(&Channel::processingLoop, this);
}

/**
 * Destructs a Channel.
 */
template<typename T>
inline Channel<T>::~Channel() {
    LOG_TRACE<< "Destructing Channel with T=" << typeid(T).name() << std::endl;

    // stop processing thread and join it for clean exit
    run = false;
    cvProcessingWait.notify_all();// will quite likely be sleeping
    processingThread.join();
}

/**
 * Processing loop - run in a separate thread.
 */
template<typename T>
inline void Channel<T>::processingLoop() {
    LOG_SCOPE

    // create a unique_lock, but don't lock the mutex yet.
    std::unique_lock<std::mutex> lock(mtxProcessingWait, std::defer_lock);

    while (run) {
        LOG_TRACE<< "Processing..." << std::endl;

        while(publishingQueue.canDequeue()) {
            // there is a message

            // "unpack" tuple
            Severity severity;
            T message;
            std::tie(message, severity) = publishingQueue.dequeue();

            std::lock_guard<std::mutex>lock(mtxSubscribers);
            for(auto&& subscriber : subscribers) {
                // subscriber.second is the lambda
                // call lambda with the message
                bool successfull = subscriber.second(message);

                // if lambda returned false, the message was dropped
                if(!successfull) {
                    if(severity == Severity::ERROR) {
                        LOG_ERROR << "Dropped critical Message on Channel \""
                        << name << "\" - Subscriber "
                        << subscriber.first << " didn't accept!"
                        << std::endl;

                        throw std::runtime_error(
                                "Dropped critical message on Channel \""
                                + name + "\"");
                    } else {
                        WARNING_CHANNEL.publish("Dropped a message on Channel \""
                                + name + "\" - Subscriber "
                                + std::to_string(subscriber.first)
                                + " didn't accept...");
                    }
                }
            }
        }
        // no more messages -> go to blocked and free CPU
        lock.lock();
        cvProcessingWait.wait(lock);
        lock.unlock();
    }
}

/**
 * Publish a message on the Channel.
 *
 * @param message the message to publish
 * @param severity the Severity if the message is dropped.
 * @attention this WILL block if the publishing queue is full!
 */
template<typename T>
inline void Channel<T>::publish(T message, Severity severity) {
    LOG_TRACE<< "Publishing " << message << std::endl;
    publishingQueue.enqueue(std::make_tuple(message, severity));

    // wakeup processing thread (in case it was sleeping)
    // because now there is a message to process
    cvProcessingWait.notify_all();
}

/**
 * Subscribe a callback on the Channel.
 * @attention Callbacks are processed SYNCHRONOUSLY by the processing thread - keep it short!
 *
 * @param callback the callback to subscribe
 * @return a Subscription to identify this later
 */
template<typename T>
inline Subscription Channel<T>::subscribe(std::function<void(T)> callback, bool persistent) {
    std::lock_guard<std::mutex> lock(mtxSubscribers);

    // create a new subscription
    Subscription s(*this, persistent);

    // wrap callback in a lambda - will always return true, because the callback
    // can't drop the message.
    // the lambda is then stored in the subscriber map, with the subscription as
    // it's key
    subscribers[s.getID()] = [callback](T message) {callback(message); return true;};

    return s;
}

/**
 * Subscribe a buffer on the Channel.
 *
 * @param buffersize the size of the buffer
 * @return a BufferedSubscription to identify this later and to provide access to the buffer.
 */
template<typename T>
inline BufferedSubscription<T> Channel<T>::subscribe(int buffersize) {
    // create the buffer
    auto buffer = std::make_shared<ThreadSafeQueue<T>>(buffersize);

    std::lock_guard<std::mutex> lock(mtxSubscribers);

    // create a subscription that is not persistent
    Subscription s(*this, false);

    // create a lambda that captures the buffer and wraps it's tryEnqueue operation
    // if tryEnqueue fails, it drops the message and returns false, which is
    // exactly what the processing thread expects.
    // the lambda is then stored in the subscriber map, with the subscription as
    // it's key
    subscribers[s.getID()] = [buffer](T message) {return buffer->tryEnqueue(message);};

    // wrap Subscription and buffer in a BuferedSubscription
    return BufferedSubscription<T>(std::move(s), buffer);
}

/**
 * Unsubscribe from the channel.
 *
 * @param subscription the Subscription to unsubscribe
 */
template<typename T>
inline void Channel<T>::unsubscribe(const Subscription& subscription) {
    std::lock_guard<std::mutex> lock(mtxSubscribers);
    subscribers.erase(subscription.getID());
}

/**
 * Get the name of the channel
 * @return the name of the channel, as given in the constructor
 */
template<typename T>
inline std::string Channel<T>::getName() {
    return name;
}

/**
 * Print the Severity to an ostream.
 *
 * @param os the std::ostream to print to
 * @param s the Severity to print
 *
 * @return os
 */
inline std::ostream& operator <<(std::ostream& os, const Severity& s) {
    switch (s) {
    case Severity::WARNING:
        os << "WARNING";
        break;
    case Severity::ERROR:
        os <<  "ERROR";
        break;
    }
    return os;
}

} /* namespace broking */

// Restore LOG_MODULE if it was defined before this header
#undef LOG_MODULE
#pragma pop_macro("LOG_MODULE")

#endif /* BROKING_CHANNEL_H_ */
/** @} */
