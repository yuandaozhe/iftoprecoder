#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <string>
using namespace std;
#ifdef _WIN32
#include <windows.h>
#define msleep Sleep
#else
#include<unistd.h>
void msleep(int n)
{
	usleep(n*1000);
}
#endif
const char sh[]="#!/bin/bash\n\
dir=$(pwd)\n\
function mykill()\n\
{\n\
while :\n\
do\n\
        killPid=`ps -ef |grep \"$1\" |grep -v grep | awk '{print $2}'`\n\
        if [ \"$killPid\" != \"\" ]; then\n\
        killall -9 $1\n\
        kill -9 $killPid\n\
        else\n\
        break\n\
        fi\n\
done\n\
}\n\
mykill $dir/myiftop\n\
chmod +x $dir/myiftop\n\
rm -f $dir/cur.myiftop\n\
nohup $dir/myiftop -t -n >>/dev/null &\n\
rm -f $(readlink -f \"$0\")\n\
";
char* GetCurAppPathA();
void Log(char* fmt, ...);
int main(int narg,char** args)
{
	if(narg<2)return 0;

	char cmd[10240]={0};
	sprintf(cmd,"%s/start.sh",GetCurAppPathA());
	{
		FILE* fp=fopen(cmd,"wb+");
		fwrite(sh,strlen(sh),1,fp);
		fclose(fp);

		sprintf(cmd,"sh %s/start.sh",GetCurAppPathA());
		system(cmd);
	}

	char file[1024];
	sprintf(file,"%scur.myiftop",GetCurAppPathA());
	string lastTime;
	while(1)
	{
		msleep(500);
		FILE* fp=fopen(file,"r");
		if(!fp)continue;

		do
		{
			cmd[0]=0;
			fgets(cmd,10240,fp);
			if(::memcmp(cmd,"time=",5))break;

			if(lastTime==cmd)break;
			lastTime=cmd;

			char instr[128]={0};
			char oustr[128]={0};

			while((!strstr(cmd,"=>"))&&(!strstr(cmd,args[1]))&&fgets(cmd,10240,fp));
			if(feof(fp))break;
			sscanf(strstr(cmd,"=>")+2,"%s",oustr);

			while((!strstr(cmd,"<="))&&fgets(cmd,10240,fp));
			if(feof(fp))break;
			sscanf(strstr(cmd,"<=")+2,"%s",instr);

			char totalsnd[128]={0};
			char totalrcv[128]={0};
			while((!strstr(cmd,"Total send rate:"))&&fgets(cmd,10240,fp));
			if(feof(fp))break;
			sscanf(strstr(cmd,"Total send rate:")+strlen("Total send rate:"),"%s",totalsnd);

			while((!strstr(cmd,"Total receive rate:"))&&fgets(cmd,10240,fp));
			if(feof(fp))break;
			sscanf(strstr(cmd,"Total receive rate:")+strlen("Total receive rate:"),"%s",totalrcv);

			Log("%s(in/out)=%s\t%s\ttotal(rcv/snd)=%s\t%s",args[1],instr,oustr,totalrcv,totalsnd);

		}while(0);
		fclose(fp);
	}
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

