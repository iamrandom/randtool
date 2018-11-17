
#include <stdio.h>
#include <string.h>
#include <io.h>
#include <windows.h>

int check_dir(const char* dir_path)
{
	WIN32_FIND_DATA wfd;
	HANDLE hFind;
	int res;
	hFind = FindFirstFile(dir_path, &wfd);

	if( (hFind  != INVALID_HANDLE_VALUE) && (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
	{
		res = 1;
	}
	else
	{
		res = 0;
	}
	FindClose(hFind);
	return res;
}

int check_swf_file(char* path)
{
	FILE *file;
	char c;
	file = fopen(path, "rb");
	c = fgetc(file);
	c = fgetc(file);
	if(c != 0x57){
		fclose(file);
		return 0;
	}
	fclose(file);
	return 1;
}

void command_swf(char* filename, char* error_folder)
{
	char out_fold_path[256];
	int filenamesize;
	int error_folder_size;
	filenamesize = strlen(filename);
	error_folder_size = strlen(error_folder);
	if( !check_swf_file(filename) )
	{
		memcpy(out_fold_path, error_folder, error_folder_size);
		out_fold_path[error_folder_size] = '\\';
		memcpy(out_fold_path + error_folder_size + 1, filename, filenamesize);
		out_fold_path[error_folder_size + 1 + filenamesize] = 0;
		CopyFile(filename, out_fold_path, FALSE);
		remove(filename);		
	}
}




int main(int argc, char** argv)
{
	
	char out_folder_name[] = "error_file";
	
	long Handle;

	struct _finddata_t fileinfo;
	
	if(!check_dir(out_folder_name))
	{
		mkdir(out_folder_name);
	}

	Handle = _findfirst("*.swf", &fileinfo);
	if(Handle == -1)
	{
		return;
	}
	command_swf(fileinfo.name, out_folder_name);
	while(_findnext(Handle, &fileinfo) == 0)
	{
		command_swf(fileinfo.name, out_folder_name);
	}
	_findclose(Handle);
	return 0;
}