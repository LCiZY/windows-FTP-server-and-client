/*
TCP客户端代码
*/

#include<winsock2.h>
#include <Ws2tcpip.h>
#include<iostream>
#include<string>
#include<sstream>
#include <direct.h>

using namespace std;
#pragma comment(lib, "ws2_32.lib")

void UPPERCMD(string& cmd);
void openDataPipe_revFile(const char* filename);
void openDataPipe_sendFile(const char* filename);

const int BUFSIZE = 1024;
string Serveripstr = "";  //ftp 服务器ip地址

int main()
{
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA data;
	if (WSAStartup(sockVersion, &data) != 0)
	{
		return 0;
	}

	char buf1[256];
	_getcwd(buf1, sizeof(buf1));

	
		SOCKET sclient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (sclient == INVALID_SOCKET)
		{
			cout << "invalid socket!" << endl;
			return 0;
		}

		sockaddr_in serAddr;
		serAddr.sin_family = AF_INET;
		serAddr.sin_port = htons(21);
		
		cout << "please input server ip:"; cin >> Serveripstr;
		//serAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");  //这句是旧的，vs2013版本之前用的函数
		inet_pton(AF_INET, Serveripstr.c_str(), &serAddr.sin_addr.s_addr);   //这是新的ip地址转换函数
		if (connect(sclient, (sockaddr *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
		{
			//连接失败 
			cout << "connect error !" << endl;
			closesocket(sclient);
			system("pause");
			return 0;
		}

		string indata;
		while (true) {
			cout << "ftp:" + Serveripstr + ">";
			char linedata[65];
			cin.getline(linedata, 65);  
			indata = linedata; UPPERCMD(indata);   if (indata.compare("CLEAR") == 0) { system("cls"); continue; }
			indata = indata + "\r\n"; //结束标志
			const char * sendData;
			sendData = indata.c_str();   //string转const char* 
		
			/*
			send()用来将数据由指定的socket传给对方主机
			int send(int s, const void * msg, int len, unsigned int flags)
				s为已建立好连接的socket，msg指向数据内容，len则为数据长度，参数flags一般设0
				成功则返回实际传送出去的字符数，失败返回-1，错误原因存于error
			*/
			send(sclient, sendData, strlen(sendData), 0);
			
			char recData[2049];
			int ret = recv(sclient, recData, 2049, 0);
			if (ret > 0)
			{
				recData[ret] = 0x00;
				cout << recData << endl;
				if (strcmp("125 Data connection already open; transfer starting.", recData) == 0) {
					//从服务器下载文件
					indata.pop_back(); indata.pop_back();
					istringstream iss(indata); string sub[2];
					for (size_t i = 0; i < 2; i++) iss >> sub[i];
					openDataPipe_revFile(sub[1].c_str());
					cout << "File locate in " << buf1 << "\\" << sub[1] << "\n";

				}
				else if (strcmp("225 Data connection open; no transfer in progress.", recData) == 0) {
					//从本地上传文件
					indata.pop_back(); indata.pop_back();
					istringstream iss(indata); string sub[2];
					for (size_t i = 0; i < 2; i++) iss >> sub[i];
					cout << "Upload file：" << buf1 << "\\" << sub[1] << "\n";
					openDataPipe_sendFile(sub[1].c_str());
				}
			}
			if (indata.compare("QUIT\r\n") == 0) break;
		}

	closesocket(sclient);
	

	WSACleanup();

	system("pause");
	return 0;
}



void UPPERCMD(string& cmd) {
	for (int i = 0; i < cmd.length(); i++)
	{
		if (cmd[i] == ' ') break;
		if (cmd[i] >= 97 && cmd[i] <= 122)
			cmd[i] = cmd[i] - 32;
	}
}

void openDataPipe_revFile(const char* filename) {

	

	SOCKET sclient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sclient == INVALID_SOCKET)
	{
		cout << "invalid socket!" << endl;
		return ;
	}

	sockaddr_in serAddr;
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(20);
	string serveripstr = Serveripstr;  //ftp 服务器ip地址
	//serAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");  //这句是旧的，vs2013版本之前用的函数
	inet_pton(AF_INET, serveripstr.c_str(), &serAddr.sin_addr.s_addr);   //这是新的ip地址转换函数
	if (connect(sclient, (sockaddr *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
	{
		//连接失败 
		cout << "connect error !" << endl;
		closesocket(sclient);
		return ;
	}

	FILE *fp = fopen(filename, "wb");
	if (fp == NULL)
	{
	    cout << "Cannot open file.\n" << endl;
		closesocket(sclient);
		return;
	}



	char buffer[BUFSIZE] = { 0 };
	int nCount = 0;
	while ((nCount = recv(sclient, buffer, BUFSIZE, 0)) > 0)
		fwrite(buffer, 1, nCount, fp);

	printf("File transfer success!\n");

	//文件接收完毕后直接关闭套接字，无需调用shutdown().
	fclose(fp);
	//关闭套接字
	closesocket(sclient);

}

void openDataPipe_sendFile(const char* filename) {
	
	SOCKET sclient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sclient == INVALID_SOCKET)
	{
		cout << "invalid socket!" << endl;
		return;
	}

	sockaddr_in serAddr;
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(20);
	string serveripstr = Serveripstr;  //ftp 服务器ip地址
	//serAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");  //这句是旧的，vs2013版本之前用的函数
	inet_pton(AF_INET, serveripstr.c_str(), &serAddr.sin_addr.s_addr);   //这是新的ip地址转换函数
	if (connect(sclient, (sockaddr *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
	{
		//连接失败 
		cout << "connect error !" << endl;
		closesocket(sclient);
		return;
	}

	//以二进制方式打开文件
	FILE *fp = fopen(filename, "rb");
	//先检查文件是否存在
	if (fp == NULL)
	{
		cout << "file not found." << endl;
		closesocket(sclient);
		return;
	}


	//缓冲区
	char buffer[BUFSIZE] = { 0 };
	int nCount = 0;
	//循环发送数据，直到文件结尾
	while ((nCount = fread(buffer, 1, BUFSIZE, fp)) > 0)
		send(sclient, buffer, nCount, 0);
	//文件读取完毕，断开输出流，向客户端发送FIN包
	shutdown(sclient, SD_SEND);
	//阻塞，等待客户端接收完毕。
	recv(sclient, buffer, BUFSIZE, 0);
	cout << "File transfer success!\n";
	fclose(fp);
	//关闭套接字
	closesocket(sclient);
}