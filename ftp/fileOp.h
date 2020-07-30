#pragma once
#include<string>
#include<vector>
#include<iostream>
#include<stdio.h>
#include<io.h>
#include<direct.h>
#include<Windows.h>


std::string getRunningPath();

void getFiles(std::string path, std::vector<std::string>& files);

bool ifFileExists(std::string path, std::string filename);

bool ifDirectoryExists(std::string path, std::string dir);

void ls(std::string path);

void cdUp(std::string& path);

bool cd(std::string& path,std::string directory);

bool cdDir(std::string& path, std::string newPath);

int createDirectory(std::string path);

int removeFile(std::string filename);

int removeDir(std::string dirname);

void removeDAF(std::string dir);

int getFileSize(std::string filepath);
