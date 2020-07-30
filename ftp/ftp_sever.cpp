/*
FTP服务端程序
*/

#include<sstream>
#include <regex>
#include <winsock2.h>  
#include <Ws2tcpip.h>



#include"fileOp.h"



#pragma comment(lib,"ws2_32.lib")  
using namespace std;


HANDLE hMutex = NULL;//互斥量
const int BUFSIZE = 1024;
int client_count = 0;
int client_total = 0;

struct clientInfo {
	SOCKET sClient;
	sockaddr_in remoteAddr;
};

struct cmdInfo {
	string part1;
	string part2;
};

/*
参数一：需要解析的FTP命令。可能包含的命令有：USER、PASS、SIZE、REST、CWD、RETR、PASV、PORT、QUIT
参数二：解析完成后的命令组结构
*/
void parse_command(const char* data, cmdInfo& cmd_info) {

	cmd_info.part1 = "";
	cmd_info.part2 = "";

	string cmd = data;
	regex reg("([A-Z]+) *([^ ]*) *\\r\\n");

	match_results<string::iterator> res1;
	if (regex_search(cmd.begin(), cmd.end(), res1, reg))
	{
		int count = -1;
		match_results<string::iterator>::const_iterator it;
		for (it = res1.begin(); it != res1.end(); ++it) {
			count++;
			if (it->length() == cmd.length()) continue;
			if (count == 1 && it->length() > 0) {
				//cout << it->length() << "  第一个参数: " << it->str() << endl;
				cmd_info.part1 = it->str();
			}
			else if (count == 2 && it->length() > 0) {
				//cout << it->length() << "  第二个参数: " << it->str() << endl;
				cmd_info.part2 = it->str();
			}
		}
	}
}

