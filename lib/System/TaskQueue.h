#ifndef TASK_QUEUE_H_
#define TASK_QUEUE_H_

#include <list>

template <typename T> class TaskQueue
{
    public:
        TaskQueue()
        {
        }

        void addElement(T elem)
        {
            m_elements.push_back(elem);
        }

        T getElement()
        {
            T elem = m_elements.front();
            m_elements.pop_front();
            return elem;
        }

        bool empty() const
        {
            return m_elements.empty();
        }

    private:
        std::list<T> m_elements;
};

#endif
