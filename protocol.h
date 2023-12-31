#pragma pack(push, 1)
#pragma once
//取消字节对齐
#ifndef PROTOCOL_H
#define PROTOCOL_H



#define STATUS_LOGIN_REQ            0x01
#define STATUS_LOGIN_ACK            0x02

#define STATUS_CONN_REQ             0x03
#define STATUS_CONN_ACK             0x04

#define STATUS_HEARTBEAT_REQ        0x05
#define STATUS_HEARTBEAT_ACK        0x06

#define STATUS_NOTIFY_REQ           0x07
#define STATUS_NOTIFY_ACK           0x08

#define STATUS_P2P_CONNECT_REQ      0x09
#define STATUS_P2P_CONNECT_ACK      0x10
#define STATUS_P2P_CONNECT_FAIL     0xA


#define STATUS_P2P_MESSAGE_REQ      0xB
#define STATUS_P2P_MESSAGE_ACK      0xC

#define MACHINE_STATUS_ENTER_BT     0xD
#define MACHINE_STATUS_ENTER_MSG    0xE
#define MACHINE_STATUS_P2P_MODEL    0xF

extern int machine_status;
int machine_status = 0;

class Protocol_Head
{
public:
    uint8_t     version;
    uint8_t     status;
    uint8_t     length;
    uint16_t    port;
    uint32_t    selfid;
    uint32_t    otherid;
    uint32_t    ip;
};

#endif