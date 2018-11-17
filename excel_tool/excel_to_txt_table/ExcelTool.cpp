
#import "libid:2DF8D04C-5BFA-101B-BDE5-00AA0044DE52" \
    rename("RGB", "MSORGB") \
    rename("DocumentProperties", "MSODocumentProperties")

using namespace Office;

#import "libid:0002E157-0000-0000-C000-000000000046"
using namespace VBIDE;

#import "libid:00020813-0000-0000-C000-000000000046" \
    rename("DialogBox", "ExcelDialogBox") \
    rename("RGB", "ExcelRGB") \
    rename("CopyFile", "ExcelCopyFile") \
    rename("ReplaceText", "ExcelReplaceText") \
    no_auto_exclude


#include <iostream>
#include <sstream>
using namespace std;


// 宽字符转UTF8 
string EncodeUtf8(wstring in)
{
	string s(in.length() * 3 + 1, ' ');
	size_t len = ::WideCharToMultiByte(CP_UTF8, 0, in.c_str(), in.length(), &s[0], s.length(), NULL, NULL);
	s.resize(len);
	return s;
}

static Excel::_ApplicationPtr xl;
char SPP = '/';

void do_excel(const char* path, const char* filename, const char* save_path);

#include <io.h>
#include <direct.h>
void iter_file(const char* path, const char* filename, const char* save_path)
{
	long handle;
	struct _finddata_t file_info;

	string sdir(save_path);
	if (_access(save_path, 0) == -1)
	{
		_mkdir(save_path);
	}
	string f(path);
	f += filename;
	if ((handle = _findfirst(f.c_str(), &file_info)) != -1L)
	{
		if (!(file_info.attrib & _A_HIDDEN))
		{
			cout << path << file_info.name << endl;
			do_excel(path, file_info.name, save_path);
			while (_findnext(handle, &file_info) == 0)
			{
				if (file_info.attrib & _A_HIDDEN) continue;
				cout << path << file_info.name << endl;
				do_excel(path, file_info.name, save_path);
			}
			_findclose(handle);
		}
	}
	
	string f2(path);
	f2 += "*";
	string now_dir(".");
	string pre_dir("..");

	if ((handle = _findfirst(f2.c_str(), &file_info)) != -1L)
	{
		string s(path);
		string s2(save_path);
		if (file_info.attrib == _A_SUBDIR)
		{
			string fn(file_info.name);
			if (!(fn.compare(now_dir) == 0 || fn.compare(pre_dir) == 0))
			{
				s += file_info.name;
				s += SPP;
				s2 += file_info.name;
				s2 += SPP;
				iter_file(s.c_str(), filename, s2.c_str());
			}
		}
		while (_findnext(handle, &file_info) == 0)
		{
			s.assign(path);
			s2.assign(save_path);
			if (file_info.attrib == _A_SUBDIR)
			{
				string fn(file_info.name);
				if (!(fn.compare(now_dir) == 0 || fn.compare(pre_dir) == 0))
				{
					s += file_info.name;
					s += SPP;
					s2 += file_info.name;
					s2 += SPP;
					iter_file(s.c_str(), filename, s2.c_str());
				}
			}
		}
		_findclose(handle);
	}
}

#include<fstream>


bool text_input(stringstream& text, VARTYPE vt, _variant_t& v)
{
	BSTR b;
	switch (vt)
	{
	case VT_LPSTR:
	case VT_BSTR:
		b = v.bstrVal;
		text << EncodeUtf8(b).c_str();
		break;
	case VT_I1:
		text << v.cVal;
		break;
	case VT_I2:
		text << v.iVal;
		break;
	case VT_I4:
		text << v.intVal;
		break;
	case VT_I8:
		text << v.llVal;
		break;
	case VT_INT:
		text << v.intVal;
		break;
	case VT_UI1:
		text << v.uiVal;
		break;
	case VT_UI2:
		text << v.uiVal;
		break;
	case VT_UI4:
		text << v.uintVal;
		break;
	case VT_UI8:
		text << v.ullVal;
		break;
	case VT_UINT:
		text << v.uintVal;
		break;
	case VT_R4:
		text << (long long)v.fltVal;
		break;
	case VT_R8:
		text << v.dblVal;
		break;
	default:
		return false;
	}
	return true;
}

