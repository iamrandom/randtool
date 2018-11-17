#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wininet.h>
#include <windows.h>

void query_reg_value(char* valueData, LPDWORD valueSize)
{
	HKEY hKey;
	char env[128];
	char* next;
	char* env_value;
	size_t env_size;
	DWORD value_re_size;
	char reg_key[] = "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\User Shell Folders";
	char valuename[] = "Cache";
	if(ERROR_SUCCESS != RegOpenKeyEx(HKEY_CURRENT_USER, reg_key, 0, KEY_ALL_ACCESS, &hKey))
	{
		printf("error : regedit cannot be oppend");
		return;
	}
	if(ERROR_SUCCESS != RegQueryValueEx(hKey, valuename,  0, NULL, valueData, valueSize) )
	{
		*valueSize = 0;
		RegCloseKey(hKey);
		return;
	}
	if(*valueSize > 0)
	{
		if(valueData[0] == '%')
		{
			next = strchr(valueData + 1, (int)'%');
			if(next)
			{
				memcpy(env, valueData + 1, (next - valueData - 1));
				env[next - valueData - 1] = 0;
				env_value = getenv(env);
				if(env_value)
				{
					env_size = strlen(env_value);
					value_re_size = *valueSize - (next + 1 - valueData);
					memcpy(valueData + env_size, next + 1, value_re_size);
					memcpy(valueData, env_value, env_size);
					valueData[env_size + value_re_size] = 0;
					*valueSize = env_size + value_re_size;
				}
			}
		}	
	}
	RegCloseKey(hKey);
}

int	check_ext(const char* pData, char** ext, size_t ext_len)
{
	size_t ext_size;
	size_t i, n;
	char* endPoi;
	if(!pData)
	{
		return 0;
	}
	endPoi = strrchr(pData, (int)'.');
	if(!endPoi)
	{
		return 0;
	}

	for(i = 0; i < ext_len; ++i)
	{
		n = strlen(ext[i]) - 1;
		if(memicmp(endPoi, ext[i], n) == 0)
		{
			return 1;
		}
	}
	return 0;
}

int	check_url(const char* pData, char** url, size_t url_len)
{
	size_t url_size;
	size_t i, n;
	if(!pData)
	{
		return 0;
	}

	for(i = 0; i < url_len; ++i)
	{
		n = strlen(url[i]);
		if(memicmp (pData, url[i], n) == 0)
		{
			return 1;
		}
	}
	return 0;
}

void replace_char(char* pData, char a, char b)
{
	if(!pData) return;
	while(*pData)
	{
		if(*pData == a)
		{
			*pData = b;
		}
		++pData;
	}
}

int check_and_rename_cache_file(const char* parent_path, DWORD len, const char* path, char* pname)
{
	size_t name_size ;
	char* pC;
	if(0 != memicmp (parent_path, path, len))
	{
		return 0;
	}
	pC = strrchr(path, '\\');
	if(pC)
	{
		pC = pC + 1;
		name_size = strlen(path) - (pC - path);
		memcpy(pname, pC, name_size);
	}
	else
	{
		name_size = strlen(path) - len;
		memcpy(pname, path + len, name_size);
	}

	pname[name_size] = 0;

	replace_char(pname, '\\', '_');
	return 1;
}

