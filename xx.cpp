#include"stdafx.h"

#define MAX_Time 360000

using namespace std;

//基本数据模型，保存文件名称和最近访问时间
class item
{
private:
	string name;
	time_t access_time;
public:
	item(string name, time_t access_time):name(name),access_time(access_time)
	{}
	string get_name(){
		return this->name;}
	time_t get_time(){
		return this->access_time;}
	~item()
	{}
};

//获取文件的扩展名
char* split(const char* name, char* token)
{
	char str_temp[100]="";
	char* extend_name=NULL;
	strcpy(str_temp, name);
	char*temp=strtok(str_temp,token);
	while(temp!=NULL)
	{
		extend_name=temp;
		temp=strtok(NULL, token);
	}
	return extend_name;
}

//检查是否为指定的扩展名
bool confirmfile(string name)
{
	char* temp=split(name.c_str(), ".");
	/*
	if(!strcmp(temp,"doc")||!strcmp(temp,"xls")||!strcmp(temp,"jpg")||!strcmp(temp,"txt")||!strcmp(temp,"cpp")||!strcmp(temp,"ppt")
		||!strcmp(temp,"caj")||!strcmp(temp,"png")||!strcmp(temp,"html")||!strcmp(temp,"pdf")||!strcmp(temp,"rar")||!strcmp(temp,"zip")
		||!strcmp(temp,"docx")||!strcmp(temp,"pptx"))
	*/
	if(strcmp(temp,"exe")&&strcmp(temp,"lnk")&&strcmp(temp,"LNK")&&strcmp(temp,"ini"))
		return true;
	else
		return false;
}

//按时间排序
bool time_sort(item s, item e)
{
	if(s.get_time()>e.get_time())
		return true;
	else
		return false;
}

//字符转换
wstring code(string sz)
{
    wchar_t wszClassName[256];
    memset(wszClassName,0,sizeof(wszClassName));
	MultiByteToWideChar(CP_ACP,0,sz.c_str(),sz.size(),wszClassName,
        sizeof(wszClassName)/sizeof(wszClassName[0]));
	wstring ws=wszClassName;
	wcout<<ws<<endl;
	return ws ;
}

//记录文件或目录的操作
bool record(fstream& f, string name, bool flag=true)
{
	time_t cur=time(NULL);
	char* t=ctime(&cur);
	if(flag)
		f<<name<<"\t \t"<<t;
	else
		f<<name<<"\t \t failed to copy"<<"\t"<<t<<"error report:"<<GetLastError()<<endl;;
	//f<<endl;
	return true;
}

//读取桌面文件名，并保存到vector中
//版本一 递归检查
/*
int getfile_check_everyfile(string path,string des,fstream& f)
{
	_finddata_t fileinfo;
	long handle=0;
	if((handle=_findfirst(path.c_str(), &fileinfo))==-1)
		return -1;
	while(_findnext(handle, &fileinfo)==0)
	{
		if(strcmp(fileinfo.name,".")&&strcmp(fileinfo.name,"..")&&confirmfile(fileinfo.name)&&!(fileinfo.attrib&_A_SUBDIR))
		{
			wstring desnation=code(des+fileinfo.name);
			wstring source=code(path+fileinfo.name);
			if(CopyFile(source.c_str(),desnation.c_str(),false))
			{
				DeleteFile(source.c_str());
				record(f,path+fileinfo.name,true);
			}
			else
				record(f,path+fileinfo.name,false);
		}
		else if(strcmp(fileinfo.name,".")&&strcmp(fileinfo.name,"..")&&fileinfo.attrib&_A_SUBDIR)
		{
			string temp=string(path.begin(),--path.end())+string(fileinfo.name)+"\\*";
			string temp_des=des+fileinfo.name+"\\";
			if(!_mkdir((path+fileinfo.name).c_str()))
				getfile_check_everyfile(temp,temp_des,f);
			else
				record(f,temp, false);
		}
	}
	//sort(filename.begin(),filename.end(),time_sort);
	return 0;
}
*/

//版本二 只针对文件
int getfile(string path, vector<item>& filename)
{
	_finddata_t fileinfo;
	long handle=0;
	if((handle=_findfirst(path.c_str(), &fileinfo))==-1)
		return -1;
	while(_findnext(handle, &fileinfo)==0)
	{
		if(strcmp(fileinfo.name,".")&&strcmp(fileinfo.name,"..")&&confirmfile(fileinfo.name)&&!(fileinfo.attrib&_A_SUBDIR))
			filename.push_back(item(fileinfo.name, fileinfo.time_access));
	}
	_findclose(handle);
	sort(filename.begin(),filename.end(),time_sort);
	return 0;
}

