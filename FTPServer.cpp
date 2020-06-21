#include<iostream>
#include<winsock.h>
#include<time.h>
#include<process.h>
#include<io.h>
#include<vector>
#include<map>
#pragma comment(lib,"ws2_32.lib")

using namespace std;

#define INETADDR "127.0.0.1"
#define CTRLPORT 6666
#define FILEPORT 6667
#define MAXSIZE 65507
const char* FILEPATH = "D:\\Code\\CWorkspace\\FTPServer\\download";
SOCKET serv_socket;
SOCKADDR_IN serv_addr;
map<SOCKET, SOCKET> ctrl2trans;

void getFiles(const string path, vector<string>& files)
{
	//文件句柄
	long   hFile = 0;
	//文件信息
	struct _finddata_t fileinfo;
	string p;
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
	{
		do
		{
			//如果是目录,迭代之
			//如果不是,加入列表
			if ((fileinfo.attrib & _A_SUBDIR))
			{
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
					getFiles(p.assign(path).append("\\").append(fileinfo.name), files);
			}
			else
			{
				files.push_back(fileinfo.name);
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
}

string getFileList() {
	string res;
	vector<string> fileList;
	getFiles(FILEPATH, fileList);
	if (fileList.size() == 0) return "服务器暂无文件";
	for (int i = 0; i < fileList.size(); i++) {
		res.append(fileList[i]);
		if (i != fileList.size() - 1) res.append("\n");
	}
	return res;
}

bool initialize() {
	//初始化套接字库
	WORD w_req = MAKEWORD(2, 2);//版本号
	WSADATA wsadata;
	if (WSAStartup(w_req, &wsadata) != 0) {
		cout << "Winsock初始化失败，错误代码:"<<WSAGetLastError() << endl;
		return false;
	}
	if ((serv_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
		cout << "socket创建失败，错误代码:" << WSAGetLastError() << endl;
		return false;
	}
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(CTRLPORT);
	serv_addr.sin_addr.S_un.S_addr = inet_addr(INETADDR);
	if (bind(serv_socket, (SOCKADDR*)&serv_addr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
		cout << "绑定套接字错误，错误代码:" << WSAGetLastError() << endl;
		closesocket(serv_socket);
		return false;
	}
	if (listen(serv_socket, SOMAXCONN) == SOCKET_ERROR) {
		cout << "侦听失败，错误代码:" << endl;
		closesocket(serv_socket);
		return false;
	}
	return true;
}

typedef struct ccc {
	const char* order;
	const char* path;
}CTRLCMD;

typedef struct ddd {
	SOCKET client;
	SOCKADDR_IN addr;
	fd_set rfds;
}LOGINCMD;

unsigned int __stdcall LoginThread(LPVOID c) {
	int recvSize;
	char *buffer = new char[MAXSIZE];
	while (true) {
		ZeroMemory(buffer, MAXSIZE * sizeof(char));
		//等待接受用户名密码
		recvSize = recv(((LOGINCMD*)c)->client, buffer, sizeof(char)*MAXSIZE, 0);
		//如果断开连接
		if (recvSize==SOCKET_ERROR || strcmp(buffer, "quit")==0) {
			cout<< inet_ntoa(((LOGINCMD*)c)->addr.sin_addr) << ":" << ntohs(((LOGINCMD*)c)->addr.sin_port) << "终止连接" << endl;
			closesocket(((LOGINCMD*)c)->client);
			delete c;
			_endthreadex(0);
			return 0;
		}
		else {
			if (strcmp(buffer, "admin 123456")!=0) {
				cout << inet_ntoa(((LOGINCMD*)c)->addr.sin_addr) << ":" << ntohs(((LOGINCMD*)c)->addr.sin_port) << "密码错误" << endl;
				send(((LOGINCMD*)c)->client, "login error", sizeof("login error"), 0);
			}
			else {
				cout << inet_ntoa(((LOGINCMD*)c)->addr.sin_addr) << ":" << ntohs(((LOGINCMD*)c)->addr.sin_port) << "登录成功" << endl;
				send(((LOGINCMD*)c)->client, "login success", sizeof("login success"), 0);

				string res;
				vector<string> fileList;
				getFiles(FILEPATH, fileList);
				if (fileList.size() == 0) res = "服务器暂无文件";
				for (int i = 0; i < fileList.size(); i++) {
					res.append(fileList[i]);
					if (i != fileList.size() - 1) res.append("\n");
				}

				send(((LOGINCMD*)c)->client, res.c_str(), sizeof(char)*res.size(), 0);
				while (true) {
					ZeroMemory(buffer, MAXSIZE * sizeof(char));
					if ((recvSize = recv(((LOGINCMD*)c)->client, buffer, sizeof(char) * MAXSIZE, 0))<=0) {
						cout << inet_ntoa(((LOGINCMD*)c)->addr.sin_addr) << ":" << ntohs(((LOGINCMD*)c)->addr.sin_port) << "断开连接" << endl;
						break;
					}
					cout << inet_ntoa(((LOGINCMD*)c)->addr.sin_addr) << ":" << ntohs(((LOGINCMD*)c)->addr.sin_port) << "执行命令" << buffer << endl;
					if (strcmp(buffer, "quit")==0) {
						cout << inet_ntoa(((LOGINCMD*)c)->addr.sin_addr) << ":" << ntohs(((LOGINCMD*)c)->addr.sin_port) << "断开连接" << endl;
						break;
					}
					if (strcmp(buffer, "list")==0) {
						res = "";
						fileList.clear();
						getFiles(FILEPATH, fileList);
						if (fileList.size() == 0) res = "服务器暂无文件";
						for (int i = 0; i < fileList.size(); i++) {
							res.append(fileList[i]);
							if (i != fileList.size() - 1) res.append("\n");
						}
						send(((LOGINCMD*)c)->client, res.c_str(), sizeof(char) * res.size(), 0);
						continue;
					}
					string tem(buffer);
					if (tem.size() > 4) {
						if (tem.substr(0, 3) == "get") {
							string fileName(tem.substr(4, tem.size() - 4));
							bool isExist = false;
							for (auto item : fileList) {
								if (item == fileName) {
									isExist = true;
									break;
								}
							}
							if (isExist) {
								string path(FILEPATH);
								path = path + "\\" + fileName;
								cout << inet_ntoa(((LOGINCMD*)c)->addr.sin_addr) << ":" << ntohs(((LOGINCMD*)c)->addr.sin_port) << "准备接受文件" << path << endl;
								send(((LOGINCMD*)c)->client, "Prepare get file...", sizeof("Prepare get file..."), 0);
								send(((LOGINCMD*)c)->client, fileName.c_str(), sizeof(char) * fileName.size(), 0);
								FILE* fp = fopen(path.c_str(), "rb");
								if (fp == NULL) {
									cout << inet_ntoa(((LOGINCMD*)c)->addr.sin_addr) << ":" << ntohs(((LOGINCMD*)c)->addr.sin_port) << "文件无法打开" << path << endl;
									continue;
								}
								ZeroMemory(buffer, MAXSIZE * sizeof(char));
								int file_length = 0;
								unsigned long long file_send = 0;
								bool transmitOver = true;
								while ((file_length = fread(buffer, sizeof(char), MAXSIZE, fp)) > 0) {
									if (send(((LOGINCMD*)c)->client, buffer, file_length, 0)<0) {
										cout << inet_ntoa(((LOGINCMD*)c)->addr.sin_addr) << ":" << ntohs(((LOGINCMD*)c)->addr.sin_port) << "断开连接，传输结束" << endl;
										fclose(fp);
										transmitOver = false;
										break;
									}
									ZeroMemory(buffer, MAXSIZE * sizeof(char));
									file_send += file_length;
								}
								fclose(fp);
								if (transmitOver) {
									cout << inet_ntoa(((LOGINCMD*)c)->addr.sin_addr) << ":" << ntohs(((LOGINCMD*)c)->addr.sin_port) << "传输完毕" << path << "共" << file_send << "字节" << endl;
									send(((LOGINCMD*)c)->client, "#####", sizeof("#####"), 0);
								}
								continue;
							}
							else {
								send(((LOGINCMD*)c)->client, "File not exist", sizeof("File not exist"), 0);
								continue;
							}
						}
					}
					cout << inet_ntoa(((LOGINCMD*)c)->addr.sin_addr) << ":" << ntohs(((LOGINCMD*)c)->addr.sin_port) << "命令错误" << buffer << endl;
					send(((LOGINCMD*)c)->client, "error command", sizeof("error command"), 0);
				}
				delete c;
				_endthreadex(0);
				return 0;
			}
		}
	}
}

int main() {
	if (!initialize()) {
		cout << "初始化失败" << endl;
		return -1;
	}
	//将accept加入到集合中
	struct fd_set rfds;
	struct timeval timeout = { 0,200 };
	FD_ZERO(&rfds);
	FD_SET(serv_socket, &rfds);
	HANDLE hThread;
	LOGINCMD* loginParam;
	CTRLCMD* ctrlParam;
	cout << "开始侦听" << INETADDR << ":" << CTRLPORT << endl;
	while (1) {
		fd_set rfds_copy = rfds;
		//把副本放进select经过处理
		int status_code = select(0, &rfds_copy, NULL, NULL, NULL);
		if (status_code > 0) {
			for (u_int i = 0; i < rfds.fd_count; i++) {
				//如果需要读写
				if (FD_ISSET(rfds.fd_array[i], &rfds_copy)) {
					//如果服务sock有数据，说明有客户端请求连接
					if (rfds.fd_array[i] == serv_socket) {
						if (rfds.fd_count < FD_SETSIZE) {
							SOCKADDR_IN client_addr;
							int len = sizeof(client_addr);
							//建立连接
							SOCKET client_socket = accept(serv_socket, (sockaddr*)(&client_addr), &len);
							if (client_socket == SOCKET_ERROR) {
								cout << "连接失败" << endl;
								closesocket(client_socket);
								continue;
							}
							else {
								//开启线程登录
								//如果验证成功就加入到fd_set
								cout << inet_ntoa(client_addr.sin_addr) << ":" << ntohs(client_addr.sin_port) << "请求连接" << endl;
								loginParam = new LOGINCMD;
								loginParam->client = client_socket;
								loginParam->addr = client_addr;
								loginParam->rfds = rfds;
								hThread = (HANDLE)_beginthreadex(NULL, 0, &LoginThread, (LPVOID)loginParam, 0, 0);
								//如果不close则会消耗内核资源
								CloseHandle(hThread);
							}
							
						}
						else {
							cout << "超出FD_SET最大连接数" << endl;
							continue;
						}
					}
				}
			}
		}
		else if (status_code == 0) {
			cout << "timeout" << endl;
			continue;
		}
		else {
			cout << "Error!!" << endl;
			return 0;
		}
	}

	return 0;
}
