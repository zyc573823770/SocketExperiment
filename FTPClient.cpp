#include<iostream>
#include<winsock.h>
#include<cmath>
#include<thread>
#include<string>
#pragma warning(disable:4996)
#pragma comment(lib,"ws2_32.lib")

using namespace std;

SOCKADDR_IN serv_addr;
SOCKET client_sock;
SOCKADDR_IN client_addr;
const char* FILEPATH = "D:\\Code\\CWorkspace\\FTPClient\\file\\";
#define MAXSIZE 65507

void initialize() {

	//初始化套接字库
	WORD w_req = MAKEWORD(2, 2);//版本号
	WSADATA wsadata;
	int err;
	err = WSAStartup(w_req, &wsadata);
	if (err != 0) {
		cout << "初始化套接字库失败！" << endl;
	}
	else {
		cout << "初始化套接字库成功！" << endl;
	}
}

//htons()--"Host to Network Short"
//
//htonl()--"Host to Network Long"
//
//ntohs()--"Network to Host Short"
//
//ntohl()--"Network to Host Long"

//给定客户端地址和服务端地址
//由线程新建套接字连接服务器并接收数据
void process() {
	client_sock = socket(AF_INET, SOCK_STREAM, 0);
	cout << "客户端socke绑定" << inet_ntoa(client_addr.sin_addr) << ":" << ntohs(client_addr.sin_port) << endl;
	if (bind(client_sock, (sockaddr*)(&client_addr), sizeof(client_addr)) == SOCKET_ERROR) {
		cout << "绑定失败，错误代码:" << WSAGetLastError() << endl;
		closesocket(client_sock);
		return;
	}
	if (connect(client_sock, (sockaddr*)(&serv_addr), sizeof(serv_addr)) == SOCKET_ERROR) {
		cout << "连接服务器失败，错误代码:" << WSAGetLastError() << endl;
		closesocket(client_sock);
		return;
	}
	string name;
	//测试多线程收数据
	while (1) {
		char buffer[MAXSIZE];
		cout << "ftp:";
		name = "";
		ZeroMemory(buffer, sizeof(buffer));
		cin.getline(buffer, MAXSIZE - 1);
		name = string(buffer);
		send(client_sock, name.c_str(), name.size() * sizeof(char), 0);
		if (name == "quit") {
			break;
		}
		ZeroMemory(buffer, sizeof(buffer));
		if (recv(client_sock, buffer, sizeof(buffer), 0) == SOCKET_ERROR) {
			cout << "接受回显错误，错误代码:" << WSAGetLastError() << endl;
			continue;
		}
		cout << buffer << endl;
		if (strcmp(buffer, "login success") == 0) {
			ZeroMemory(buffer, sizeof(buffer));
			if (recv(client_sock, buffer, sizeof(buffer), 0) == SOCKET_ERROR) {
				cout << "接受回显错误，错误代码:" << WSAGetLastError() << endl;
				continue;
			}
			//初次登陆成功显示列表
			cout << buffer<<endl;
		}
		if (strcmp(buffer, "Prepare get file...") == 0) {
			ZeroMemory(buffer, sizeof(buffer));
			if (recv(client_sock, buffer, sizeof(buffer), 0) == SOCKET_ERROR) {
				cout << "Can't get filename" << WSAGetLastError() << endl;
				continue;
			}
			string filename(buffer);
			FILE* fp = fopen((string(FILEPATH) + string(buffer)).c_str(), "wb");
			//cout << buffer << endl;
			if (fp == NULL) {
				cout << "Can't init filename" << WSAGetLastError() << endl;
				continue;
			}
			int file_length = 0;
			bool transmitOver = true;
			ZeroMemory(buffer, sizeof(buffer));
			while ((file_length = recv(client_sock, buffer, sizeof(buffer), 0)) > 0) {
				if (strcmp(buffer, "#####")==0) break;
				if (fwrite(buffer, sizeof(char), file_length, fp) < file_length) {
					cout << "Write error" << endl;
					transmitOver = false;
					break;
				}
				ZeroMemory(buffer, sizeof(buffer));
			}
			fclose(fp);
			if (transmitOver) {
				cout << (string(FILEPATH) + filename).c_str() << " transmit over" << endl;
			}
		}
	}
	closesocket(client_sock);
}

int main() {
	initialize();
	//访问的服务器的IP和端口
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(6666);
	serv_addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	int MIN = 2;
	int MAX = 254;
	//随机生成客户端端口和IP
	srand((unsigned)time(0));
	int port = rand() % (MAX - MIN + 1) + MIN;
	client_addr.sin_family = AF_INET;
	client_addr.sin_port = htons(6668 + port);
	char stem[1024] = "127.0.0.";
	client_addr.sin_addr.S_un.S_addr = inet_addr(strcat(stem, to_string(port).c_str()));
	process();
	WSACleanup();
	return 0;
}