void do_excel(const char* path, const char* filename, const char* save_path)
{
	string fn(path);
	fn += filename;
	Excel::_WorkbookPtr pWorkbook = NULL;

	try
	{
		//cout << "Starting Excel " << SPP << filename << endl;
		_variant_t tt(true);

		pWorkbook  = xl->Workbooks->Open(fn.c_str(), vtMissing, tt);  // open excel file 
		string name(pWorkbook->Name);
		name = name.substr(0, name.size() - 5);
		
		Excel::SheetsPtr sheets = pWorkbook->Worksheets;
		Excel::_WorksheetPtr sheet;
		
		BSTR b;
		for (int i = 0; i < sheets->Count; ++i)
		{
			int head = 0;

			sheet = sheets->Item[i + 1];
			if (sheet != NULL)
			{
				stringstream text;
				//cout << i << ends << sheet->Name << ends << sheet->UsedRange->Column << ends << sheet->UsedRange->Row << endl;
				Excel::RangePtr p = sheet->Range["A1"];
				while (1)
				{
					VARTYPE vt = p->Value.vt;	
					if (!vt) break;
					++head;
					b = p->Value.bstrVal;
					if (head > 1)
					{
						text << "	";
					}
					text_input(text, vt, p->Value);
					p = p->Next;
				}
				if (head == 0) return;
				text << "\n";
				int row = 2;
				while (1)
				{
					stringstream sn;
					sn<< 'A';
					sn << row;
					++row;
					Excel::RangePtr p_row = sheet->Range[sn.str().c_str()];
					stringstream s;
					bool need_stop = true;
					for (int j = 0; j < head; ++j)
					{
						_variant_t v  = p_row->Value;
						VARTYPE vt = v.vt;						
						b = p_row->Value.bstrVal;
						if (j > 0)
						{
							s << "	";
						}
						if (text_input(s, vt, v))
						{
							need_stop = false;
						}
						p_row = p_row->Next;
					}
					if (need_stop)
					{
						break;
					}
					text << s.str();
					text << "\n";
				}
				string sss = text.str();
				//cout << text.str().c_str() << endl;
				ofstream ofile;               //定义输出文件
				string sp(save_path);
				sp += name;
				sp += "#";
				sp += sheet->Name;
				sp += ".txt";
				ofile.open(sp.c_str(), ios::out);     //作为输出文件打开
				ofile.write(sss.c_str(), sss.size());
				ofile.close();
			}
		}
		//cout << "GetWorksheets : " << sheets->Count << endl;
	}
	catch (_com_error& error)
	{
		cout << error.Description() << endl;
	}
	if (pWorkbook)
	{
		pWorkbook->Close(VARIANT_FALSE);
	}
	
}


int main(int argc, char* argv[])
{
	if (argc < 3) return 0;
	CoInitialize(NULL);
	char buffer[MAX_PATH];
	getcwd(buffer, MAX_PATH);
	xl.CreateInstance(L"Excel.Application");
	xl->PutVisible(0, VARIANT_FALSE);

	const char* path = argv[1];

	int index = strlen(path) - 1;
	string paths(path, index + 1);
	if (paths.size() > 2 && path[1] == ':' && path[2] == '/' || path[2] == '\\')
	{

	}
	else
	{
		paths = buffer;
		paths += SPP;
		paths += path;
	}
	
	if (paths[paths.size() - 1]!= '/' && path[paths.size() - 1] != '\\')
	{
		paths += SPP;
	}
	const char* save_path = argv[2];
	index = strlen(save_path) - 1;
	string pathsv(save_path, index + 1);
	if (pathsv.size() > 2 && save_path[1] == ':' && save_path[2] == '/' || save_path[2] == '\\')
	{

	}
	else
	{
		pathsv = buffer;
		pathsv += SPP;
		pathsv += save_path;
	}
	if (pathsv[pathsv.size() - 1] != '/' && pathsv[pathsv.size() - 1] != '\\')
	{
		pathsv += SPP;
	}
	cout << paths.c_str()  << endl;
	cout << pathsv.c_str() << endl;
	iter_file(paths.c_str(), "*.xlsx", pathsv.c_str());
	//do_excel(xl, path, "BuildScience.xlsx", save_path);
	xl->Quit();
	//system("pause");
	CoUninitialize();
	return 0;
}