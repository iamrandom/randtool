

#include <stdio.h>
#include <stdlib.h>
#include <string>
// #include <iostream>
// #include <tchar.h>
#include <windows.h>
// #define _DEBUG

#define TCHAR char



std::string CopyToTempDirectoryIfNeeded(TCHAR* srcbuf)
{
	std::string src(srcbuf);
	TCHAR tempDir[512];
	GetTempPath(512, tempDir);

	std::string dest(tempDir);
	std::string xt = "";
	if(src.length() > 9)
	{
		xt = src.substr(src.length() - 9, 9);
	}

	if (strcmp(xt.c_str(), ".svn-base") == 0) { 
		std::string name = src.substr(src.find_last_of('\\') + 1, src.length() - (src.find_last_of('\\') + 1));
		dest += name;
		dest += src.substr(src.length() - 13, 4);

		BOOL b = CopyFile(srcbuf, dest.c_str(), FALSE);
		LONG l = SetFileAttributes(dest.c_str(), FILE_ATTRIBUTE_NORMAL);
		BOOL b1 = MoveFileEx(dest.c_str(), NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
		return dest;
	}
	else {
		// static int count = 1;
		std::string dir = src.substr(0, src.find_last_of('\\') + 1);

		if (strcmp(tempDir, dir.c_str()) == 0) {
			return src;
		}

		std::string name = src.substr(src.find_last_of('\\') + 1, src.length() - (src.find_last_of('\\') + 1));
		dest += name;

		CopyFile(srcbuf, dest.c_str(), FALSE);
		SetFileAttributes(dest.c_str(), FILE_ATTRIBUTE_NORMAL);	
		MoveFileEx(dest.c_str(), NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
		return dest;
	}
}


std::string GetTortoiseSVNDirectory()
{
	HKEY hkey;
	LONG retCode;
	
	retCode = RegCreateKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\TortoiseSVN", 0, NULL, REG_OPTION_NON_VOLATILE,
		KEY_EXECUTE, NULL, &hkey, NULL);
		
	if (retCode != ERROR_SUCCESS) {
		return "";
	}
	
	BYTE data[1024] = {0};
	DWORD size = 1024;
	RegQueryValueEx(hkey, "Directory", NULL, NULL, data, &size);
	RegCloseKey(hkey);
	
	std::string tortoiseSvnDirectory;
	tortoiseSvnDirectory = (char*)data;
	
	return tortoiseSvnDirectory;
}


std::string GetDiffToolPath()
{
	HKEY hkey;
	LONG retCode;
	
	
	retCode = RegCreateKeyEx(HKEY_CURRENT_USER, "SOFTWARE\\TortoiseSVN", 0, NULL, REG_OPTION_NON_VOLATILE,
		KEY_EXECUTE, NULL, &hkey, NULL);
		
	if (retCode != ERROR_SUCCESS) {
	
		return 0;
	}
	
	BYTE data[1024] = {0};
	DWORD size = 1024;
	RegQueryValueEx(hkey, "Diff", NULL, NULL, data, &size);
	RegCloseKey(hkey);
	
	std::string path;
	path = (char*)data;
	
		if (path.length() > 0 && path[0] == '#')  path = "";

	return path;
}


bool ConvertToText(std::string convexepath, std::string filepath1, std::string filepath2)
{
	std::string command;
	command	= " -f -8 \"";
	command += filepath1;
	command += "\"";

	STARTUPINFO startupinfo;
	PROCESS_INFORMATION proc_info;
	TCHAR command_t[2048];
	lstrcpy(command_t, command.c_str());

	ZeroMemory(&startupinfo, sizeof(startupinfo));
	startupinfo.cb = sizeof(startupinfo);
	startupinfo.dwFlags = STARTF_USESHOWWINDOW;
	startupinfo.wShowWindow = SW_HIDE;

	BOOL ret;
	ret = CreateProcess(convexepath.c_str(), command_t, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, 
		&startupinfo, &proc_info);

	if (!ret) {
		
		DWORD errorcode = GetLastError();

		TCHAR buff[512];
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, errorcode, GetSystemDefaultLangID(), buff, 512, NULL); 

		printf(buff);
		printf("\n");
		return false;
	}

	std::string command2;
	command2 = " -f -8 \"";
	command2 += filepath2;
	command2 += "\"";
	lstrcpy(command_t, command2.c_str());

	STARTUPINFO startupinfo2;
	ZeroMemory(&startupinfo2, sizeof(startupinfo2));
	startupinfo2.cb = sizeof(startupinfo2);
	startupinfo2.dwFlags = STARTF_USESHOWWINDOW;
	startupinfo2.wShowWindow = SW_HIDE;
	PROCESS_INFORMATION proc_info2;
	ret = CreateProcess(convexepath.c_str(), command_t, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, 
		&startupinfo, &proc_info2);

	if (!ret) {
	
		DWORD errorcode = GetLastError();

		TCHAR buff[512];
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, errorcode, GetSystemDefaultLangID(), buff, 512, NULL); 

		printf(buff);
		printf("\n");
		return false;
	}

	WaitForSingleObject(proc_info.hProcess, INFINITE);	
	WaitForSingleObject(proc_info2.hProcess, INFINITE);
	return true;
}


