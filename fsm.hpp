#pragma once
#include <cstdint>
#include <cstring>

#define STATECOUNT 10                                   // 最大狀態數量
#define STATE_CONNECTCOUNT STATECOUNT *(STATECOUNT - 1) // 最大狀態關係數量
class FSM
{
    typedef struct
    {
        uint8_t id;
        const char *name;
        void (*handle)();
    } stateData_t;
    typedef struct
    {
        uint8_t fromID;
        uint8_t toID;
        bool (*reason)();
    } stateTrans_t;
    stateTrans_t stateTransForm[STATE_CONNECTCOUNT];
    stateData_t stateForm[STATECOUNT] = {0};
    uint8_t currentID = 0xff;
    uint16_t transCount = 0;

public:
    bool stateCreat(const char *name, void (*func)())
    {
        for (int i = 0; i < STATECOUNT; i++)
        {
            if (stateForm[i].name == nullptr)
            {
                stateForm[i].id = i;
                stateForm[i].name = name;
                stateForm[i].handle = func;
                return true;
            }
        }
        return false;
    }
    bool stateDelete(const char *name)
    {
        for (int i = 0; i < STATECOUNT; i++)
        {
            if (stateForm[i].name && !strcmp(stateForm[i].name, name))
            {
                stateForm[i].name = nullptr;
                stateForm[i].handle = nullptr;
                return true;
            }
        }
        return false;
    }
    bool stateSetTrans(const char *from, const char *to, bool (*func)())
    {
        uint8_t fromId = 0xff, toId = 0xff;
        for (int i = 0; i < STATECOUNT; i++)
        {
            if (stateForm[i].name && !strcmp(stateForm[i].name, from))
                fromId = i;
            if (stateForm[i].name && !strcmp(stateForm[i].name, to))
                toId = i;
        }
        if (fromId == 0xFF || toId == 0xFF)
            return false;
        stateTransForm[transCount] = {fromId, toId, func};
        transCount++;
        return true;
    }

    void start(const char *startName)
    {
        for (int i = 0; i < STATECOUNT; i++)
        {
            if (stateForm[i].name && !strcmp(stateForm[i].name, startName))
            {
                currentID = i;
                return;
            }
        }
        currentID = 0xff;
    }
    void stateUpdate()
    {
        if (currentID == 0xff)
            return;
        for (int i = 0; i < transCount; i++)
        {
            if (stateTransForm[i].fromID == currentID && stateTransForm[i].reason && stateTransForm[i].reason())
            {
                currentID = stateTransForm[i].toID;
                break;
            }
        }
        if (stateForm[currentID].handle)
            stateForm[currentID].handle();
    }
};
