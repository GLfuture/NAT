#pragma once
#ifndef UDP_H
#define UDP_H

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <cstring>
#include <memory>
#include <map>
#include <iostream>
#define MTU_LENGTH 1500

class UDP_Base
{
public:
    using Ptr = std::shared_ptr<UDP_Base>;

    using Addr_Ptr =  std::shared_ptr<sockaddr_in>;
    virtual int Sock() = 0;
    virtual ssize_t Recv_From(Addr_Ptr& from_addr,std::string& buffer,int len) = 0;
    virtual ssize_t Send_To(const Addr_Ptr& to_addr,const std::string& buffer) = 0; 
    virtual ~UDP_Base() {}
};

class UDP_Client_Base:public UDP_Base
{
public:
    using Ptr=std::shared_ptr<UDP_Client_Base>;
    virtual int Connect(const Addr_Ptr sin) = 0;
    virtual ~UDP_Client_Base() {}
};


class UDP_Server_Base:public UDP_Base
{
public:

    using Ptr = std::shared_ptr<UDP_Server_Base>;

    virtual int Bind(const Addr_Ptr addr) = 0;
    virtual const Addr_Ptr Get_Client_Addr(uint32_t selfid) = 0;
    virtual void Add_Client_Addr(uint32_t selfid,const Addr_Ptr addr) = 0;
    virtual ~UDP_Server_Base() {}
};

class UDP_Client:public UDP_Client_Base
{
public:
    int Sock() override
    {
        fd = socket(AF_INET,SOCK_DGRAM,0);
        return fd;
    }


    int Connect(const Addr_Ptr sin) override
    {
        return connect(fd,(sockaddr*)sin.get(),sizeof(sin));
    }


    ssize_t Recv_From(Addr_Ptr& from_addr,std::string& buffer,int len) override
    {
        sockaddr_in addr;
        char* temp = new char[len];
        memset(temp,0,len);
        socklen_t slen = sizeof(sockaddr_in);
        ssize_t ret = recvfrom(fd,temp,len,0,(sockaddr*)&addr,&slen);
        if(ret == -1) return ret;
        from_addr.reset(new sockaddr_in(addr));
        buffer.assign(temp,ret);
        delete temp;
        return ret;
    }

    ssize_t Send_To(const Addr_Ptr& to_addr,const std::string& buffer) override
    {
        return sendto(fd,buffer.c_str(),buffer.length(),0,(sockaddr*)to_addr.get(),sizeof(sockaddr));
    }

    uint32_t Get_Selfid() { return selfid;}

protected:
    int fd;
    uint32_t selfid;
};


class UDP_Server:public UDP_Server_Base
{
    
public:

    int Sock() override
    {
        fd = socket(AF_INET,SOCK_DGRAM,0);
        return fd;
    }

    int Bind(const Addr_Ptr addr) override
    {
        return bind(this->fd,(sockaddr*)addr.get(),sizeof(sockaddr_in));
    }

    ssize_t Recv_From(Addr_Ptr& from_addr,std::string& buffer,int len) override
    {
        sockaddr_in addr;
        char* temp = new char[len];
        memset(temp,0,len);
        socklen_t slen = sizeof(sockaddr_in);
        ssize_t ret = recvfrom(fd,temp,len,0,(sockaddr*)&addr,&slen);
        if(ret == -1) return ret;
        from_addr.reset(new sockaddr_in(addr));
        buffer.assign(temp,ret);
        delete temp;
        return ret;
    }

    ssize_t Send_To(const Addr_Ptr& to_addr,const std::string& buffer) override
    {
        return sendto(fd,buffer.c_str(),buffer.length(),0,(sockaddr*)to_addr.get(),sizeof(sockaddr));
    }
    
    const Addr_Ptr Get_Client_Addr(uint32_t selfid) override
    {
        std::map<uint32_t,Addr_Ptr>::iterator it = clients.find(selfid);
        if(it == clients.end()) return nullptr;
        return it->second;
    }

    void Add_Client_Addr(uint32_t selfid,const Addr_Ptr addr) override
    {
        clients[selfid] = addr;
    }

protected:
    std::map<uint32_t,Addr_Ptr> clients;
    int fd;
};



#endif