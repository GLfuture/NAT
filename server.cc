#include "udp.h"
#include "decoder.h"
#include <memory>
#define SERVER_PORT 10000


int main(int argc,char*argv[])
{
    UDP_Server_Base::Ptr server = std::make_shared<UDP_Server>();
    int fd1 = server->Sock();
    UDP_Base::Addr_Ptr sin = std::make_shared<sockaddr_in>();
    sin->sin_family = AF_INET;
    sin->sin_port = htons(SERVER_PORT);
    sin->sin_addr.s_addr= htonl(INADDR_ANY);

    server->Bind(sin);
    Server_Decoder decoder;
    while(1)
    {
        UDP_Base::Addr_Ptr caddr = std::make_shared<sockaddr_in>();
        std::string buffer;
        int len = server->Recv_From(caddr,buffer,MTU_LENGTH);
        if(len <= 0) continue;
        decoder.decode(server,buffer,caddr);
    }
    return 0;
}