int check_file_ok(const char* path)
{
	int path_size;
	FILE *file;
	char c;
	path_size = strlen(path);
	if (path_size < 4) return 1;
	if( path[path_size] != 'f') return 1;
	if( path[path_size - 1] != 'w') return 1;
	if( path[path_size - 2] != 's') return 1;
	if( path[path_size - 3] != '.') return 1;

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

void display(INTERNET_CACHE_ENTRY_INFO* entry, char** ext, size_t ext_len, char** url, size_t url_len, char* out_fold)
{
	size_t url_size;
	char temp_path[512];
	char name[256];
	char out_fold_path[128];
	size_t fold_size;
	DWORD temp_size;


	if(ext_len > 0)
	{
		// printf("%s 2\n", ext[0]);
		if(!check_ext(entry->lpszLocalFileName, ext, ext_len))
		{
			return;
		}
	}

	if(url_len > 0)
	{
		// printf("%s 3\n", url[0]);
		if(!check_url(entry->lpszSourceUrlName, url, url_len))
		{
			return;
		}
	}

	query_reg_value(&temp_path[0], &temp_size);
	if(!temp_size)
	{
		return;
	}
	
	if(!check_and_rename_cache_file(temp_path, strlen(temp_path), entry->lpszLocalFileName, &name[0]))
	{
		return;
	}

	if(!check_file_ok(entry->lpszLocalFileName))
	{
		return;
	}

	printf("[%s] ", name);
	printf("%s ", entry->lpszLocalFileName);
	printf("%s ", entry->lpszSourceUrlName);
	printf("\n");

	if(!out_fold)
	{
		return;
	}
	fold_size = strlen(out_fold);
	if(fold_size == 0 || fold_size > 128)
	{
		return;
	}

	memcpy(out_fold_path, out_fold, fold_size);
	out_fold_path[fold_size] = 0;
	if(out_fold_path[fold_size] != '\\')
	{
		strcat(out_fold_path, "\\");
	}
	strcat(out_fold_path, name);
	printf("%s\n", out_fold_path);
	CopyFile(entry->lpszLocalFileName, out_fold_path, FALSE);

}


void get_out_dir(char* out_fold)
{
	size_t fold_size = strlen(out_fold);
	if(fold_size >= 2 && out_fold[0] == out_fold[fold_size] && (out_fold[0] == '"' || out_fold[0] == '\''))
	{
		memcpy(out_fold, out_fold + 1, fold_size - 2);
		out_fold[fold_size - 2] = 0;
	}
}

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


int main(int argc, char** argv)
{
	int uCount = 0;
	char* ext[16];
	size_t ext_len = 0;
	char* url[16];
	size_t url_len = 0;
	size_t i = 0;
	char dest_fold[128];

	HANDLE hCacheDir;
	DWORD MaxEntrySize = 4096;
	DWORD uEntrySize = MaxEntrySize;
	INTERNET_CACHE_ENTRY_INFO* pCacheEntry;

	memset(&ext[0], 0, sizeof(ext));
	memset(&url[0], 0, sizeof(url));
	memset(&dest_fold[0], 0, sizeof(dest_fold));
	if(argc > 1)
	{
		for(i = 1; i < argc; ++i)
		{
			if(argv[i][0] == '.')
			{
				if(ext_len < sizeof(ext)/sizeof(ext[0]))
				{
					ext[ext_len] = argv[i];
					++ext_len;
				}
			}
			else
			{
				if(argv[i][0] == '-' && argv[i][1] =='O')
				{
					memcpy(dest_fold, argv[i] + 2, strlen(argv[i]) - 2);
					get_out_dir(dest_fold);
					if(!check_dir(dest_fold))
					{
						printf("ERROR : not such folder : %s", dest_fold);
						return;
					}
					continue;
				}
				if(url_len < sizeof(url)/sizeof(url[0]))
				{
					url[url_len] = argv[i];
					++url_len;
				}
			}
		}
	}

	pCacheEntry = (INTERNET_CACHE_ENTRY_INFO*)malloc(MaxEntrySize);
	memset(pCacheEntry, 0, MaxEntrySize);
	pCacheEntry->dwStructSize = uEntrySize;
	hCacheDir = FindFirstUrlCacheEntry(NULL, pCacheEntry, &uEntrySize);
	if(!hCacheDir)
	{
		printf("%d\n", GetLastError());
		printf("%d, %d, %d", ERROR_INSUFFICIENT_BUFFER, ERROR_NO_MORE_ITEMS, ERROR_SUCCESS);
		return 0;
	}
	display(pCacheEntry, &ext[0], ext_len, &url[0], url_len, &dest_fold[0]);
	uEntrySize = MaxEntrySize;
	pCacheEntry->dwStructSize = uEntrySize;

	while(FindNextUrlCacheEntry(hCacheDir, pCacheEntry,  &uEntrySize))
	{
		++uCount;
		display(pCacheEntry, &ext[0], ext_len, &url[0], url_len, &dest_fold[0]);
		uEntrySize = MaxEntrySize;
	}
	FindCloseUrlCache(hCacheDir);
	free(pCacheEntry);
	return 0;
}