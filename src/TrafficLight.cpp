#include <iostream>
#include <random>
#include "TrafficLight.h"
#include <chrono>

/* Implementation of class "MessageQueue" */

 
template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 

    // perform work under the lock
    std::unique_lock<std::mutex> Lock(_mutex);

    // pass unique lock to condition variable
    _condition.wait(Lock, [this] { return !_queue.empty(); });

    // remove last element from queue
    T msg = std::move(_queue.back());
    _queue.clear();    

    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    
    // perform work under the lock
    std::lock_guard<std::mutex> Lock(_mutex);

    // add message to the queue
    _queue.push_back(std::move(msg));

    // notify client after pushing message
    _condition.notify_one();
}


/* Implementation of class "TrafficLight" */

 
TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.

    while(true){
        if(_message.receive() == TrafficLightPhase::green){
            return;
        }        
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class.
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

//Function to generate random number
long CalculateCycleDuration(){
    return 4000 + ( std::rand() % ( 6000 - 4000 + 1 ) ); //credit: https://stackoverflow.com/questions/7560114/random-number-c-in-some-range
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 

    // init stop watch
    std::chrono::time_point<std::chrono::system_clock> lastUpdate;
    lastUpdate = std::chrono::system_clock::now();

    //determine inital cycle duration
    long CycleDuration = CalculateCycleDuration();

    while(true){        

        // apply delay
        std::this_thread::sleep_for(std::chrono::milliseconds(1));       

        // compute time difference to stop watch
        long timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - lastUpdate).count();

        if(timeSinceLastUpdate >= CycleDuration){
            
            if(_currentPhase == TrafficLightPhase::green){
                // set traffic light to red
                _currentPhase = TrafficLightPhase::red; 
            }
            else
            {
                // set traffic light to green
                _currentPhase = TrafficLightPhase::green;
            }                  
            
            // reset stop watch for next cycle
            lastUpdate = std::chrono::system_clock::now();

            // calculate next cycle duration
            CycleDuration = CalculateCycleDuration();
        }

        // send update method to message queue
        _message.send(std::move(_currentPhase));
    }
}

