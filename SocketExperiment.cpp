#include "headers.h"

char buffer[65535];
struct sockaddr_in from;
IP_HEADER* ip;
TCP_HEADER* tcp;
UDP_HEADER* udp;
int sock, bytes_recieved, fromlen;

bool init();
string getBits(unsigned char a);
string getBits(unsigned short a);
void processTCP(TCP_HEADER* tcp);
void processIP(IP_HEADER* ip);
void processUDP(UDP_HEADER* udp);
void processAPP(int len);
int _tmain(int argc, _TCHAR* argv[]){
	if (!init()) {
		cout << "初始化失败!" << endl;
		return -1;
	}
	cout << "初始化成功" << endl;
	cout << "筛选条件:\n1.IP\t2.TCP\t3.UDP\t4.混合" << endl;
	int choose;
	cin >> choose;
	int i = 0;
	while (true) {
		if (choose < 1 || choose>4) {
			cout << "重新输入" << endl;
			cin >> choose;
		}
		ZeroMemory(buffer, sizeof(buffer));
		fromlen = sizeof(from);
		bytes_recieved = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr*) & from, &fromlen);
		ip = (IP_HEADER*)buffer;
		time_t timep;
		time(&timep);
		char tmp[64];
		strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", localtime(&timep));
		if (choose == 1) {
			cout << "                     NO.#" << (++i) << "\t" << tmp << endl;
			processIP(ip);
			cout << "--------------------------------------结束--------------------------------------" << endl << endl << endl;
		}
		else {
			if (ip->proto == 6) {
				if (choose == 2 || choose == 4) {
					cout << "                     NO.#" << (++i) << "\t" << tmp << endl;
					processIP(ip);
					tcp = (TCP_HEADER*)(buffer + 20);
					processTCP(tcp);
					char* ptr = buffer + (ip->h_verlen & 0x0f) * 4 + ((tcp->tcp_lenres & 0xf0) >> 4) * 4;
					int len = bytes_recieved - (ip->h_verlen & 0x0f) * 4 - ((tcp->tcp_lenres & 0xf0) >> 4) * 4;
					memcpy(buffer, ptr, len);//取出数据
					processAPP(len);
					cout << "--------------------------------------结束--------------------------------------" << endl << endl << endl;
				}
			}
			else {
				if (choose == 3 || choose == 4) {
					cout << "                     NO.#" << (++i) << "\t" << tmp << endl;
					processIP(ip);
					udp = (UDP_HEADER*)(buffer + 20);
					processUDP(udp);
					char* ptr = buffer + (ip->h_verlen & 0x0f) * 4 + 8;
					int len = bytes_recieved - (ip->h_verlen & 0x0f) * 4 - 8;
					memcpy(buffer, ptr, len);//取出数据
					processAPP(len);
					cout << "--------------------------------------结束--------------------------------------" << endl << endl << endl;
				}
			}
		}
	}

	return 0;
}
void processAPP(int len) {
	cout << "--------------------------------------应用层--------------------------------------" << endl;
	if (len == 0) {
		cout << "(空)" << endl;
	}
	else {
		for (int i = 0; i < len; i++) {
			//如果不加这个32-255的约束会有问题，显示某些字符会卡住
			if (buffer[i] >= 32 && buffer[i] <= 255)cout <<(unsigned char) buffer[i];
			else cout << ".";
			if (i % 32 == 31) cout << endl;
		}
	}
	cout << endl;
}

void processIP(IP_HEADER* ip) {
	cout << "--------------------------------------网络层--------------------------------------" << endl;
	//左4位
	cout << "IP版本号:\t" << ((ip->h_verlen & 0xf0)>>4)<< "\t" << getBits(ip->h_verlen).substr(0, 4) << " ...." << endl;
	//右4位
	cout << "IP头部长度:\t" << (ip->h_verlen & 0x0f)*4<< " 字节\t" << ".... " << getBits(ip->h_verlen).substr(4, 4) << endl;
	cout << "IP数据报总长度\t" << ntohs(ip->ip_length) << " 字节\t"<< endl;//这一步得到的原始长度为网络流（最左为0）转换为字节流（最右为0）
	cout << "源IP:\t\t" << inet_ntoa(ip->srcIP) << endl;
	cout << "目的IP\t\t" << inet_ntoa(ip->desIP) << endl;
}

