#include <cstring>		//for memset

#include "IMsg.h"
#include "ISocket.h"
#include "HudpImpl.h"
#include "HudpConfig.h"
#include "OrderListImpl.h"

using namespace hudp;

CRecvList::CRecvList() {
    
}

CRecvList::~CRecvList() {

}

uint16_t CRecvList::HashFunc(uint16_t id) {
    return id & (__msx_cache_msg_num - 1);
}

CReliableOrderlyList::CReliableOrderlyList(uint16_t start_id) : _expect_id(start_id) {
    memset(_order_list, 0, sizeof(_order_list));
}

CReliableOrderlyList::~CReliableOrderlyList() {
    std::unique_lock<std::mutex> lock(_mutex);
    for (size_t i = 0; i < __msx_cache_msg_num; i++) {
        if (_order_list[i]) {
            // return to msg pool
            CHudpImpl::Instance().ReleaseMessage(_order_list[i]);
        }
    }
}

uint16_t CReliableOrderlyList::Insert(CMsg* msg) {
	auto id = msg->GetId();
    uint16_t index = HashFunc(id);
    
    {
        std::unique_lock<std::mutex> lock(_mutex);

        if (id == _expect_id) {
			_order_list[index] = msg;
            while (_order_list[index]) {
                _expect_id++;
                _recv_list.Push(_order_list[index]);
                _order_list[index] = nullptr;

                index++;
                if (index >= __msx_cache_msg_num) {
                    index = 0;
                }
            }

        // is't expect id
        } else {
            // a repeat bag
            if (_order_list[index]) {
                return 1;
            } else {
				_order_list[index] = msg;
            }
            
        }
    }
    
    if (_recv_list.Size() > 0) {
		CMsg* item = nullptr;

        while (_recv_list.Pop(item)) {
			auto sock = item->GetSocket();
			sock->ToRecv(item);
        }
        _recv_list.Clear();
    }
    return 0;
}

CReliableList::CReliableList(uint16_t start_id) : _expect_id(start_id) {
    memset(_order_list, 0, sizeof(_order_list));
}

CReliableList::~CReliableList() {
    
}

// reliable list, only judgement repetition in msg cache
uint16_t CReliableList::Insert(CMsg* msg) {
	auto id = msg->GetId();
    uint16_t index = HashFunc(id);
    // too farm, discard this msg
    if (std::abs(id - _expect_id) || (_expect_id < (__max_id - __max_compare_num)) && id < __max_compare_num) {
        return 0;
    }

    {
        std::unique_lock<std::mutex> lock(_mutex);

        if (_order_list[index] == id) {
            return 1;

        } else {
            _order_list[index] = id;
        }

    }
    _expect_id = id;
    auto sock = msg->GetSocket();
	sock->ToRecv(msg);
    return 0;
}

COrderlyList::COrderlyList(uint16_t start_id) : _expect_id(start_id) {

}

COrderlyList::~COrderlyList() {

}

// orderly list, if msg id is bigger than expect id, recv it.
uint16_t COrderlyList::Insert(CMsg* msg) {
	auto id = msg->GetId();
    if (id < _expect_id) {
        return 0;
    }

    _expect_id = id;
	auto sock = msg->GetSocket();
    sock->ToRecv(msg);
    return 0;
}