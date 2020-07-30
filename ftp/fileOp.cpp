#include "fileOp.h"

std::string getRunningPath() {
	char buf[256];
	_getcwd(buf, sizeof(buf));
	return buf;
}

void getFiles(std::string path, std::vector<std::string>& files)
{
	//文件句柄  
	long   hFile = 0;
	//文件信息  
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
	//文件句柄  
	long   hFile = 0;
	//文件信息  
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
	//文件句柄  
	long   hFile = 0;
	//文件信息  
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
显示指定目录下的所有文件及文件夹名
参数：指定目录path
*/
void ls(std::string path) {
	std::vector<std::string> files;

	//获取该路径下的所有文件  
	getFiles(path, files);

	for (int i = 0; i < files.size(); i++)
	{
		std::cout << files[i].c_str() << std::endl;
	}
}

/*
访问上一级目录
参数：当前目录path
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
返回值：表示访问某个文件夹是否成功
参数：源目录path，文件夹名directory
*/
bool cd(std::string& path,std::string directory) {
	if (!ifDirectoryExists(path, directory)) return false;
	path.append("\\").append(directory);
	return true;
}

/*
返回值：表示cd某个新绝对目录是否成功
参数：源目录path，新目录newPath
*/
bool cdDir(std::string& path, std::string newPath) {
	if (newPath.back() != '/' && newPath.back() != '\\') newPath.append("\\");
	if (_access(newPath.c_str(), 0) == -1) return false;
	path.assign(newPath);
	return true;
}

/*
返回值：-1表示创建失败，0表示创建成功
参数：要创建的目录名：绝对路径和相对路径均可
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
删除名称为filename的文件
*/
int removeFile(std::string filename) {
	return remove(filename.c_str());
}

/*
删除名称为dirname的空目录
*/
int removeDir(std::string dirname) {
	return RemoveDirectory(dirname.c_str());
}

/*
删除文件或文件夹及里面的文件
*/
void removeDAF(std::string dir) {
	if (dir.back() != '\\'&&dir.back() != '/') dir.append("\\");
	std::vector<std::string> files;

	//获取该路径下的所有文件  
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
返回指定文件大小，文件不存在或出错返回-1.否则返回文件大小
参数：文件路径filepath
*/
int getFileSize(std::string filepath) {

	FILE *pFile;
	pFile = fopen(filepath.c_str(), "rb");  //获取已打开文件的指针
	fseek(pFile, 0, SEEK_END);  //先用fseek将文件指针移到文件末尾
	return ftell(pFile);    //再用ftell获取文件内指针当前的文件位置。
	//这个位置就是文件大小。
}