void ReplaceString(std::string& base, std::string oldstr, std::string newstr)
{
	int pos;
	
	while ((pos = base.find(oldstr)) != std::string::npos) {
		base.replace(pos, oldstr.length(), newstr);
	}
}
#define  argc __argc
#define  argv __argv
// #ifdef _DEBUG
// int main(int argc, TCHAR** argv)
// #else
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
// #endif
{
 	if (argc < 3)  return 0;

	std::string registryDiffToolPath = GetDiffToolPath();	
	
	if (registryDiffToolPath.length() == 0) {
	
		registryDiffToolPath = GetTortoiseSVNDirectory();
		
		if (registryDiffToolPath.length() == 0)  return 0;
		
		registryDiffToolPath = "\"" + registryDiffToolPath;
		registryDiffToolPath += "bin\\TortoiseMerge.exe\"  /base:%base /mine:%mine /basename:%bname /minename:%yname";
	}

#ifdef _DEBUG
	
	for (int i=0; i<argc; i++) {
		printf("argv[%d]: %s\n", i, argv[i]);
	}
	
	printf("registryDiffToolPath: %s\n", registryDiffToolPath.c_str());

	// int n;
	// scanf("%d", &n);
#endif

	std::string exepath(argv[0]);
	std::string convexepath;

	convexepath = exepath.substr(0, exepath.find_last_of("\\"));
	convexepath += "\\xdoc2txt.exe";

	std::string srcfile1 = CopyToTempDirectoryIfNeeded(argv[1]);
	std::string srcfile2 = CopyToTempDirectoryIfNeeded(argv[2]);

	BOOL ret;
	ret = ConvertToText(convexepath, srcfile1, srcfile2);
	if (!ret)  return 0;

	if (srcfile1 != argv[1])  DeleteFile(srcfile1.c_str());
	if (srcfile2 != argv[2])  DeleteFile(srcfile2.c_str());

	std::string dstfile1;	
	dstfile1 = "\"";
	dstfile1 += srcfile1.substr(0, srcfile1.find_last_of("."));
	dstfile1 += ".txt\"";

	std::string dstfile2;		
	dstfile2 = "\"";
	dstfile2 += srcfile2.substr(0, srcfile2.find_last_of("."));
	dstfile2 += ".txt\"";

	std::string commandString = registryDiffToolPath;
	
	if (commandString.find("%base") == std::string::npos && commandString.find("%mine") == std::string::npos) {
		commandString += " ";
		commandString += dstfile1;
		commandString += " ";
		commandString += dstfile2;
	}
	else {
		std::string filename1 = dstfile1.substr( dstfile1.find_last_of('\\') + 1, dstfile1.length() - dstfile1.find_last_of('\\') - 2);
		std::string filename2 = dstfile2.substr( dstfile2.find_last_of('\\') + 1, dstfile2.length() - dstfile2.find_last_of('\\') - 2);

		ReplaceString(commandString, "%base", dstfile1);
		ReplaceString(commandString, "%mine", dstfile2);
		ReplaceString(commandString, "%bname", filename1);
		ReplaceString(commandString, "%yname", filename2);
	}

#ifdef _DEBUG
	
	printf("commandString: %s\n", commandString.c_str());

	// scanf("%d", &n);	
#endif

	STARTUPINFO startupinfo;
	PROCESS_INFORMATION proc_info;
	ZeroMemory(&startupinfo, sizeof(startupinfo));
	startupinfo.cb = sizeof(startupinfo);

	TCHAR command_t[2048];
	lstrcpy(command_t, commandString.c_str());

	ret = CreateProcess(NULL, command_t, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, 
		&startupinfo, &proc_info);

	if (!ret) {
	
		DWORD errorcode = GetLastError();

		TCHAR buff[512];
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, errorcode, GetSystemDefaultLangID(), buff, 512, NULL); 

		printf(buff);
		printf("\n");

	
		return 0;
	}
	WaitForSingleObject(proc_info.hProcess, INFINITE);
	MoveFileEx(dstfile1.c_str(), NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
	MoveFileEx(dstfile2.c_str(), NULL, MOVEFILE_DELAY_UNTIL_REBOOT);

	return 0;
}

