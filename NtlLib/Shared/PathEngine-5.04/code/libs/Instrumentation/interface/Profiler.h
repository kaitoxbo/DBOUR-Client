//**********************************************************************
//
// Copyright (c) 2002
// PathEngine
// Lyon, France
//
// All Rights Reserved
//
//**********************************************************************

#ifndef PROFILER_INCLUDED
#define PROFILER_INCLUDED

#include "platform_common/Timer.h"

const long PROFILER_TIMER_SHIFT = 8;

class cProfile;

#include "common/STL/vector.h"

class cProfile
{
public:
    cProfile() {}
    void init(const char* id)
    {
        m_children.clear();
        m_id=id;
        m_time=0.0;
        m_count=0;
    }
    const char* m_id;
    double m_time;
    long m_count;
    std::vector<cProfile> m_children;
    cProfile& refChild(const char* id);
    void addRemainderEntries();
};

const long PROFILE_OFFSET_LOW = 0;
const long PROFILE_OFFSET_EXIT_SIZE = 1;
const long PROFILE_OFFSET_ID = 1;
const long PROFILE_OFFSET_ENTRY_SIZE = 1 + sizeof(const char*) / sizeof(unsigned long);
const long PROFILE_OFFSET_MAXIMUM_SIZE = PROFILE_OFFSET_ENTRY_SIZE;

class cProfileBuffer
{
    std::vector<unsigned long> m_buffer;
    long m_pos;
    long m_last_valid_pos;

    unsigned long m_start_high, m_start_low;
    bool m_was_stopped;
    unsigned long m_stop_high, m_stop_low;

    long build(long start_pos, cProfile &profile, unsigned long &overflow_counter);

    bool isExit(long pos);
    double getTime(long pos);
    void incrementPastEntry(long &pos);
    void incrementPastExit(long &pos);
    void incrementPastEntry(long &pos, unsigned long &overflow_counter);
    void incrementPastExit(long &pos, unsigned long &overflow_counter);
    const char* getID(long pos);
    bool isEndMarker(long pos);
public:
    
    cProfileBuffer();
    void start(long requested_buffer_size);
    void stop();

    bool bufferFull() const;

    bool bufferFilledBeforeStop() const
    {
        return bufferFull() && !m_was_stopped;
    }

    double getTotalTime() const;

    void sectionEnters(const char* id)
    {
        if(m_pos <= m_last_valid_pos)
        {
            unsigned long high, low;
            DoRDTSC(high, low);
            low = ((low >> PROFILER_TIMER_SHIFT) | (high << (32 - PROFILER_TIMER_SHIFT)));
            m_buffer[m_pos + PROFILE_OFFSET_LOW] = (low & 0xfffffffe);
            *reinterpret_cast<const char**>(&m_buffer[m_pos + PROFILE_OFFSET_ID]) = id;
            m_pos += PROFILE_OFFSET_ENTRY_SIZE;
        }
    }
    void sectionExits()
    {
        if(m_pos <= m_last_valid_pos)
        {
            unsigned long high, low;
            DoRDTSC(high, low);
            low = ((low >> PROFILER_TIMER_SHIFT) | (high << (32 - PROFILER_TIMER_SHIFT)));
            m_buffer[m_pos+PROFILE_OFFSET_LOW] = (low | 1);
            m_pos+=PROFILE_OFFSET_EXIT_SIZE;
        }
    }

    void build(cProfile &profile);

    static cProfileBuffer* _theInstance;
    static void initInstance();
    static void destroyInstance();
};

#ifdef PROFILING_ON

class cProfilerScope
{
public:
    cProfilerScope(const char* id)
    {
        cProfileBuffer::_theInstance->sectionEnters(id);
    }
    ~cProfilerScope()
    {
        cProfileBuffer::_theInstance->sectionExits();
    }
};

#define PROFILER_ENTERS(id) do{cProfileBuffer::_theInstance->sectionEnters(id);}while(0)
#define PROFILER_EXITS do{cProfileBuffer::_theInstance->sectionExits();}while(0)
#define PROFILER_CHANGES(id_after) \
do{\
    cProfileBuffer::_theInstance->sectionExits();\
    cProfileBuffer::_theInstance->sectionEnters(id_after);\
}while(0)
#define PROFILER_SCOPE(id) cProfilerScope profiler_scope_object(id);

#else
    
#define PROFILER_ENTERS(id) do{}while(0)
#define PROFILER_EXITS do{}while(0)
#define PROFILER_CHANGES(id_after) do{}while(0)
#define PROFILER_SCOPE(id) do{}while(0)

#endif

#define PROFILER_START(buffer_size) do{cProfileBuffer::_theInstance->start(buffer_size);}while(0)
#define PROFILER_STOP do{cProfileBuffer::_theInstance->stop();}while(0)

#endif // include guard PROFILER_INCLUDED

