#ifndef EVENTHDL_H
#define EVENTHDL_H

#include <arduino.h>

#define EVENT_QUEUE_SIZE 16
#define MAX_LISTENERS 16

#define signed_byte int8_t // equivalent to 'char' but more clear

class EventQueue
{
public:
    EventQueue() : m_array(), m_size(0), m_front(-1), m_back(-1) {}
    signed_byte pop(); // Returns the popped element's value (front of queue).
    void enqueue(signed_byte event_code);
    bool empty() const;
    byte size() const {return m_size;}

private:
    struct Node
    {
        Node() : value(-1), prev(-1), next(-1) {}
        Node(signed_byte v) : value(v), prev(-1), next(-1) {}
        signed_byte value;
        // prev and next store the array indeces of the previous and next elements (prev points towards the front, next points towards the back. Front element has prev -1, back element has next -1).
        signed_byte prev;
        signed_byte next;
    };
    // Empty array elements have value -1.
    Node m_array[EVENT_QUEUE_SIZE];
    byte m_size;
    signed_byte m_front;
    signed_byte m_back;
};

class EventHandler
{
public:
    EventHandler() : m_listeners_list(), m_listener_num(0) {}
    void enqueueEvent(byte event_code);
    void processEvent();		// Prcesses event in the front of the queue.
    void listen();				// Runs all the listeners and adds events to the queue if event happens (but does not process them).
    // listenFunc is a function that returns true if the event being listened to happens. callbackFunc is the function to be called when the event happens.
    void addListener(bool (*listenFunc)(), void (*callbackFunc)());

private:
    struct Listener
    {
        Listener() : event_code(-1), listenFunc(nullptr), callbackFunc(nullptr) {}
        Listener(byte code, bool (*listF)(), void (*cbF)()) : event_code(code), listenFunc(listF), callbackFunc(cbF) {}
        byte event_code;
        bool (*listenFunc)(); // Listening function must take no parameters and return a bool.
        void (*callbackFunc)(); // Event functions must take no parameters and return void.
    };
    Listener m_listeners_list[MAX_LISTENERS];
    byte m_listener_num;
    EventQueue m_eventq;

};

#endif // EVENTHDL_H