void processTCP(TCP_HEADER* tcp) {
	cout << "--------------------------------------传输层IP------------------------------------" << endl;
	cout << "源端口:\t\t" << ntohs(tcp->tcp_sport) << endl;
	cout << "目的端口:\t" << ntohs(tcp->tcp_dport) << endl;
	cout << "序列号:\t\t" << ntohl(tcp->tcp_seq) << endl;
	cout << "确认号:\t\t" << ntohl(tcp->tcp_ack) << endl;
	cout << "头部长度:\t" << ((tcp->tcp_lenres & 0xf0)>>4)*4<<" 字节" << endl;//右四位为保留字
	//flag字段的左2位为保留字
	cout << "ACK:\t\t" << getBits(tcp->tcp_flag).substr(3, 1) << endl;
	cout << "SYN:\t\t" << getBits(tcp->tcp_flag).substr(6, 1) << endl;
	cout << "FIN:\t\t" << getBits(tcp->tcp_flag).substr(7, 1) << endl;
}

void processUDP(UDP_HEADER* udp) {
	cout << "--------------------------------------传输层UDP-----------------------------------" << endl;
	cout << "源端口:\t\t" << ntohs(udp->srcport) << endl;
	cout << "目的端口:\t" << ntohs(udp->dstport) << endl;
	cout << "UDP长度:\t" << ntohs(udp->headerlen) << endl;
}

string getBits(unsigned char a) {
	char tem[9];
	tem[8] = '\0';
	for (int i = 0; i < 8; i++) {
		if ((a & 0x01) == 1) tem[7 - i] = '1';
		else tem[7 - i] = '0';
		a = a >> 1;
	}
	return string(tem);
}

string getBits(unsigned short a) {
	char tem[16];
	a = ntohs(a);
	for (int i = 0; i < 16; i++) {
		if ((a & 0x01) == 1) tem[15-i] = '1';
		else tem[15-i] = '0';
		a = a >> 1;
	}
	return string(tem);
}

bool init() {
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		cout << "加载winsock失败，错误代码为:" << WSAGetLastError() << endl;
		return false;
	}
	if ((sock = socket(AF_INET, SOCK_RAW, IPPROTO_IP)) == INVALID_SOCKET) {
		cout << "创建套接字失败，错误代码为:" << WSAGetLastError() << endl;
		return false;
	}
	BOOL flag = true;
	setsockopt(sock, IPPROTO_IP, IP_HDRINCL, (char*)&flag, sizeof(BOOL));
	char  name[128];
	if (-1 == gethostname(name, sizeof(name)))
	{
		closesocket(sock);
		cout << WSAGetLastError();
		return 0;
	}
	struct hostent* pHostent;
	pHostent = gethostbyname(name);
	sockaddr_in addr;
	//某些代码使用127.0.0.1和INADDR_ANY，端口0，测试无效
	addr.sin_family = AF_INET;
	addr.sin_port = htons(8888);
	addr.sin_addr = *(in_addr*)pHostent->h_addr_list[0];
	if (bind(sock, (sockaddr*)&addr, sizeof(sockaddr)) == SOCKET_ERROR) {
		cout << "绑定失败，错误代码:" << WSAGetLastError() << endl;
		closesocket(sock);
		return false;
	}
	//设置该socket可以接受所有接受和发出的包，常规写法显示的包很少
	u_long sioarg = 1;
	DWORD wt = 0;
	if (WSAIoctl(sock, SIO_RCVALL, &sioarg, sizeof(sioarg), NULL, 0, &wt, NULL, NULL)==SOCKET_ERROR) {
		cout << "无法设置socket，错误代码:" << WSAGetLastError() << endl;
		closesocket(sock);
		return false;
	}
	//设置阻塞模式
	u_long bioarg = 0;
	if (ioctlsocket(sock, FIONBIO, &bioarg) == SOCKET_ERROR) {
		cout << "无法设置为阻塞IO，错误代码为:" << WSAGetLastError() << endl;
		closesocket(sock);
		return false;
	}
	return true;
}

