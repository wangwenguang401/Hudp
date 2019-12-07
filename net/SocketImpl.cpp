#include <cstring>		//for memset

#include "SocketImpl.h"
#include "SendWnd.h"
#include "OrderList.h"
#include "PriorityQueue.h"
#include "IncrementalId.h"
#include "IMsg.h"
#include "HudpImpl.h"
#include "Timer.h"
#include "PendAck.h"
#include "Log.h"
#include "HudpConfig.h"
#include "CommonFlag.h"

using namespace hudp;

// this size may be a dynamic algorithm control
static const uint16_t __send_wnd_size = 5;

CSocketImpl::CSocketImpl(const HudpHandle& handle) : _handle(handle) {
    memset(_send_wnd, 0, sizeof(_send_wnd));
    memset(_recv_list, 0, sizeof(_recv_list));
    memset(_pend_ack, 0, sizeof(_pend_ack));
}

CSocketImpl::~CSocketImpl() {
    for (uint16_t i = 0; i < __wnd_size; i++) {
        if (_send_wnd[i]) {

            delete _send_wnd[i];
        }
        if (_recv_list[i]) {
            delete _recv_list[i];
        }
        if (_pend_ack[i]) {
            delete _pend_ack[i];
        }
    }
}

HudpHandle CSocketImpl::GetHandle() {
    return _handle;
}

void CSocketImpl::SendMessage(CMsg* msg) {
    auto header_flag = msg->GetHeaderFlag();
    if (header_flag & HTF_ORDERLY) {
        AddToSendWnd(WI_ORDERLY, msg);

    } else if (header_flag & HTF_RELIABLE) {
        AddToSendWnd(WI_RELIABLE, msg);

    } else if (header_flag & HTF_RELIABLE_ORDERLY) {
        AddToSendWnd(WI_RELIABLE_ORDERLY, msg);

    } else {
        CHudpImpl::Instance().SendMessageToNet(msg);
    }
}

void CSocketImpl::RecvMessage(CMsg* msg) {
    // get ack info
    GetAckToSendWnd(msg);
    // recv msg to orderlist
    bool done = false;
    uint16_t ret = 0;

    auto header_flag = msg->GetHeaderFlag();
    // reliable and orderly
    if (header_flag & HTF_RELIABLE_ORDERLY) {
        AddToRecvList(WI_RELIABLE_ORDERLY, msg);
        done = true;

        // only orderly
    } else if (header_flag & HTF_ORDERLY) {
        AddToRecvList(WI_ORDERLY, msg);
        done = true;

        // only reliable
    } else if (header_flag & HTF_ORDERLY) {
        AddToRecvList(WI_RELIABLE, msg);
        done = true;
    }

    // normal udp. 
    if (!done) {
        CHudpImpl::Instance().RecvMessageToUpper(_handle, msg);
    }
}

void CSocketImpl::ToRecv(CMsg* msg) {
    // send ack msg to remote
    base::LOG_DEBUG("[receiver] :receiver msg. id : %d", msg->GetId());
    AddAck(msg);
    CHudpImpl::Instance().RecvMessageToUpper(_handle, msg);
}

void CSocketImpl::ToSend(CMsg* msg) {
    // add to timer
    if (msg->GetHeaderFlag() & HTF_RELIABLE_ORDERLY || msg->GetHeaderFlag() & HTF_RELIABLE) {
        // TODO
    }

    CHudpImpl::Instance().SendMessageToNet(msg);
}

void CSocketImpl::AckDone(CMsg* msg) {
    // release msg here
    CHudpImpl::Instance().ReleaseMessage(msg);
}

void CSocketImpl::TimerOut(CMsg* msg) {
    if (!(msg->GetFlag() & msg_is_only_ack)) {
        msg->AddSendDelay();
    }

    // add ack 
    AddAckToMsg(msg);
    // send to net
    CHudpImpl::Instance().SendMessageToNet(msg);
}

void CSocketImpl::AddAck(CMsg* msg) {
    auto header_flag = msg->GetHeaderFlag();
    if (header_flag & HTF_RELIABLE_ORDERLY) {
        AddToPendAck(WI_RELIABLE_ORDERLY, msg);

    } else if (header_flag & HTF_RELIABLE) {
        AddToPendAck(WI_RELIABLE, msg);
    }

    // add to timer
    if (!_is_in_timer) {
        // add to timer
        // TODO
        _is_in_timer = true;
    }
}

void CSocketImpl::AddAckToMsg(CMsg* msg) {
    // clear prv ack info
    msg->ClearAck();

    if (_pend_ack[WI_RELIABLE_ORDERLY] && _pend_ack[WI_RELIABLE_ORDERLY]->HasAck()) {
        bool continuity = false;
        std::vector<uint16_t> ack_vec;
        // get acl from pend ack
        // TODO
        msg->SetAck(HPF_RELIABLE_ORDERLY_ACK_RANGE, ack_vec, continuity);
    }

    if (_pend_ack[WI_RELIABLE] && _pend_ack[WI_RELIABLE]->HasAck()) {
        bool continuity = false;
        std::vector<uint16_t> ack_vec;
        // get acl from pend ack
        // TODO
        msg->SetAck(HPF_WITH_RELIABLE_ACK, ack_vec, continuity);
    }
}

void CSocketImpl::GetAckToSendWnd(CMsg* msg) {
    if (msg->GetHeaderFlag() & HPF_WITH_RELIABLE_ORDERLY_ACK) {
        //auto time_stap = CTimer::Instance().GetTimeStamp();
        std::vector<uint16_t> vec;
        msg->GetAck(HPF_WITH_RELIABLE_ORDERLY_ACK, vec);
        for (uint16_t index = vec[0], i = 0; i < vec.size(); index++, i++) {
            _send_wnd[WI_RELIABLE_ORDERLY]->AcceptAck(index);
            // TODO
            // set rtt sample
            //_rto.SetAckTime(index, time_stap);
        }
    }

    if (msg->GetHeaderFlag() & HPF_WITH_RELIABLE_ACK) {
        //auto time_stap = CTimer::Instance().GetTimeStamp();
        std::vector<uint16_t> vec;
        msg->GetAck(HPF_WITH_RELIABLE_ACK, vec);
        for (uint16_t index = vec[0], i = 0; i < vec.size(); index++, i++) {
            _send_wnd[WI_RELIABLE_ORDERLY]->AcceptAck(index);
            // TODO
            // set rtt sample
            //_rto.SetAckTime(index, time_stap);
        }
    }
}

void CSocketImpl::AddToSendWnd(WndIndex index, CMsg* msg) {
    if (!_send_wnd[index]) {
        //_send_wnd[index] = new CSendWnd();
    }
    _send_wnd[index]->PushBack(msg);
}

void CSocketImpl::AddToRecvList(WndIndex index, CMsg* msg) {
    if (!_recv_list[index]) {
        if (index == WI_ORDERLY) {
            _recv_list[index] = new COrderlyList();

        } else if (index == WI_RELIABLE) {
            _recv_list[index] = new CReliableList();

        } else if (index == WI_RELIABLE_ORDERLY) {
            _recv_list[index] = new CReliableOrderlyList();
        }
    }
    int ret = _recv_list[index]->Insert(msg);
    // get a repeat msg
    if (ret == 1) {
        AddAck(msg);
    }
}

void CSocketImpl::AddToPendAck(WndIndex index, CMsg* msg) {
    if (!_pend_ack[index]) {
        _pend_ack[index] = new CPendAck();
    }
    _pend_ack[index]->AddAck(msg->GetId());
}