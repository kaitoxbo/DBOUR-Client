//**********************************************************************
//
// Copyright (c) 2004
// PathEngine
// Lyon, France
//
// All Rights Reserved
//
//**********************************************************************

#ifndef SWAP_OUT_MIRROR_INCLUDED
#define SWAP_OUT_MIRROR_INCLUDED

#include "i_SwapOutSubscriber.h"
#include "SwapOut.h"
#include "common/Containers/ReplacementList.h"
#include "common/STL/vector.h"

class cSwapOutPublisher
{
    cList_NoSize<iSwapOutSubscriber*> _subscribers;
    long _currentSize;

public:

    cSwapOutPublisher()
    {
        _currentSize = 0;
    }
    ~cSwapOutPublisher()
    {
        assertD(_subscribers.empty());
        assertD(_currentSize == 0);
    }

    cList_NoSize<iSwapOutSubscriber*>::iterator addSubscriber(iSwapOutSubscriber* ptr)
    {
        long i;
        for(i = 0; i < _currentSize; ++i)
        {
            ptr->pushBack();
        }
        _subscribers.push_front(ptr);
        return _subscribers.begin();
    }

    void swapOut(long index)
    {
        cList_NoSize<iSwapOutSubscriber*>::iterator i;
        for(i = _subscribers.begin(); i != _subscribers.end(); ++i)
        {
            (*i)->swapOut(index);
        }
        _currentSize--;
    }
    void pushBack()
    {
        cList_NoSize<iSwapOutSubscriber*>::iterator i;
        for(i = _subscribers.begin(); i != _subscribers.end(); ++i)
        {
            (*i)->pushBack();
        }
        _currentSize++;
    }
};


template <class T>
class cPointerSwapOutHandler : public iSwapOutSubscriber
{
    std::vector<T*>& _vector;

public:

    cPointerSwapOutHandler(std::vector<T*>& vector) :
      _vector(vector)
    {
    }

// iSwapOutSubscriber

    void swapOut(long index)
    {
        delete _vector[index];
        SwapOut(_vector, index);
    }
    void pushBack()
    {
        _vector.push_back(0);
    }
};

template <class T>
class cPointerSwapOutMirror
{
    std::vector<T*> _vector;
    cList_NoSize<iSwapOutSubscriber*>::iterator _node;

public:

    cPointerSwapOutMirror(cSwapOutPublisher& publisher)
    {
        iSwapOutSubscriber* handler = new cPointerSwapOutHandler<T>(_vector);
        _node = publisher.addSubscriber(handler);
    }
    ~cPointerSwapOutMirror()
    {
        iSwapOutSubscriber* handler = *_node;
        while(!_vector.empty())
        {
            handler->swapOut(_vector.size() - 1);
        }
        _node.erase();
        delete handler;
    }

    T*& operator[](long index)
    {
        return _vector[index];
    }
    T*const& operator[](long index) const
    {
        return _vector[index];
    }
    long size() const
    {
        return _vector.size();
    }
};

class cCheckedIndexSwapOutMirror : public iSwapOutSubscriber
{
    std::vector<long> _vector;
    cList_NoSize<iSwapOutSubscriber*>::iterator _node;

public:

    cCheckedIndexSwapOutMirror(cSwapOutPublisher& publisher)
    {
        _node = publisher.addSubscriber(this);
    }
    ~cCheckedIndexSwapOutMirror()
    {
        _node.erase();
        long i;
        for(i = 0; i < SizeL(_vector); ++i)
        {
            assertD(_vector[i] == -1);
        }
    }

    long& operator[](long index)
    {
        return _vector[index];
    }
    const long& operator[](long index) const
    {
        return _vector[index];
    }
    long size() const
    {
        return SizeL(_vector);
    }

// iSwapOutSubscriber

    void swapOut(long index)
    {
        assertD(_vector[index] == -1);
        SwapOut(_vector, index);
    }
    void pushBack()
    {
        _vector.push_back(-1);
    }
};

template <class T, T initialiseTo>
class cSwapOutMirror : public iSwapOutSubscriber
{
    std::vector<T> _vector;
    cList_NoSize<iSwapOutSubscriber*>::iterator _node;

public:

    cSwapOutMirror(cSwapOutPublisher& publisher)
    {
        _node = publisher.addSubscriber(this);
    }
    ~cSwapOutMirror()
    {
        _node.erase();
    }

    T& operator[](long index)
    {
        return _vector[index];
    }
    const T& operator[](long index) const
    {
        return _vector[index];
    }
    long size() const
    {
        return _vector.size();
    }

// iSwapOutSubscriber

    void swapOut(long index)
    {
        SwapOut(_vector, index);
    }
    void pushBack()
    {
        // COMPILERBUG? VC6
        //_vector.push_back(initialiseTo); <- wont accept this
        const T toPushBack = initialiseTo;
        _vector.push_back(toPushBack);
    }
};

#endif
