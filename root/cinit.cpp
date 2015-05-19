/*
	Small c++ application to initialize the system
	Running N threads using a simple dependency list
	it tend to be faster than alternative using bash interpreter
	g++ -std=c++11 -g -lpthread cinit.cpp -o cinit
*/
#include <mutex>
#include <iostream>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <cstring>
#include <sys/mount.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdarg.h>

#define TASK_ID(x)	\
	x(none)\
	x(hostname) \
	x(deferred) \
	x(procfs) \
	x(sysfs) \
	x(fs)\
	x(bootchart)\
	x(bootchart_end)\
	x(read_ahead)\
	x(devfs)\
	x(udev)\
	x(x11)\
	x(udev_add)\
	x(udev_mtab)\
	x(wait)\
	x(dbus)\
	
#define TO_STRING(id)                 #id
#define TO_NAME(id,...)               TO_STRING(id),
#define TO_ID(id,...)                 id ## _id,

typedef enum   {
    TASK_ID(TO_ID)
} task_id;

const char* getTaskName(task_id id)
{
	 static const char* const names[] =
    {   TASK_ID(TO_NAME) ""};
    if (id >= sizeof(names)/sizeof(*names))
        return "";
    return names[id];
}

enum task_status
{
	waiting, running, done,
};

class task
{
public:
  task (void (*fnc)(void),task_id id,task_id parent = none_id) : fnc(fnc),parent(nullptr),status(waiting), ms(0),id(id),parent_id(parent)
  {};  
	void (*fnc)(void);
	struct task* parent;	// dependency
	enum task_status status;
	unsigned long ms;	// task spend time
	// task id for dependencies
	task_id id;
	task_id parent_id;
};


class linux_init
{
	public:
	linux_init(task* begin,task* end) : begin(begin),end(end)
	{
		// resolve dependencies		
	}
	// Peek a new task from list
	task* peekTask(task* prev)
	{
		task* it = begin;
		bool towait = false;	// if true means wait for completion, false return current task or null
		std::unique_lock<std::mutex> lock(mtx);
		if (prev != nullptr)
		{
			printf("E %s\n",getTaskName(prev->id));
			prev->status = done;
			cond_var.notify_all();
		}
		do
		{	
			towait = false;
			while (it != end)
			{
				// find any ready task			
				if (it->status == waiting)
				{
					if (it->parent == nullptr || it->parent->status == done)
					{
						it->status = running;
						printf("S %s\n",getTaskName(it->id));
						return it;
					}
					towait = true;									
				}
				++it;
			}
			if (towait)
			{
				printf("W\n");
				cond_var.wait(lock);	
			}
		} while (towait);
		return nullptr;
	}
	// do not forget (char*) nullptr as last argument
	static int launch(bool wait,const char * const * argv)
	{
		int status;
		pid_t pid = fork();
		if (pid == -1)
		{
			perror("failed to fork");
			status = -1;
		}	
		else if (pid > 0)
		{		
			if (wait) 
				waitpid(pid, &status, 0);		
		}
		if (pid == 0)
		{
			// child
			execv(argv[0],(char* const*)argv);
			_exit(EXIT_FAILURE); 				
		}
		return status;
	}
  public:
    std::mutex	mtx;
    std::condition_variable cond_var;	
	task* begin,*end;
};

void threadFnc(linux_init* lnx)
{
	task* t = nullptr;
	while ((t = lnx->peekTask(t)) != nullptr)
	{
		t->fnc();		
	}
}

// mount proc
void mountproc()
{
  int r = mount("","/proc","proc",MS_NOATIME|MS_NODIRATIME|MS_NODEV|MS_NOEXEC|MS_SILENT|MS_NOSUID  ,"");
  if ( r == -1)
	  std::cout << "mount proc failed with code : "  << errno << std::endl;
  r = mount("","/sys","sysfs",MS_NOATIME|MS_NODIRATIME|MS_NODEV|MS_NOEXEC|MS_SILENT|MS_NOSUID  ,"");
}

