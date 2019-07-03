#include "OsNet.h"
#include "../Test.h"

using namespace hudp;

void UtestOsNet() {
    Expect_True(COsNet::Init());

    auto ipv4 = COsNet::GetOsIp();
    std::cout << "ipv4 : " << ipv4.c_str() << std::endl;
    auto ipv6 = COsNet::GetOsIp(false);
    std::cout << "ipv4 : " << ipv6.c_str() << std::endl;

    uint64_t socket1 = COsNet::UdpSocket();
    Expect_Bigger(socket1, 0);
    
    Expect_True(COsNet::Bind(socket1, ipv4, 4396));

    uint64_t socket2 = COsNet::UdpSocket();
    Expect_Bigger(socket2, 0);

    Expect_True(COsNet::Bind(socket2, ipv4, 4397));
    int ret = COsNet::SendTo(socket2, "test msg", strlen("test msg"), ipv4, 4396);
    Expect_Bigger(ret, 0);

    char buf[128] = {};
    std::string from_ip;
    uint16_t port = 0;
    ret = COsNet::RecvFrom(socket1, buf, 128, from_ip, port);
    Expect_Bigger(ret, 0);

    Expect_True(COsNet::Close(socket1));
    Expect_True(COsNet::Close(socket2));
}

#include <memory>
template <class T, void(*deleter)(T*)> class CSmartPtr : public std::unique_ptr<T, void(*)(T*)> {
public:
    CSmartPtr() : std::unique_ptr<T, void(*)(T*)>(nullptr, deleter) {}
    CSmartPtr(T* object) : std::unique_ptr<T, void(*)(T*)>(object, deleter) {}
};

void Destroy(int* arr) {
    std::cout << "delete" << std::endl;
    delete[] arr;
}

int main() {

    {
        int* arr = new int[1000];
        arr[0] = 10000;
        CSmartPtr<int, Destroy> p(arr);
    }

    int a= 0;
    a++;
}