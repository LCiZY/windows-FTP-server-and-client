#include "fileOp.h"

std::string getRunningPath() {
	char buf[256];
	_getcwd(buf, sizeof(buf));
	return buf;
}

void getFiles(std::string path, std::vector<std::string>& files)
{
	//�ļ����  
	long   hFile = 0;
	//�ļ���Ϣ  
	struct _finddata_t fileinfo;
	std::string p;
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
	{
		do
		{
			if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
				files.push_back(fileinfo.name);
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
}

bool ifFileExists(std::string path, std::string filename) {
	//�ļ����  
	long   hFile = 0;
	//�ļ���Ϣ  
	struct _finddata_t fileinfo;
	std::string p;
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
	{
		do
		{
			if ((fileinfo.attrib &  _A_SUBDIR)) {
				if (strcmp(fileinfo.name, filename.c_str()) == 0)
				return false;
			}
			else {
				if(strcmp(fileinfo.name,filename.c_str())==0)
					return true;
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
	return false;
}

bool ifDirectoryExists(std::string path, std::string dir) {
	//�ļ����  
	long   hFile = 0;
	//�ļ���Ϣ  
	struct _finddata_t fileinfo;
	std::string p;
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
	{
		do
		{
			if ((fileinfo.attrib &  _A_SUBDIR)) {
				if (strcmp(fileinfo.name, dir.c_str()) == 0)
					return true;
			}
			else {
				if (strcmp(fileinfo.name, dir.c_str()) == 0)
					return false;
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
	return false;
}

/*
��ʾָ��Ŀ¼�µ������ļ����ļ�����
������ָ��Ŀ¼path
*/
void ls(std::string path) {
	std::vector<std::string> files;

	//��ȡ��·���µ������ļ�  
	getFiles(path, files);

	for (int i = 0; i < files.size(); i++)
	{
		std::cout << files[i].c_str() << std::endl;
	}
}

/*
������һ��Ŀ¼
��������ǰĿ¼path
*/
void cdUp(std::string& path) {

	for (int i = path.length() - 1; i > 0; i--)
	{
		if (path[i] == '\\' || path[i] == '/') {
			while (path.back() != '\\' && path.back() != '/')
			{
				path.pop_back();
				if (path.back() == '\\' || path.back() == '/') {
					path.pop_back();
					if (path.back() == '\\' || path.back() == '/') {
						path.pop_back();
					}
					return;
				}
			}

		}
	}

}

/*
����ֵ����ʾ����ĳ���ļ����Ƿ�ɹ�
������ԴĿ¼path���ļ�����directory
*/
bool cd(std::string& path,std::string directory) {
	if (!ifDirectoryExists(path, directory)) return false;
	path.append("\\").append(directory);
	return true;
}

/*
����ֵ����ʾcdĳ���¾���Ŀ¼�Ƿ�ɹ�
������ԴĿ¼path����Ŀ¼newPath
*/
bool cdDir(std::string& path, std::string newPath) {
	if (newPath.back() != '/' && newPath.back() != '\\') newPath.append("\\");
	if (_access(newPath.c_str(), 0) == -1) return false;
	path.assign(newPath);
	return true;
}

/*
����ֵ��-1��ʾ����ʧ�ܣ�0��ʾ�����ɹ�
������Ҫ������Ŀ¼��������·�������·������
*/
int createDirectory(std::string path)
{
	if (path.back() != '/'&&path.back() != '\\') path.append("\\");
	int len = path.length();
	char tmpDirPath[256] = { 0 };
	for (int i = 0; i < len; i++)
	{
		tmpDirPath[i] = path[i];
		if (tmpDirPath[i] == '\\' || tmpDirPath[i] == '/')
		{
			if (_access(tmpDirPath, 0) == -1)
			{
				int ret = _mkdir(tmpDirPath);
				if (ret == -1) return ret;
			}
		}
	}
	return 0;
}

/*
ɾ������Ϊfilename���ļ�
*/
int removeFile(std::string filename) {
	return remove(filename.c_str());
}

/*
ɾ������Ϊdirname�Ŀ�Ŀ¼
*/
int removeDir(std::string dirname) {
	return RemoveDirectory(dirname.c_str());
}

/*
ɾ���ļ����ļ��м�������ļ�
*/
void removeDAF(std::string dir) {
	if (dir.back() != '\\'&&dir.back() != '/') dir.append("\\");
	std::vector<std::string> files;

	//��ȡ��·���µ������ļ�  
	getFiles(dir, files);

	for (int i = 0; i < files.size(); i++)
	{
		std::cout << files[i].c_str() << std::endl;
		if (ifFileExists(dir, files[i])) removeFile(dir);
		else { 
			removeDAF(dir + files[i]);
			removeDir(dir + files[i]);
		}
	}
}

/*
����ָ���ļ���С���ļ������ڻ������-1.���򷵻��ļ���С
�������ļ�·��filepath
*/
int getFileSize(std::string filepath) {

	FILE *pFile;
	pFile = fopen(filepath.c_str(), "rb");  //��ȡ�Ѵ��ļ���ָ��
	fseek(pFile, 0, SEEK_END);  //����fseek���ļ�ָ���Ƶ��ļ�ĩβ
	return ftell(pFile);    //����ftell��ȡ�ļ���ָ�뵱ǰ���ļ�λ�á�
	//���λ�þ����ļ���С��
}