// Mount home, var remount root
void mountfs()
{
  int r;
  r = mount("","/run","tmpfs",MS_NODEV|MS_NOEXEC|MS_SILENT|MS_NOSUID  ,"");
  r = mount("","/run/lock","tmpfs",MS_NODEV|MS_NOEXEC|MS_SILENT|MS_NOSUID  ,"");
  r = mount("","/run/shm","tmpfs",MS_NODEV|MS_NOEXEC|MS_SILENT|MS_NOSUID  ,"");
  r = mount("","/tmp","tmpfs",MS_NODEV|MS_NOEXEC|MS_SILENT|MS_NOSUID  ,"");
    
  r = mount("/dev/sda5","/home","ext4",MS_NOATIME|MS_NODIRATIME|MS_NODEV|MS_SILENT,"");
  if ( r == -1)
	  std::cout << "mount home failed with code : "  << errno << std::endl;
  r = mount("/dev/sda6","/mnt/data","ext4",MS_NOATIME|MS_NODIRATIME|MS_NODEV|MS_SILENT,"");
  if ( r == -1)
	  std::cout << "mount data failed with code : "  << errno << std::endl;	
}

void deferred()
{
	FILE * pFile;
	char buffer [10];
	pFile = fopen ("/proc/deferred_initcalls" , "r"); 
	if (pFile == NULL) perror ("Error opening file /proc/deferred_initcalls");
	else
	{
		fread(buffer,sizeof(buffer),1,pFile);
		fclose(pFile);
	}		
}

void bootchartd()
{
	const char* arg[] = {"/sbin/bootchartd","start",(char*)nullptr};
	linux_init::launch(true,arg);
}

void bootchartd_stop()
{
	const char* arg[] = {"/sbin/bootchartd","stop",(char*)nullptr};
	linux_init::launch(true,arg);
}

void udev()
{
	const char* arg[] = {"/sbin/udevadm","trigger"," --action=add",(char*)nullptr};	
}

void startx()
{
	const char* arg[] = {"/bin/su","-c","startx","lester",(char*)nullptr};
	linux_init::launch(false,arg);
	/*
	const char* arg[] = {"/usr/bin/startx",(char*)nullptr};
	pid_t pid = fork();
	if (pid == -1)
	{
		perror("failed to fork");
	}	
	else if (pid == 0)
	{
		// childs
		seteuid(1000);
		setegid(1000);
		setuid(1000);
		execv(arg[0],(char* const *)arg);
		_exit(EXIT_FAILURE); 				
	}
	*/
}

void hostname()
{
	//read etc/hostname if not empty the apply otherwise use localhost
	const char* name = "localhost";
	char buffer [100];
	int r;
	FILE * pFile = fopen ("/etc/hostname" , "r"); 
	if (pFile != NULL)
	{
		r = fread(buffer,sizeof(buffer),1,pFile);
		if (r > 0)
		{
			buffer[r] = 0;
			name = buffer;			
		}		
		fclose(pFile);
	}	
	sethostname(name,strlen(name));
}

// Wait for all task in running state
void waitall()
{
	sleep(5);	
}

/*
Execution list plus dependencies.
use id to find dependencies.
mount tmpfs /run tmpfs nosuid
mount tmpfs /run/lock tmpfs nosuid noexec nodev
mount tmpfs /run/shm tmpfs nosuid noexec nodev
mount tmpfs /tmp tmpfs nosuid nodev

run_migrate /run/shm /dev/shm

*/
int main()
{	
  task tasks[] = {  //
  {mountproc,procfs_id}, //
  {bootchartd,bootchart_id}, //
  {deferred,deferred_id,bootchart_id},//
  {mountfs,fs_id},//
  {udev,udev_id,fs_id},//
  {hostname,hostname_id},//
  {startx,x11_id,fs_id},  //
  {waitall,wait_id,x11_id},
  {bootchartd_stop,bootchart_end_id,wait_id}, //
  }; 
  /*
    task tproc{mountproc,"procfs"};
	task tbootchart {bootchartd,"bootchartd start",&tproc};
	task tdeferred {deferred,"deferred",&tbootchart};
	task tfs{mountfs,"fs",&tbootchart};
	task x11{startx,"x11",&tfs};
	task udev_end{udev,"udev trigger",&x11};
	task sleep5{waitall,"wait",&udev_end};
	task tbootchart_end {bootchartd_stop,"bootchartd stop",&sleep5};
	
	task* tasks[] = {&tbootchart,&tproc,&tfs,&tdeferred,&x11,&tbootchart_end,&sleep5,&udev_end};
	*/
	
	linux_init	lnx(tasks,tasks+sizeof(tasks)/sizeof(*tasks));
	
	std::thread t1 (threadFnc,&lnx); 
	std::thread t2 (threadFnc,&lnx); 
	std::thread t3 (threadFnc,&lnx); 
	t1.join();
	t2.join();
	t3.join();
	return 0;
}