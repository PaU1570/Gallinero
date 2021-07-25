#include "EventHandler.h"

// --------------------------------------------------------------------- //
// --					EVENT HANDLER IMPLEMENTATION				  -- //
// --------------------------------------------------------------------- //

void EventHandler::addListener(bool (*listenFunc)(), void (*callbackFunc)())
{
    if (m_listener_num == MAX_LISTENERS)
        return;

    byte event_code = m_listener_num;
    
    Listener lis(event_code, listenFunc, callbackFunc);
    m_listeners_list[m_listener_num] = lis;
    m_listener_num++;
}

void EventHandler::listen()
{
    for (byte i = 0; i < m_listener_num; i++)
    {
        if (m_listeners_list[i].listenFunc())
            m_eventq.enqueue(m_listeners_list[i].event_code);
    }
}

void EventHandler::enqueueEvent(byte event_code)
{
    m_eventq.enqueue(event_code);
}

void EventHandler::processEvent()
{
    if (m_eventq.empty())
        return;

    byte event_code = m_eventq.pop();
    // Find corresponding listener.
    byte i;
    // Assuming that the event is in the list (otherwise infinite loop)
    for (i = 0; i < MAX_LISTENERS; i++)
    {
        if (m_listeners_list[i].event_code == event_code)
        {
            m_listeners_list[i].callbackFunc();
            break;
        }
    }
}



// --------------------------------------------------------------------- //
// --					EVENT QUEUE IMPLEMENTATION					  -- //
// --------------------------------------------------------------------- //

signed_byte EventQueue::pop()
{
    if (m_size == 0)
        return -1;
    
    signed_byte val = m_array[m_front].value;
    // Case where there is only one element left:
    if (m_size == 1)
    {
        m_array[m_front] = Node();
        m_front = -1;
        m_back = -1;
    }
    else
    {
        signed_byte target = m_front;
        m_front = m_array[m_front].next;
        m_array[m_front].prev = -1;
        m_array[target] = Node();
    }
    m_size--;
    
    return val;
}

void EventQueue::enqueue(signed_byte event_code)
{
    if (m_size == EVENT_QUEUE_SIZE)
        return;
    
    // Find first empty spot.
    byte i;
    for (i = 0; m_array[i].value != -1; i++)
    {
    }
    
    Node n(event_code);
    // Case where queue is empty:
    if (m_size == 0)
    {
        m_array[i] = n;
        m_front = i;
        m_back = i;
    }
    else
    {
        n.prev = m_back;
        m_array[m_back].next = i;
        m_back = i;
        m_array[i] = n;
    }
    m_size++;
}

bool EventQueue::empty() const
{
    return (m_size == 0);
}