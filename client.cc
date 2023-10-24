#include "udp.h"
#include "protocol.h"
#include "decoder.h"
#include <iostream>
#include <thread>


#define SERVER_IP                   "192.168.124.142"
#define SERVER_PORT                 10000

static uint32_t selfid;
static Client_Decoder decoder;

void Connect_Req(UDP_Base::Ptr base,uint32_t otherid,UDP_Base::Addr_Ptr saddr)
{
    UDP_Client::Ptr client = std::dynamic_pointer_cast<UDP_Client>(base);
    Protocol_Head head;
    head.version = 1;
    head.otherid = otherid;
    head.status = STATUS_CONN_REQ;
    head.selfid = selfid;
    int hlen = sizeof(Protocol_Head);
    char* buffer=new char[hlen];
    memcpy(buffer,&head,hlen);
    std::string temp(buffer,hlen);
    client->Send_To(saddr,temp);
    delete buffer;

}

void Work(UDP_Client::Ptr client,UDP_Base::Addr_Ptr server_addr)
{
    while(1)
    {
        UDP_Base::Addr_Ptr addr = std::make_shared<sockaddr_in>();
        std::string res;
        int len = client->Recv_From(addr, res, MTU_LENGTH);
        decoder.decode(client, res, addr);
    }
}


int main(int argc,char* argv[])
{
    if(argc!= 2){
        std::cout<<argv[0]<<" selfid"<<std::endl;
        return 0;
    }
    selfid = atoi(argv[1]);
    UDP_Client::Ptr client = std::make_shared<UDP_Client>();
    int fd = client->Sock();
    UDP_Base::Addr_Ptr server_addr = std::make_shared<sockaddr_in>();
    server_addr->sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr->sin_port = htons(SERVER_PORT);
    server_addr->sin_family = AF_INET;
    decoder._saddr = server_addr;
    UDP_Base::Addr_Ptr addr;
    //client->Connect(server_addr);
    {
        Protocol_Head head;
        head.version = 1;
        head.status = STATUS_LOGIN_REQ;
        head.selfid = selfid;
        head.length = 0;
        int hlen = sizeof(Protocol_Head);
        char *temp = new char[hlen];
        memcpy(temp, &head, hlen);
        std::string buffer(temp, hlen);
        delete temp;
        client->Send_To(server_addr, buffer);
        memset(&head, 0, hlen);
        buffer.clear();
        client->Recv_From(addr, buffer, hlen);
        memcpy(&head, buffer.c_str(), hlen);
        if (head.status == STATUS_LOGIN_ACK)
        {
            std::cout << "connection success" << std::endl;
        }
    }
    std::thread th(Work,client,server_addr);
    std::cout<<"Press C to change model"<<std::endl;
    std::cout<<"Press Q to exit program"<<std::endl;
    std::cout<<"Press X to enter p2p model"<<std::endl;
    machine_status = MACHINE_STATUS_ENTER_MSG;
    int hlen = sizeof(Protocol_Head);
    char* temp = new char[hlen];
    while(1)
    {
        std::string buffer;
        if(machine_status == MACHINE_STATUS_ENTER_MSG){
            std::cout<<"send msg : ";
        }
        else if(machine_status == MACHINE_STATUS_ENTER_BT)
        {
            std::cout<<"search bt : ";
        }
        std::getline(std::cin,buffer);
        if(buffer.compare("Q") == 0) break;
        if(buffer.compare("C") == 0)
        {
            machine_status = (machine_status==MACHINE_STATUS_ENTER_MSG)?MACHINE_STATUS_ENTER_BT:MACHINE_STATUS_ENTER_MSG;
            continue;
        }
        else if(buffer.compare("X") == 0)
        {
            std::cout<<"Enter p2p model"<<std::endl;
            std::cout<<"Please enter other id :";
            uint32_t otherid;
            std::cin>>otherid;
            getchar();
            Connect_Req(client,otherid,server_addr);

            continue;
        }
        if(machine_status == MACHINE_STATUS_ENTER_MSG){
            Protocol_Head head;
            head.version = 1;
            head.length = buffer.length();
            head.selfid = selfid;
            head.status = STATUS_P2P_MESSAGE_REQ;
            memset(temp,0,hlen);
            memcpy(temp,&head,hlen);
            std::string msg(temp,hlen);
            msg+=buffer;
            client->Send_To(server_addr,msg);
        }
        else if(machine_status == MACHINE_STATUS_P2P_MODEL)
        {
            Protocol_Head head;
            head.version = 1;
            head.length = buffer.length();
            head.selfid = selfid;
            head.status = STATUS_P2P_MESSAGE_REQ;
            memset(temp,0,hlen);
            memcpy(temp,&head,hlen);
            std::string msg(temp,hlen);
            msg+=buffer;
            client->Send_To(decoder._caddr,msg);
        }

    }
    th.join();
    delete temp;
    return 0;
}
