/*
TCP�ͻ��˴���
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
string Serveripstr = "";  //ftp ������ip��ַ

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
		//serAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");  //����Ǿɵģ�vs2013�汾֮ǰ�õĺ���
		inet_pton(AF_INET, Serveripstr.c_str(), &serAddr.sin_addr.s_addr);   //�����µ�ip��ַת������
		if (connect(sclient, (sockaddr *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
		{
			//����ʧ�� 
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
			indata = indata + "\r\n"; //������־
			const char * sendData;
			sendData = indata.c_str();   //stringתconst char* 
		
			/*
			send()������������ָ����socket�����Է�����
			int send(int s, const void * msg, int len, unsigned int flags)
				sΪ�ѽ��������ӵ�socket��msgָ���������ݣ�len��Ϊ���ݳ��ȣ�����flagsһ����0
				�ɹ��򷵻�ʵ�ʴ��ͳ�ȥ���ַ�����ʧ�ܷ���-1������ԭ�����error
			*/
			send(sclient, sendData, strlen(sendData), 0);
			
			char recData[2049];
			int ret = recv(sclient, recData, 2049, 0);
			if (ret > 0)
			{
				recData[ret] = 0x00;
				cout << recData << endl;
				if (strcmp("125 Data connection already open; transfer starting.", recData) == 0) {
					//�ӷ����������ļ�
					indata.pop_back(); indata.pop_back();
					istringstream iss(indata); string sub[2];
					for (size_t i = 0; i < 2; i++) iss >> sub[i];
					openDataPipe_revFile(sub[1].c_str());
					cout << "File locate in " << buf1 << "\\" << sub[1] << "\n";

				}
				else if (strcmp("225 Data connection open; no transfer in progress.", recData) == 0) {
					//�ӱ����ϴ��ļ�
					indata.pop_back(); indata.pop_back();
					istringstream iss(indata); string sub[2];
					for (size_t i = 0; i < 2; i++) iss >> sub[i];
					cout << "Upload file��" << buf1 << "\\" << sub[1] << "\n";
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
	string serveripstr = Serveripstr;  //ftp ������ip��ַ
	//serAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");  //����Ǿɵģ�vs2013�汾֮ǰ�õĺ���
	inet_pton(AF_INET, serveripstr.c_str(), &serAddr.sin_addr.s_addr);   //�����µ�ip��ַת������
	if (connect(sclient, (sockaddr *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
	{
		//����ʧ�� 
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

	//�ļ�������Ϻ�ֱ�ӹر��׽��֣��������shutdown().
	fclose(fp);
	//�ر��׽���
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
	string serveripstr = Serveripstr;  //ftp ������ip��ַ
	//serAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");  //����Ǿɵģ�vs2013�汾֮ǰ�õĺ���
	inet_pton(AF_INET, serveripstr.c_str(), &serAddr.sin_addr.s_addr);   //�����µ�ip��ַת������
	if (connect(sclient, (sockaddr *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
	{
		//����ʧ�� 
		cout << "connect error !" << endl;
		closesocket(sclient);
		return;
	}

	//�Զ����Ʒ�ʽ���ļ�
	FILE *fp = fopen(filename, "rb");
	//�ȼ���ļ��Ƿ����
	if (fp == NULL)
	{
		cout << "file not found." << endl;
		closesocket(sclient);
		return;
	}


	//������
	char buffer[BUFSIZE] = { 0 };
	int nCount = 0;
	//ѭ���������ݣ�ֱ���ļ���β
	while ((nCount = fread(buffer, 1, BUFSIZE, fp)) > 0)
		send(sclient, buffer, nCount, 0);
	//�ļ���ȡ��ϣ��Ͽ����������ͻ��˷���FIN��
	shutdown(sclient, SD_SEND);
	//�������ȴ��ͻ��˽�����ϡ�
	recv(sclient, buffer, BUFSIZE, 0);
	cout << "File transfer success!\n";
	fclose(fp);
	//�ر��׽���
	closesocket(sclient);
}