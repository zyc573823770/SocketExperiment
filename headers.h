#pragma once
#include <WinSock2.h>
#include <tchar.h>
#include <stdio.h>
#include <iostream>
#include <ws2tcpip.h>
#include <time.h>
#pragma comment(lib,"Ws2_32.lib")
#define SIO_RCVALL _WSAIOW(IOC_VENDOR,1)
using namespace std;

//TCP头
typedef struct tcpheaer{
	unsigned short   tcp_sport;		// 16位源端口
	unsigned short   tcp_dport;		// 16位目的端口
	unsigned int     tcp_seq;		// 32位序列号
	unsigned int     tcp_ack;		// 32位确认号
	unsigned char    tcp_lenres;	// 4位首部长度/4位保留字
	unsigned char    tcp_flag;		// 2位保留字/6位标志位
	unsigned short   tcp_win;		// 16位窗口大小
	unsigned short   tcp_sum;		// 16位校验和
	unsigned short   tcp_urp;		// 16位紧急数据偏移量
}TCP_HEADER;

//IP头
typedef struct ipheader{
	unsigned char h_verlen;			// 4位首部长度,4位IP版本号
	unsigned char tos;				// 8位服务类型TOS
	unsigned short ip_length;       // 16位总长度（字节）
	unsigned short ident;			// 16位标识
	unsigned short flag;			// 3位标志位
	unsigned char ttl;				// 8位生存时间 TTL
	unsigned char proto;			// 8位协议 (TCP, UDP 或其他)
	unsigned short checksum;		// 16位IP首部校验和
	struct in_addr srcIP;			// 32位源IP地址
	struct in_addr desIP;			// 32位目的IP地址
}IP_HEADER;

//UDP头
typedef struct udpheader
{
	unsigned short srcport;			//源端口号
	unsigned short dstport;			//目的端口号
	unsigned short headerlen;		//UDP长度
	unsigned short chk_sum;			//校验和
}UDP_HEADER;