/*
子线程运行的方法
参数：传入子线程的参数(void*)
*/
DWORD WINAPI handleConnection(LPVOID lpParamter)
{
	clientInfo* c_info = new clientInfo;
    c_info = (clientInfo*)lpParamter;

	char revData[255];
	char str[INET_ADDRSTRLEN];
	WaitForSingleObject(hMutex, INFINITE);
	int clientid = ++client_total;
	ReleaseMutex(hMutex);
	cout << "接收到一个客户端连接，IP:" << inet_ntop(AF_INET, &(c_info->remoteAddr.sin_addr), str, sizeof(str)) <<
		"     当前在线客户端数："<< ++client_count <<endl;
	
	string user_name = "";
	bool if_login = false;
	string path = getRunningPath();

	while (true)
	{
		//接收数据  
		int ret = recv(c_info->sClient, revData, 255, 0);
		if (ret > 0)
		{
			revData[ret] = 0x00;
			cout <<"与客户端 "<< clientid <<" 的连接："<< "来自客户端数据：";
			printf(revData);
		}
		else {
			//发送数据  
			const char * sendData = "500 Syntax error,command unrecognized.";
			send(c_info->sClient, sendData, strlen(sendData), 0);
			continue;
		}

		cmdInfo cmd_info;
		parse_command(revData, cmd_info);
		cout << "与客户端 " << clientid << " 的连接：" << "解析后：" << cmd_info.part1 << " --- " << cmd_info.part2 <<"\n";

		if(!if_login)
		if (strcmp(cmd_info.part1.c_str(), "QUIT") != 0 && strcmp(cmd_info.part1.c_str(), "HELP") != 0) {
			if (cmd_info.part2.compare("") == 0) { const char * sendData = "530 Not logged in."; send(c_info->sClient, sendData, strlen(sendData), 0); continue; }
		}

		if (strcmp(cmd_info.part1.c_str(),"USER")==0)
		{
			user_name = cmd_info.part2;
			const char * sendData = "331 Username OK, password required.";
			send(c_info->sClient, sendData, strlen(sendData), 0);
		}
		else if(strcmp(cmd_info.part1.c_str(), "PASS")==0)
		{
			//登陆成功 
			if (cmd_info.part2.compare("123456")==0)
			{
				if_login = true;
				const char * sendData = "230 User logged in,proceed.";
				send(c_info->sClient, sendData, strlen(sendData), 0);
			}
			else {
				const char * sendData = "530 Not logged in.";
				send(c_info->sClient, sendData, strlen(sendData), 0);
			}
		}
		else if (strcmp(cmd_info.part1.c_str(), "QUIT") == 0)
		{
			closesocket(c_info->sClient);
			break;
		}
		else if (strcmp(cmd_info.part1.c_str(), "HELP") == 0) 
		{
		    string temp = string("USER<SP><username>\n") +
				string("PASS<SP><password>\n") +
				string("QUIT\n") +
				string("SIZE<SP><filename>\n") +
				string("LS<SP>[<-l>]\n") +
				string("CWD<SP><dirname>\n") +
				string("CD<SP><directoryname>\n") +
				string("MKD<SP><dirname>\n") +
				string("PWD\n") +
				string("CDUP\n") +
				string("RMD<SP><filename>\n") +
				string("RETR<SP><filename>\n") +
				string("STOR<SP><filename>\n") +
				string("HELP");
			const char * sendData = temp.c_str();
			send(c_info->sClient, sendData, strlen(sendData), 0);
		}
		else if (!if_login) {
			const char * sendData = "530 Not logged in.";
			send(c_info->sClient, sendData, strlen(sendData), 0);
		}
		else if (strcmp(cmd_info.part1.c_str(), "SIZE") == 0)
		{
			int fsize = getFileSize(cmd_info.part2);
			stringstream ss; ss << fsize;
			string temp = ss.str();
			const char * sendData = temp.c_str();
			/*const char * ddd = ss.str().c_str(); //ss.str().c_str()是有内容的
			cout <<"额："<<ddd;//这句无输出*/
			send(c_info->sClient, sendData, strlen(sendData), 0);
		}
		else if (strcmp(cmd_info.part1.c_str(), "LS") == 0)
		{
			vector<string> files;
			getFiles(path, files);
			string filesStr = ""; string dividedChar = (cmd_info.part2.compare("-L") == 0 || cmd_info.part2.compare("-l") == 0) ? "\n" : "  ";
			for (int i = 0; i < files.size(); i++)
				filesStr = filesStr + files.at(i) + dividedChar;
			const char * sendData = filesStr.length() == 0 ? " " : filesStr.c_str();
			send(c_info->sClient, sendData, strlen(sendData), 0); 
		}
		else if (strcmp(cmd_info.part1.c_str(), "CWD") == 0)
		{
			if (cdDir(path, cmd_info.part2)) {
				string temp = "200 directory changed to " + cmd_info.part2;
				const char * sendData = temp.c_str();
				send(c_info->sClient, sendData, strlen(sendData), 0);
			}
			else {
				const char * sendData = "431 No such directory.";
				send(c_info->sClient, sendData, strlen(sendData), 0);
			}
		}
		else if (strcmp(cmd_info.part1.c_str(), "CD") == 0)
		{
			if (cd(path, cmd_info.part2)) {
				string temp = "200 directory changed to " + cmd_info.part2;
				const char * sendData = temp.c_str();
				send(c_info->sClient, sendData, strlen(sendData), 0);
			}
			else {
				const char * sendData = "431 No such directory.";
				send(c_info->sClient, sendData, strlen(sendData), 0);
			}
		}
		else if (strcmp(cmd_info.part1.c_str(), "MKD") == 0)
		{
			if (createDirectory(path+"\\"+cmd_info.part2) == 0) {
				string temp = "257 \""+ cmd_info.part2 + "\" directory created";
				const char * sendData = temp.c_str();
				send(c_info->sClient, sendData, strlen(sendData), 0);
			}
			else {
				const char * sendData = "521 taking no action.";
				send(c_info->sClient, sendData, strlen(sendData), 0);
			}
		}
		else if (strcmp(cmd_info.part1.c_str(), "PWD") == 0)
		{
			const char * sendData = path.c_str();
			send(c_info->sClient, sendData, strlen(sendData), 0);
		}
		else if (strcmp(cmd_info.part1.c_str(), "CDUP") == 0)
		{
			cdUp(path);
			string temp = "200 directory changed to " + path;
			const char * sendData = temp.c_str();
			send(c_info->sClient, sendData, strlen(sendData), 0);
		}
		else if (strcmp(cmd_info.part1.c_str(), "RMD") == 0)
		{
			if (removeFile(cmd_info.part2) == 0) {
				const char * sendData = "200 Command okay.";
				send(c_info->sClient, sendData, strlen(sendData), 0);
			}
			else {
		        const char * sendData = "550 Requested action not taken.";
				send(c_info->sClient, sendData, strlen(sendData), 0);
			}
		}
		else if (strcmp(cmd_info.part1.c_str(), "RETR") == 0) //下载文件
		{
				string fileName = path + "\\" + cmd_info.part2;
				//先检查文件是否存在
				//文件名
				const char *filename = fileName.c_str();
				cout << "与客户端 " << clientid << " 的连接：" << "请求文件：" << filename;
				//以二进制方式打开文件
				FILE *fp = fopen(filename, "rb");

				if (fp == NULL)
				{
					const char * sendData = "550 Requested action not taken : file not found.";
					send(c_info->sClient, sendData, strlen(sendData), 0);
					continue;
				}

				//创建套接字  
				SOCKET slisten = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
				if (slisten == INVALID_SOCKET) { cout << "与客户端 " << clientid << " 的连接：" << "create socket error !" << endl; fclose(fp); continue; }

				//绑定IP和端口  
				sockaddr_in sin;
				sin.sin_family = AF_INET;
				sin.sin_port = htons(20); //20号数据端口
				sin.sin_addr.S_un.S_addr = INADDR_ANY;
				if (bind(slisten, (LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR) cout << "与客户端 " << clientid << " 的连接：" << "bind error !" << endl;
				
				//开始监听  
				if (listen(slisten, 5) == SOCKET_ERROR){cout << "与客户端 " << clientid << " 的连接：" << "listen error !" << endl; fclose(fp); continue;}

				//循环接收数据  
				SOCKET sClient;
				sockaddr_in remoteAddr;
				int nAddrlen = sizeof(remoteAddr);
				cout << "\n等待客户端发起数据连接..." << endl;
				//const char * sendData = "225 Data connection open; no transfer in progress.";
				const char * sendData = "125 Data connection already open; transfer starting.";
				send(c_info->sClient, sendData, strlen(sendData), 0);
				//此句阻塞
				sClient = accept(slisten, (SOCKADDR *)&remoteAddr, &nAddrlen);

				if (sClient == INVALID_SOCKET){cout << "与客户端 " << clientid << " 的连接：" << "accept error !" << endl; fclose(fp); closesocket(sClient); closesocket(slisten); continue;}

				char str1[INET_ADDRSTRLEN];
				cout << "与客户端 " << clientid << " 的连接：" << "接收到一个客户端数据连接，IP:" << inet_ntop(AF_INET, &(remoteAddr.sin_addr), str1, sizeof(str1)) << endl;
				//缓冲区
				char buffer[BUFSIZE] = { 0 };
				int nCount = 0;
				//循环发送数据，直到文件结尾
				while ((nCount = fread(buffer, 1, BUFSIZE, fp)) > 0)
					send(sClient, buffer, nCount, 0);
				//文件读取完毕，断开输出流，向客户端发送FIN包
				shutdown(sClient, SD_SEND);
				//阻塞，等待客户端接收完毕。
				recv(sClient, buffer, BUFSIZE, 0);
				cout << "与客户端 " << clientid << " 的连接：" << "File transfer success!\n";
				fclose(fp);
				//关闭套接字
				closesocket(sClient);
				closesocket(slisten);

		}
		else if (strcmp(cmd_info.part1.c_str(), "STOR") == 0)
		{
				string fileName = path + "\\" + cmd_info.part2;
				//先检查文件是否存在
				//文件名
				const char *filename = fileName.c_str();  
				cout << "与客户端 " << clientid << " 的连接：" << "客户端上传文件至：" << filename;
				FILE *fp = fopen(filename, "wb");
				if (fp == NULL)
				{
					const char * sendData = "550 Requested action not taken : Cannot make file.";
					send(c_info->sClient, sendData, strlen(sendData), 0);
					cout << "与客户端 " << clientid << " 的连接：" << "Cannot make file.\n" << endl;
					continue;
				}

				//创建套接字  
				SOCKET slisten = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
				if (slisten == INVALID_SOCKET) { cout << "与客户端 " << clientid << " 的连接：" << "create socket error !" << endl; fclose(fp); continue; }

				//绑定IP和端口  
				sockaddr_in sin;
				sin.sin_family = AF_INET;
				sin.sin_port = htons(20); //20号数据端口
				sin.sin_addr.S_un.S_addr = INADDR_ANY;
				if (bind(slisten, (LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR) cout << "与客户端 " << clientid << " 的连接：" << "bind error !" << endl;

				//开始监听  
				if (listen(slisten, 5) == SOCKET_ERROR) { cout << "与客户端 " << clientid << " 的连接：" << "listen error !" << endl; fclose(fp); continue; }

				SOCKET sClient;
				sockaddr_in remoteAddr;
				int nAddrlen = sizeof(remoteAddr);
				
				cout << "与客户端 " << clientid << " 的连接：" << "等待客户端发起数据连接..." << endl;
				const char * sendData = "225 Data connection open; no transfer in progress.";
				send(c_info->sClient, sendData, strlen(sendData), 0);

				//此句阻塞
				sClient = accept(slisten, (SOCKADDR *)&remoteAddr, &nAddrlen);

				if (sClient == INVALID_SOCKET) {cout << "与客户端 " << clientid << " 的连接：" << "accept error !" << endl; fclose(fp);closesocket(sClient);closesocket(slisten); continue; }

				char str1[INET_ADDRSTRLEN];
				cout << "与客户端 " << clientid << " 的连接：" << "接收到一个客户端数据连接，IP:" << inet_ntop(AF_INET, &(remoteAddr.sin_addr), str1, sizeof(str1)) << endl;
				
				char buffer[BUFSIZE] = { 0 };
				int nCount = 0;
				while ((nCount = recv(sClient, buffer, BUFSIZE, 0)) > 0)
					fwrite(buffer, 1, nCount, fp);

				cout << "与客户端 " << clientid << " 的连接：" << "File transfer success!\n";

				//文件接收完毕后直接关闭套接字，无需调用shutdown().
				fclose(fp);
				//关闭套接字
				closesocket(sClient);
				closesocket(slisten);
		}
		
		else {
			const char * sendData = "500 Syntax error,command unrecognized.";
			send(c_info->sClient, sendData, strlen(sendData), 0);
		}
		
		



		
	}
	client_count--;
	
	return 0L;//表示返回的是long型的0
}


int main(int argc, char* argv[])
{
	//初始化互斥量
	hMutex = CreateMutex(NULL, FALSE, "ClientCount");

	//初始化WSA  
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	if (WSAStartup(sockVersion, &wsaData) != 0)
	{
		return 0;
	}

	//创建套接字  
	SOCKET slisten = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (slisten == INVALID_SOCKET)
	{
		cout << "create socket error !" << endl;
		return 0;
	}

	//绑定IP和端口  
	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(21);
	sin.sin_addr.S_un.S_addr = INADDR_ANY;
	if (bind(slisten, (LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR)
	{
		cout << "bind error !" << endl;
	}

	//开始监听  
	if (listen(slisten, 5) == SOCKET_ERROR)
	{
		cout << "listen error !" << endl;
		return 0;
	}

	//循环接收数据  
	SOCKET sClient;
	sockaddr_in remoteAddr;
	int nAddrlen = sizeof(remoteAddr);

	while (true)
	{
		cout << "\n等待客户端发起连接..." << endl; 
		//此句阻塞
		sClient = accept(slisten, (SOCKADDR *)&remoteAddr, &nAddrlen);

		if (sClient == INVALID_SOCKET)
		{
			cout << "accept error !" << endl;
			continue;
		}

		//传入子线程的参数
		clientInfo *c_info = new clientInfo;
		c_info->remoteAddr = remoteAddr;
		c_info->sClient = sClient;
		//开启子线程
		HANDLE hThread = CreateThread(NULL, 0, handleConnection, (void*)c_info, 0, NULL);
		CloseHandle(hThread);
	}

	closesocket(slisten);
	WSACleanup();
	return 0;
}