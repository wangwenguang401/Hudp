#ifndef HEADER_NET_SOCKET
#define HEADER_NET_SOCKET

#include <atomic>
#include "CommonType.h"
#include "TimerSolt.h"

namespace hudp {

    static const uint8_t __wnd_size  = 3;

    enum WndIndex {
        WI_ORDERLY          = 0,
        WI_RELIABLE         = 1,
        WI_RELIABLE_ORDERLY = 2
    };

    class CSendWnd;
    class CRecvList;
    class CPriorityQueue;
    class CIncrementalId;
    class NetMsg;
    class CSenderRelialeOrderlyNetMsg;
    class CPendAck;

    class CSocket : public CTimerSolt {
    public:
        CSocket();
        ~CSocket();

        // get msg from pri queue. 
        // not blocked. return null if empty.
        NetMsg* GetMsgFromPriQueue();

        // send msg to module of socket.
        void SendMsgToPriQueue(NetMsg* msg);
        void SendMsgToSendWnd(NetMsg* msg);
        void SendMsgToNet(NetMsg* msg);

        // recv msg to module of socket.
        void RecvMsgUpper(NetMsg* msg);
        void RecvMsgToOrderList(NetMsg* msg);

        // timer call back
        void OnTimer();

        // set RTT time. a msg will be send again in timer.
        void SetTimerOutTime(uint16_t timer_out);
        // add a msg to timer.
        void AddToTimer(CSenderRelialeOrderlyNetMsg* msg);

    private:
        // Instantiating corresponding classes.
        void CreateSendWnd(WndIndex index);
        void CreateRecvList(WndIndex index);
        void CreatePendAck(WndIndex index);

    private:
        CIncrementalId* _inc_id[__wnd_size];
        CSendWnd*       _send_wnd[__wnd_size];
        CRecvList*      _recv_list[__wnd_size];
        CPendAck*       _pend_ack[__wnd_size];
        CPriorityQueue* _pri_queue;
        std::atomic<uint16_t> _timer_out_time; //default 50ms
    };
}

#endif