//版本三 只针对目录
int getdir(string path, vector<item>& filename, bool dir)
{
	_finddata_t fileinfo;
	long handle=0;
	if((handle=_findfirst(path.c_str(), &fileinfo))==-1)
		return -1;
	while(_findnext(handle, &fileinfo)==0)
	{
		bool flag=false;
		if(dir)
		{
			item dir_name=filename.back();
			filename.pop_back();
			if(strcmp(fileinfo.name,".")&&strcmp(fileinfo.name,"..")&&fileinfo.attrib&_A_SUBDIR)
			{
				string temp=string(path.begin(),--path.end())+string(fileinfo.name)+"\\*";
				filename.push_back(item(dir_name.get_name()+"\\"+fileinfo.name,dir_name.get_time()));
				getdir(temp,filename,true);
			}
			else if(strcmp(fileinfo.name,".")&&strcmp(fileinfo.name,".."))
				filename.push_back(item(dir_name.get_name()+"\\"+fileinfo.name,dir_name.get_time()));
			filename.push_back(dir_name);
		}
		else
		{
			if(strcmp(fileinfo.name,".")&&strcmp(fileinfo.name,"..")&&strcmp(fileinfo.name,"mass")&&fileinfo.attrib&_A_SUBDIR)
			{
				string temp=string(path.begin(),--path.end())+string(fileinfo.name)+"\\*";
				filename.push_back(item(fileinfo.name,fileinfo.time_access));
				getdir(temp,filename,true);
			}
		}
	}
	_findclose(handle);
	//sort(filename.begin(),filename.end(),time_sort);
	return 0;
}

//输出vector中的文件
void print(vector<item>& filename)
{
	int len=filename.size();
	for(int i=0; i<len; i++)
	{
		printf(filename[i].get_name().c_str());
		printf("\t %d", filename[i].get_time());
		printf("\n");
	}
}

//复制文件
bool copyfile(vector<item>& filename, string source, string des, fstream& f)
{
	int len=filename.size();
	for (int i=0; i<len; i++)
	{
		//将char类型字符转换为w_char类型
		//locale loc("zh_CN.utf8");
		string temp=source+filename[i].get_name();
		string desName=des+"\\"+filename[i].get_name();
		wstring source=code(temp);
		wstring desnation=code(desName);
		//复制文件
		if(CopyFile(source.c_str(),desnation.c_str(),false))
		{
			//时间检查，删除过期文件
			if(time(NULL)-filename[i].get_time()>MAX_Time)
				DeleteFile(source.c_str());
			record(f,filename[i].get_name(),true);
		}
		else
			record(f,filename[i].get_name(),false);
	}
	return true;
}

//复制目录
bool copydir(vector<item>& filename, string source, string des, fstream& f)
{
	bool check=false;
	int len=filename.size();
	vector<string> fliedir;
	for(int i=0; i<len; i++)
	{
		item it=filename.back();
		filename.pop_back();
		string path=source+"\\"+it.get_name();
		string temp_des=des+"\\"+it.get_name();
		wstring wpath=code(path);
		if(GetFileAttributes(wpath.c_str())&FILE_ATTRIBUTE_DIRECTORY)
		{
			if(access(temp_des.c_str(),0)==-1)
				_mkdir(temp_des.c_str());
			fliedir.push_back(path);
		}
		else
		{
			wstring wdes=code(temp_des);
			if(CopyFile(wpath.c_str(),wdes.c_str(),false))
			{
				record(f,it.get_name(),true);
				DeleteFile(wpath.c_str());
			}
			else
				record(f,it.get_name(),false);
		}
	}
	while(fliedir.size())
	{
		if(_rmdir(fliedir.back().c_str()))
			record(f,fliedir.back(),false);
		fliedir.pop_back();
	}
	return true;
}

//执行
void exe()
{
	string const getfile_path="C:\\Users\\ASUS\\Desktop\\*";
	string const source_path="C:\\Users\\ASUS\\Desktop\\";
	string const des_path="C:\\Users\\ASUS\\Desktop\\mass";
	string const record_path=des_path+"\\record.txt";
	fstream fin(record_path,ios::app);
	vector<item> v;
	int flag=getfile(getfile_path, v);
	copyfile(v,source_path,des_path, fin);
	v.clear();
	vector <item> dir;
	flag=getdir(getfile_path, dir, false);
	copydir(dir,source_path,des_path, fin);
	fin.flush();
	fin.close();
}

int main()
{
	//exe();
	return 0;
}