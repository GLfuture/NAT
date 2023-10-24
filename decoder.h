#pragma once
#ifndef CLASSIFY_H
#define CLASSIFY_H
#include <iostream>
#include <memory>
#include "udp.h"
#include "protocol.h"
#include <string.h>

class Decoder_Base
{
public:
    //using UDP_Base_Ptr = std::shared_ptr<UDP_Base>;
    // using UDP_Server_Ptr = std::shared_ptr<UDP_Server_Base>;
    // enum NAT_TYPE
    // {
    //     BEG = 0,
    //     NAT_SYMMETRIC = BEG,        //对称NAT
    //     NAT_FULL_TAPERED,           //完全锥形NAT
    //     NAT_IP_RESTRICTION_TAPER,   //IP限制锥形NAT
    //     NAT_PORT_RESTRICTION_TAPER, //PORT限制锥形NAT
    //     END
    // };
    virtual void decode(UDP_Base::Ptr executor,const std::string &buffer,const UDP_Base::Addr_Ptr& caddr) = 0;
};

class Server_Decoder:public Decoder_Base
{
public: 
    using Ptr = std::shared_ptr<Server_Decoder>;

    void decode(UDP_Base::Ptr executor,const std::string &buffer,const UDP_Base::Addr_Ptr& caddr) override
    {
        UDP_Server::Ptr server = std::dynamic_pointer_cast<UDP_Server>(executor);
        Protocol_Head head;
        memcpy(&head,buffer.c_str(),sizeof(Protocol_Head));
        server->Add_Client_Addr(head.selfid,caddr);
        std::string body = buffer.substr(sizeof(Protocol_Head));
        switch (head.status)
        {
        case STATUS_LOGIN_REQ:
        {
            server->Add_Client_Addr(head.selfid,caddr);
            deal_login_req(executor, caddr);
            //std::cout<<inet_ntoa(server->Get_Client_Addr(head.selfid)->sin_addr)<<std::endl;
            break;
        }
        case STATUS_CONN_REQ:
        {
            UDP_Base::Addr_Ptr other_addr = server->Get_Client_Addr(head.otherid);
            std::cout<<head.otherid<<std::endl;
            std::cout<<"other addr :"<<inet_ntoa(other_addr->sin_addr)<<std::endl; 
            deal_conn_req(executor,caddr,other_addr);
            break;
        }
        case STATUS_NOTIFY_ACK:
        {
            break;
        }
        case STATUS_HEARTBEAT_REQ:
            break;
        case STATUS_NOTIFY_REQ:
            break;
        case STATUS_P2P_MESSAGE_REQ:
        {
            deal_p2p_msg_req(executor,caddr,body);
            break;
        }
        default:
            break;
        }
    }

private:
    void deal_login_req(UDP_Base::Ptr executor,const UDP_Base::Addr_Ptr& caddr)
    {
        //std::cout<< inet_ntoa(caddr->sin_addr)<<std::endl;
        UDP_Server::Ptr server = std::dynamic_pointer_cast<UDP_Server>(executor);
        std::string buffer;
        Protocol_Head head;
        head.version = 1;
        head.status = STATUS_LOGIN_ACK;
        int hlen = sizeof(Protocol_Head);
        char* temp = new char[hlen];
        memcpy(temp,&head,hlen);
        buffer.assign(temp,hlen);
        delete temp;
        int len = server->Send_To(caddr,buffer);
        if(len == -1) perror("sendto");
    }

    void deal_conn_req(UDP_Base::Ptr executor,const UDP_Base::Addr_Ptr& caddr, const UDP_Base::Addr_Ptr& other_addr)
    {
        
        UDP_Server::Ptr server = std::dynamic_pointer_cast<UDP_Server>(executor);
        std::string msg;
        Protocol_Head head;
        head.version = 1;
        int hlen= sizeof(Protocol_Head);
        char* buffer = new char[hlen];
        if (other_addr == nullptr)
        {
            head.status = STATUS_P2P_CONNECT_FAIL;
            memcpy(buffer,&head,hlen);
            msg.assign(buffer,hlen);
            server->Send_To(caddr,msg);
            return;
        }
        head.status = STATUS_CONN_ACK;
        head.ip = other_addr->sin_addr.s_addr;
        head.port = other_addr->sin_port;
        memset(buffer,0,hlen);
        memcpy(buffer,&head,hlen);
        msg.assign(buffer,hlen);
        server->Send_To(caddr,msg);
        head.ip = caddr->sin_addr.s_addr;
        head.port = caddr->sin_port;
        head.status = STATUS_NOTIFY_REQ;
        memset(buffer,0,hlen);
        memcpy(buffer,&head,hlen);
        msg.assign(buffer,hlen);
        server->Send_To(other_addr,msg);
        delete buffer;
    }

