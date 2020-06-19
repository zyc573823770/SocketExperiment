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

//TCPͷ
typedef struct tcpheaer{
	unsigned short   tcp_sport;		// 16λԴ�˿�
	unsigned short   tcp_dport;		// 16λĿ�Ķ˿�
	unsigned int     tcp_seq;		// 32λ���к�
	unsigned int     tcp_ack;		// 32λȷ�Ϻ�
	unsigned char    tcp_lenres;	// 4λ�ײ�����/4λ������
	unsigned char    tcp_flag;		// 2λ������/6λ��־λ
	unsigned short   tcp_win;		// 16λ���ڴ�С
	unsigned short   tcp_sum;		// 16λУ���
	unsigned short   tcp_urp;		// 16λ��������ƫ����
}TCP_HEADER;

//IPͷ
typedef struct ipheader{
	unsigned char h_verlen;			// 4λ�ײ�����,4λIP�汾��
	unsigned char tos;				// 8λ��������TOS
	unsigned short ip_length;       // 16λ�ܳ��ȣ��ֽڣ�
	unsigned short ident;			// 16λ��ʶ
	unsigned short flag;			// 3λ��־λ
	unsigned char ttl;				// 8λ����ʱ�� TTL
	unsigned char proto;			// 8λЭ�� (TCP, UDP ������)
	unsigned short checksum;		// 16λIP�ײ�У���
	struct in_addr srcIP;			// 32λԴIP��ַ
	struct in_addr desIP;			// 32λĿ��IP��ַ
}IP_HEADER;

//UDPͷ
typedef struct udpheader
{
	unsigned short srcport;			//Դ�˿ں�
	unsigned short dstport;			//Ŀ�Ķ˿ں�
	unsigned short headerlen;		//UDP����
	unsigned short chk_sum;			//У���
}UDP_HEADER;