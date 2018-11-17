#include <stdio.h>
#include <string.h>
#include <io.h>

#define J_SING 1
#define P_SING 2


void docommad(char* name, int sign, char* swf, char* folder, size_t folderSize)
{
	char comm[2048];
	char destname[2048];
	char type_dir[16];
	size_t name_size;
	size_t type_dir_size;
	name_size = strlen(name);

	printf("!!!!!222!!!!!!!! %s !!!!!\n", folder);

	memcpy(destname, folder, folderSize);
	switch(sign)
	{
		case J_SING:
			sprintf(type_dir, "jpg");
			break;
		case P_SING:
			sprintf(type_dir, "png");
			break;
		default:
			return;
	}
	type_dir_size = strlen(type_dir);


	if(destname[folderSize - 1] != '\\' && destname[folderSize - 1] != '/' )
	{
		destname[folderSize] = '\\';
		
		memcpy(destname + folderSize + 1, type_dir, type_dir_size);
		destname[folderSize + 1 + folderSize + type_dir_size] = 0;
		printf("%s         %s\n", destname, name);
		if(access(destname, 0))
		{
			mkdir(destname);
		}	
	}
	else
	{
		memcpy(destname + folderSize, type_dir, type_dir_size);
		destname[folderSize + folderSize + type_dir_size] = 0;
		if(access(destname, 0))
		{
			mkdir(destname);
		}
	}

	printf("!!!!!!!!!!!!! %s !!!!!\n", destname);

	switch(sign)
	{
	case J_SING:

		sprintf(comm, "swfextract -o %s\\%s.jpg -j %s %s", destname, name, name,  swf);
		printf("%s\n", comm);
		system(comm);
		break;
	case P_SING:
		sprintf(comm, "swfextract -o %s\\%s.png -p %s %s", destname, name, name,  swf);
		printf("%s\n", comm);
		system(comm);
		break;
	}
}

void find_int(char* buf, int sign, char* swf, char* folder)
{
	char* p;
	char *pint1;
	char * pint0;
	char intname[128];
	
	
	int i = 0;
	int a, b;
	size_t folderSize = strlen(folder);
	p = buf;
	pint0 = 0;
	pint1 = 0;
	while(*p)
	{
		++i;
		if( (*p >= '0') && (*p <= '9'))
		{
			if(!pint1)
			{
				pint1 = p;
			}
		}
		else
		{
			if(*p == '-')
			{
				pint0 = pint1;
			}
			else if(pint1)
			{
				if(pint0)
				{
					a = atoi(pint0);
					b = atoi(pint1);
					for(; a <= b; ++a)
					{
						sprintf(intname, "%d", a);
						docommad(intname, sign, swf, folder, folderSize);
					}
					pint0 = 0;
				}
				else
				{
					memcpy(intname, pint1, (size_t)(p - pint1));
					intname[(size_t)(p - pint1)] = 0;
					docommad(intname, sign, swf, folder, folderSize);
				}
				
			}
			pint1 = 0;
		}

		++p;
	}
	if(pint1)
	{
		if(pint0)
		{
			a = atoi(pint0);
			b = atoi(pint1);
			for(; a <= b; ++a)
			{
				sprintf(intname, "%d", a);
				docommad(intname, sign, swf, folder, folderSize);
			}
			pint0 = 0;
		}
		else
		{
			memcpy(intname, pint1, (size_t)(p - pint1));
			intname[(size_t)(p - pint1)] = 0;
			docommad(intname, sign, swf, folder, folderSize);
		}

	}
	// printf("\n");
}


int main(int argc, char** argv)
{
	char* filename;
	char* backup_dir;
	FILE* file;
	char buf[4096];
	char now_names[1024];
	char dir_names[512];

	size_t line_size ;
	size_t back_dir_size;
	int create_dir;
	int line;

	char *end1, *end2, *findchar;

	if(argc < 3) return 0;

	backup_dir = argv[1];
	filename = argv[2];
	if(access(backup_dir, 0))
	{
		mkdir(backup_dir);
	}
	file = fopen(filename, "r");
	char objects[] = "Objects in file ";
	char _j[] = " [-j] ";
	
	char _JPEG[] = "JPEG";
	char _p[] = " [-p] ";
	char _PNG[] = "PNG";
	create_dir = 0;
	line = 0;
	back_dir_size = strlen(backup_dir);
	printf("start to parse %s %s\n", filename, backup_dir);
	while(fgets(buf, sizeof(buf), file))
	{
		printf("%d \n", line);
		fflush(stdout);
		++line;

		if(buf[0] == '#') continue;
		line_size = strlen(buf);
		if(buf[line_size - 1] == '\n') buf[line_size - 1] = 0;
		
		if(line_size == 0) continue;
		if(0 == memcmp(buf, objects, sizeof(objects) - 1))
		{
			create_dir = 0;
			memcpy(now_names, buf + sizeof(objects) -1, line_size -sizeof(objects));
			now_names[line_size -sizeof(objects)] = 0;
			
			
			
			end2 = strrchr(now_names, (int)'.');
			memcpy(dir_names, backup_dir, back_dir_size);
			dir_names[back_dir_size] = 0;
			printf("=========== %s %s \n", dir_names, backup_dir);
			if(dir_names[back_dir_size - 1] != '\\' || dir_names[back_dir_size - 1] != '/')
			{
				dir_names[back_dir_size] = '\\';
				dir_names[back_dir_size + 1] = 0;
			}
			end1 = strrchr(now_names, (int)'\\');
			if(!end1)
			{
				end1 = strrchr(now_names, (int)'/');
			}
			if(end1)
			{	
				memcpy(dir_names + back_dir_size + 1, end1 + 1, (size_t)(end2 - end1) - 1);
				dir_names[back_dir_size + (size_t)(end2 - end1) ] = 0;
				printf("111111=== %s\n", dir_names);
			}
			else
			{
				memcpy(dir_names + back_dir_size + 1, now_names, strlen(now_names) - 1);
				dir_names[back_dir_size + 1 + strlen(now_names) ] = 0;
				printf("22222=== %s\n", dir_names);
			}
		
			end2 = strrchr(now_names, (int)':');
			*end2 = 0;
			
			// printf("[%s]\n", dir_names);
			continue;
		}
	
		if(0 == memcmp(buf, _j, sizeof(_j) - 1))
		{
			if(access(dir_names, 0))
			{
				mkdir(dir_names);
			}
			printf("%s\n", buf);
			findchar = strstr(buf, _JPEG);
			if(findchar)
			{
				printf("[%s] %d\n", dir_names, J_SING);
				find_int(findchar + sizeof(_JPEG) - 1, J_SING, now_names, dir_names);
			}
			continue;
		}
		
		if(0 == memcmp(buf, _p, sizeof(_p) - 1))
		{
			if(access(dir_names, 0))
			{
				mkdir(dir_names);
			}
			findchar = strstr(buf, _PNG);
			if(findchar)
			{
				printf("[%s], %d\n", dir_names, P_SING);
				find_int(findchar + sizeof(_PNG) - 1, P_SING, now_names, dir_names);
			}
			continue;
		}
		
	}
	fclose(file);
	return 0;
}