    void deal_p2p_msg_req(UDP_Base::Ptr executor,const UDP_Base::Addr_Ptr& caddr,std::string& body)
    {
        UDP_Server::Ptr server = std::dynamic_pointer_cast<UDP_Server>(executor);
        std::cout<<inet_ntoa(caddr->sin_addr)<<" : "<<ntohs(caddr->sin_port)<<"  msg :"<<body<<std::endl;
        Protocol_Head head;
        head.version = 1;
        head.status = STATUS_P2P_MESSAGE_ACK;
        int hlen = sizeof(Protocol_Head);
        char* buffer = new char[hlen];
        memcpy(buffer,&head,hlen);
        std::string msg(buffer,hlen);
        server->Send_To(caddr,msg);
    }

};

class Client_Decoder:public Decoder_Base
{
public:
    Client_Decoder(const UDP_Base::Addr_Ptr& saddr)
    {
        this->_caddr = std::make_shared<sockaddr_in>();
        this->_saddr = saddr;
        P2P_Modle = false;
    }

    using UDP_Client_Ptr = std::shared_ptr<UDP_Client>;

    void decode(UDP_Base::Ptr executor,const std::string& buffer,const UDP_Base::Addr_Ptr& caddr) override
    {
        UDP_Client_Ptr client = std::dynamic_pointer_cast<UDP_Client>(executor);
        Protocol_Head head;
        memcpy(&head,buffer.c_str(),sizeof(Protocol_Head));
        std::string body = buffer.substr(sizeof(Protocol_Head));
        int hlen = sizeof(Protocol_Head);
        switch (head.status)
        {
        case STATUS_NOTIFY_REQ:
        {
            Protocol_Head head;
            head.version = 1;
            head.status = STATUS_NOTIFY_ACK;
            char* temp = new char[hlen];
            std::string str;
            memcpy(temp,&head,hlen);
            str.assign(temp,hlen);
            client->Send_To(_saddr,str);
            head.status = STATUS_P2P_CONNECT_REQ;
            _caddr->sin_addr.s_addr = head.ip;
            _caddr->sin_port = head.port;
            _caddr->sin_family = AF_INET;
            memset(temp,0,hlen);
            memcpy(temp,&head,hlen);
            str.assign(temp,hlen);
            client->Send_To(_caddr,str);
            delete temp;
            break;
        }
        case STATUS_P2P_CONNECT_REQ:
        {
            if(caddr->sin_addr.s_addr == _caddr->sin_addr.s_addr&&_caddr->sin_port == caddr->sin_port)
            {
                Protocol_Head head;
                head.version = 1;
                head.status = STATUS_P2P_CONNECT_ACK;
                char* temp = new char[hlen];
                memcpy(temp,&head,hlen);
                std::string str(temp,hlen);
                client->Send_To(_caddr,str);
                delete temp;
            }
            else{
                Protocol_Head head;
                head.version = 1;
                head.status = STATUS_P2P_CONNECT_FAIL;
                char* temp = new char[hlen];
                memcpy(temp,&head,hlen);
                std::string str(temp,hlen);
                client->Send_To(_caddr,str);
                delete temp;
            }
            break;
        }
        case STATUS_P2P_CONNECT_ACK:
        {
            std::cout<<"connect success"<<std::endl;
            break;
        }
        case STATUS_P2P_CONNECT_FAIL:
        {
            std::cout<<"connect failed"<<std::endl;
            break;
        }
        case STATUS_HEARTBEAT_ACK:
            break;
        case STATUS_CONN_ACK: //请求连接的ack
        {
            _caddr->sin_addr.s_addr = head.ip;
            _caddr->sin_port = head.port;
            _caddr->sin_family = AF_INET;
            break;
        }
        case STATUS_P2P_MESSAGE_REQ:
        {
            Protocol_Head head;
            head.version = 1;
            head.status = STATUS_P2P_MESSAGE_ACK;
            char* buffer = new char[hlen];
            memcpy(buffer,&head,hlen);
            std::cout<<inet_ntoa(caddr->sin_addr)<<" : "<<ntohs(caddr->sin_port)<<"  msg :"<<body<<std::endl;
            std::string str(buffer,hlen);
            client->Send_To(caddr,str);
            break;
        }
        case STATUS_P2P_MESSAGE_ACK:
            break;
        default:
            break;
        }
    }

    UDP_Base::Addr_Ptr _saddr;
    UDP_Base::Addr_Ptr _caddr;
    bool P2P_Modle;
};


#endif
