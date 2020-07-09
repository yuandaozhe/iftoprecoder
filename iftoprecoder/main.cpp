#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#else
#include<unistd.h>
#endif

char* GetCurAppPathA();
void Log(char* fmt, ...);
int main(int narg,char** args)
{
	if(narg<2)return 0;

	char cmd[10240]={0};
	sprintf(cmd,"%smyiftop -n -t -F %s/255.255.255.255",GetCurAppPathA(),args[1]);
	Log("cmd=%s\n",cmd);
	FILE* fp=0;
#ifdef _WIN32
	fp=_popen(cmd,"rt");
#else
	fp=popen(cmd,"r");
#endif

	while(1)
	{
		cmd[0]=0;
		fgets(cmd,10240,fp);
		char* p=strstr(cmd,"=>");
		if(!p)continue;
		p+=2;
		char instr[128]={0};
		char oustr[128]={0};
		sscanf(p,"%s",instr);

		fgets(cmd,10240,fp);
		p=strstr(cmd,"<=");
		if(!p)continue;
		p+=2;
		sscanf(p,"%s",oustr);
		//Total send rate:
		//Total receive rate:

		char totalsnd[128]={0};
		char totalrcv[128]={0};
		while(!strstr(cmd,"Total send rate:"))fgets(cmd,10240,fp);
		sscanf(strstr(cmd,"Total send rate:")+strlen("Total send rate:"),"%s",totalsnd);
		while(!strstr(cmd,"Total receive rate:"))fgets(cmd,10240,fp);
		sscanf(strstr(cmd,"Total receive rate:")+strlen("Total receive rate:"),"%s",totalrcv);
		
		Log("%s(in/out)\t%s\t%s\ttotal=%s\t%s",args[1],instr,oustr,totalrcv,totalsnd);
	}
	fclose(fp);
	return 0;
}
char* GetLogName()
{
	struct sTemp
	{
		sTemp(){temp=new char[10240];memset(temp,0,10240);}
		~sTemp(){delete temp;temp=0;}
		char* temp;
	};
	static sTemp t;
	if(0==t.temp[0])
	{
#ifdef _WIN32
		::GetModuleFileNameA(0,t.temp,10240-1);
#else
		readlink ("/proc/self/exe", t.temp, 10240-1);
#endif
		strcat(t.temp,".log");
	}

	return t.temp;
}
char* GetCurAppPathA()
{
	struct sTemp
	{
		sTemp(){temp=new char[10240];memset(temp,0,10240);}
		~sTemp(){delete temp;temp=0;}
		char* temp;
	};
	static sTemp t;
	if(0==t.temp[0])
	{
#ifdef _WIN32
		::GetModuleFileNameA(0,t.temp,10240-1);
#else
		readlink ("/proc/self/exe", t.temp, 10240-1);
#endif
		for(int i=(int)strlen(t.temp)-1;i>=0;i--)
		{
			if(t.temp[i]=='\\'||t.temp[i]=='/')
			{
				t.temp[i+1]=0;
				break;
			}
		}
	}

	return t.temp;
}
void Log(char* fmt, ...)
{
    va_list args;
    va_start (args, fmt);
	char* tmp=new char[10240];
	memset(tmp,0,10240);
	vsprintf(tmp,fmt, args);

	time_t t;
	tm* time2;
	t=time(NULL);
	time2=localtime(&t);

	FILE* fp=fopen(GetLogName(),"a+");
	if(fp)
	{
		fprintf(fp,"%04d/%02d/%02d %02d:%02d:%02d\t%s\n",time2->tm_year+1900,time2->tm_mon+1,time2->tm_mday,time2->tm_hour,time2->tm_min,time2->tm_sec,tmp);
		fclose(fp);
	}
   
	delete tmp;
    va_end(args);
}

