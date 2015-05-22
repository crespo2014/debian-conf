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
#include <chrono>
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
	x(udev_trigger)\
	x(wait)\
	x(dbus)\
	
#define TO_STRING(id)                 #id
#define TO_NAME(id,...)               TO_STRING(id),
#define TO_ID(id,...)                 id ## _id,

typedef enum
{
  TASK_ID(TO_ID)
} task_id;

const char* getTaskName(task_id id)
{
  static const char* const names[] = { TASK_ID(TO_NAME)"" };
  if (id >= sizeof(names) / sizeof(*names)) return "";
  return names[id];
}

enum task_status
{
  waiting, running, done,
};



class linux_init
{
public:
  // define type to function
  typedef void (linux_init::*thread_fnc_typ)(void);
  
  // define task data container`
  class task
  {
  public:
    task(thread_fnc_typ fnc, task_id id, task_id parent = none_id) :
        fnc(fnc), parent(nullptr), status(waiting), ms(0), id(id), parent_id(parent)
    {
    }
    thread_fnc_typ fnc;
    struct task* parent;    // dependency
    enum task_status status;
    unsigned long ms;    // task spend time
    // task id for dependencies
    task_id id;
    task_id parent_id;
  };
  
  // class methods 
  linux_init() 
  {
	task tasks[] = {    //
	
    { &linux_init::mountproc, procfs_id },    //
    //{ bootchartd, bootchart_id,procfs_id },    //
    { &linux_init::hostname, hostname_id },    //
    { &linux_init::deferred, deferred_id, x11_id },    //
    { &linux_init::mountfs, fs_id,hostname_id },    //
    { &linux_init::udev, udev_id, x11_id},    //
    { &linux_init::startx, x11_id, fs_id },    //
    { &linux_init::udev_trigger,udev_trigger_id,x11_id }, //
//        { waitall, wait_id, x11_id }, //
//        { bootchartd_stop, bootchart_end_id, udev_trigger_id },    //

    };
	begin = tasks;
	end = tasks + sizeof(tasks) / sizeof(*tasks);
	// resolve dependencies
    for (auto it_c = begin; it_c != end; ++it_c)
    {
      if (it_c->parent_id != none_id)
      {
        auto it_p = begin;
        while (it_p != end && it_p->id != it_c->parent_id)
          ++it_p;
        if (it_p == end)
        {
          printf("Parent task %s not found", getTaskName(it_c->parent_id));
        } else
          it_c->parent = it_p;
      }
    }
    std::thread t1(sthread,this);
    std::thread t2(sthread,this);
    std::thread t3(sthread,this);
    t1.join();
    t2.join();
    t3.join();    
  }

  
  // Peek a new task from list
  task* peekTask(task* prev)
  {
    task* it = begin;
    bool towait = false;    // if true means wait for completion, false return current task or null
    std::unique_lock<std::mutex> lock(mtx);
    if (prev != nullptr)
    {
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
  static int launch(bool wait, const char * const * argv)
  {
    int status;
    pid_t pid = fork();
    if (pid == -1)
    {
      perror("failed to fork");
      status = -1;
    } else if (pid > 0)
    {
      if (wait) waitpid(pid, &status, 0);
    }
    if (pid == 0)
    {
      // child
      execv(argv[0], (char* const *) argv);
      _exit(EXIT_FAILURE);
    }
    return status;
  }
  // thread function
  void thread()
  {
	task* t = nullptr;
    while ((t = peekTask(t)) != nullptr)
    {
      printf("S %s\n", getTaskName(t->id));
      std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
      (this->*(t->fnc))();
      std::chrono::steady_clock::time_point end= std::chrono::steady_clock::now();
      printf("E %s %d msec \n", getTaskName(t->id),std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count());
    }	  
  }
  static void sthread(linux_init* lnx)
  {
	  return lnx->thread();
  }
  inline void testrc(int r)
  {
  //  if (r < 0)
  //    printf("Operation failed with code %d \n",errno);
  }

  // mount proc
  void mountproc()
  {
  //  testrc(mount("", "/proc", "proc", MS_NOATIME | MS_NODIRATIME | MS_NODEV | MS_NOEXEC | MS_SILENT | MS_NOSUID, ""));
    testrc(mount("", "/sys", "sysfs", MS_NOATIME | MS_NODIRATIME | MS_NODEV | MS_NOEXEC | MS_SILENT | MS_NOSUID, ""));
  }
  
  // Mount home, var remount root
  void mountfs()
  {
    testrc(mount("/dev/sda5", "/", "ext4",MS_NOATIME | MS_NODIRATIME| MS_REMOUNT| MS_SILENT , ""));
    testrc(mount("run", "/run", "tmpfs", MS_NODEV | MS_NOEXEC | MS_SILENT | MS_NOSUID, ""));
    testrc(mount("lock", "/run/lock", "tmpfs", MS_NODEV | MS_NOEXEC | MS_SILENT | MS_NOSUID, ""));
    testrc(mount("shm", "/run/shm", "tmpfs", MS_NODEV | MS_NOEXEC | MS_SILENT | MS_NOSUID, ""));
    testrc(mount("tmp", "/tmp", "tmpfs", MS_NODEV | MS_NOEXEC | MS_SILENT | MS_NOSUID, ""));
    testrc(mount("pts", "/dev/pts", "devpts", MS_SILENT |MS_NOSUID | MS_NOEXEC , "gid=5,mode=620"));
    testrc(mount("/dev/sda7", "/home", "ext4", MS_NOATIME | MS_NODIRATIME |  MS_SILENT, ""));
    testrc(mount("/dev/sda8", "/mnt/data", "ext4", MS_NOATIME | MS_NODIRATIME |  MS_SILENT, ""));
  }
  
  void deferred()
  {
    FILE * pFile;
    pFile = fopen("/proc/deferred_initcalls", "r");
    if (pFile == NULL)
      printf("Error opening file /proc/deferred_initcalls \n");
    else
    {
      fclose(pFile);
    }
  }
  
  void bootchartd()
  {
    const char* arg[] = { "/sbin/bootchartd", "start", (char*) nullptr };
    linux_init::launch(true, arg);
  }
  
  void bootchartd_stop()
  {
    const char* arg[] = { "/sbin/bootchartd", "stop", (char*) nullptr };
    linux_init::launch(true, arg);
  }
  
  void udev()
  {
    FILE * pFile;
  
    pFile = fopen("/sys/kernel/uevent_helper", "w");
    if (pFile == NULL)
    {
      printf("Error opening file /sys/kernel/uevent_helper \n");
    } else
    {
      fwrite("", 1, 1, pFile);
      fclose(pFile);
    }
    const char* arg[] = { "/sbin/udevd", (char*) nullptr };
    linux_init::launch(false, arg);
  }
  
  void udev_trigger()
  {
    const char* arg[] = { "/sbin/udevadm", "trigger", "--action=add", (char*) nullptr };
    linux_init::launch(true, arg);
  }
  
  /*
  startxfc4 script c++ translation
  */
  
  void startxfce4()
  {/*
  	std::string str;
  	const char* env = getenv("XDG_CONFIG_HOME");	
  	const char* app = "xfce4";
  	if (env == nullptr)
  	{
  		env = "$HOME/.config"
  	}
  	str.append(env);
  	str.append("xfce4");
  	setenv("BASEDIR");
  */
  }
  
  void startx()
  {
  	/*
    // Prepare environment to run X server xinit, required files ~/.xinitrc ~/xserverrc 
    lester   31477 27677  0 11:06 pts/0    00:00:00 /bin/sh /usr/bin/startx
    lester   31494 31477  0 11:06 pts/0    00:00:00 xinit /etc/X11/xinit/xinitrc -- /etc/X11/xinit/xserverrc :0 -auth /tmp/serverauth.826flacMFH
    root     31495 31494  5 11:06 tty2     00:00:00 /usr/bin/X -nolisten tcp :0 -auth /tmp/serverauth.826flacMFH
    
    TODO:
    set environment
    prepare auth file
    start X with arguments (no wait)
    start xfce4 ( no wait )	// su -l -c startx-xfc lester
  */
  	const char* env;
    constexpr const char* X = "/usr/bin/X";
    constexpr const char* X_arg = "-nolisten tcp :0 vt7 ";
    constexpr const char* Xclient ="/usr/bin/xfce4";
    constexpr const char* home = "/home/lester";
    char srv_auth_file[]="serverauth.XXXXXX";
   
  	unsetenv("DBUS_SESSION_BUS_ADDRESS");
  	unsetenv("SESSION_MANAGER");
    std::string str;
    str = home;
    str.append("/.Xauthority");
    setenv("XAUTHORITY",str.c_str(),true);
    mktemp(srv_auth_file);
    
  
    	
    //setenv
    //unset
    //putenv
    //getenv
    
    const char* arg[] = { "/bin/su","-l", "-c", "startx", "lester", (char*) nullptr };
    linux_init::launch(false, arg);
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
    char buffer[100];
    int r;
    FILE * pFile = fopen("/etc/hostname", "r");
    if (pFile != NULL)
    {
      r = fread(buffer,1, sizeof(buffer), pFile);
      if (r > 0)
      {
        buffer[r] = 0;
        name = buffer;
      }
      fclose(pFile);
    }
    sethostname(name, strlen(name));
    printf("Host:%s",name);
  }
  
  // Wait for all task in running state
  void waitall()
  {
    sleep(5);
  }  
public:
  std::mutex mtx;
  std::condition_variable cond_var;
  task* begin, *end;
};

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
  linux_init lnx;
  return 0;
}
