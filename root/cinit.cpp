/*
 Small c++ application to initialize the system
 Running N threads using a simple dependency list
 it tend to be faster than alternative using bash interpreter
 g++ -std=c++11 -g -lpthread cinit.cpp -o cinit
 apt-get install inotify-tools

 SIGUSR1
 This signal is used quite differently from either of the above. When the server starts, it checks to see if it has inherited SIGUSR1 as SIG_IGN instead of the usual SIG_DFL. In this case, the server sends a SIGUSR1 to its parent process after it has set up the various connection schemes. Xdm uses this feature to recognize when connecting to the server is possible.

 */
#include <mutex>
#include <iostream>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <chrono>
#include <cstring>
#include <vector>
#include <sys/mount.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/inotify.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

#define TASK_ID(x)	\
	x(none)\
	x(hostname) \
	x(deferred) \
	x(fs)\
	x(bootchart)\
	x(bootchart_end)\
	x(read_ahead)\
	x(devfs)\
	x(udev)\
	x(x11)\
	x(X)\
	x(dev_subfs) \
	x(udev_add)\
	x(udev_mtab)\
	x(udev_trigger)\
	x(wait)\
	x(dbus)\
	x(xfce4)\
	x(init_d)\
	x(readahead)\
	x(late_readahead)\
	

#define TO_STRING(id)                 #id
#define TO_NAME(id,...)               TO_STRING(id),
#define TO_ID(id,...)                 id ## _id,

void print_error(int r, int no)
{

}

#define EXIT(fnc,cnd) do \
  { \
    int r = fnc;  \
    if (r cnd) \
    { \
      printf("operation %s failed errno %d",TO_STRING(fnc),errno); \
      exit(-1); \
    } \
  } while (0)

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

static char srv_auth_file[] = "/tmp/.server.auth.XXXXXX";
static char usr_auth_file[] = "/tmp/.user.auth.XXXXXX";
static char mcookie[40];


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

  /*
   * Main fucntion as main entry point
   * mount proc and keep it, the application is enable only if fastboot kernel argument is supplied
   * single will disable fastboot
   */
  int main()
  {
    char tstr[255];
    testrc(mount("", "/proc", "proc", MS_NOATIME | MS_NODIRATIME | MS_NODEV | MS_NOEXEC | MS_SILENT | MS_NOSUID, ""));
    // mount proc check for single and exit
    bool fast =  false;   
    std::vector<char*> cmdline;
    cmdline.reserve(15);
    *tstr = 0;
    auto fd = open("/proc/cmdline",O_RDONLY);
    if (fd > 0 )
    {
      int r = read(fd,tstr,sizeof(tstr) - 1);
      if (r > 0) tstr[r-1] = 0;     // remove ending \n
      close(fd);
      split(tstr,cmdline);
      for (auto p : cmdline)
      {
        if (strcmp(p,"fastboot") == 0 )
        {
          fast = true;
        } else if (strcmp(p,"single") == 0 )
        {
          fast = false;
          break;
        }
  
      }
    }
    else
    {
      printf("failed to open /proc/cmdline");
    }
    if (!fast)
    {
      printf("Fastboot aborted\n");
      return -1;
    }
    // block SIGUSR1 on main thread
    sigset_t sig_mask;
    sigset_t oldmask;

    sigemptyset(&sig_mask);
    sigaddset(&sig_mask, SIGUSR1);
    sigprocmask(SIG_BLOCK, &sig_mask, &oldmask);
    //signal(SIGUSR1,SIG_IGN);
    
    task tasks[] = {
        { &linux_init::hostname, hostname_id },    //
        { &linux_init::mountfs, fs_id,hostname_id },    //
        { &linux_init::startXserver, X_id, fs_id },    //
        { &linux_init::startxfce4, xfce4_id, X_id },    //
        { &linux_init::deferred, deferred_id,fs_id },    //
        { &linux_init::udev, udev_id, deferred_id },    //
        { &linux_init::mountdevsubfs, dev_subfs_id, udev_id },    //
        { &linux_init::procps, dev_subfs_id, udev_id },    //
        { &linux_init::udev_trigger, udev_trigger_id, udev_id },    //
        { &linux_init::init_d, init_d_id, udev_trigger_id },    //
        //{ &linux_init::late_readahead,late_readahead_id,X_id },
        // { &linux_init::readahead, readahead_id,fs_id },    //
        //{ bootchartd, bootchart_id,procfs_id },    //
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
    std::thread t1(sthread, this);
    std::thread t2(sthread, this);
    std::thread t3(sthread, this);
    t1.join();
    t2.join();
    t3.join();
    return 0;
  }

  // class methods
  linux_init()
  {

  }
  /*
   * Split element of string delimiter by spaces
   */
  void split(char* list,std::vector<char*>& v)
  {
    char *cptr = list;
    do
    {
      // find letter
      while (*cptr == ' ' && *cptr != 0)
        ++cptr;
      if (*cptr == '\'')    // argument with delimiters
      {
        ++cptr;
        v.push_back(cptr);
        while (*cptr != '\'' && *cptr != 0)
          ++cptr;
        if (*cptr == '\'')
        {
          *cptr = 0;
          ++cptr;
        }
      } else if (*cptr != 0)
      {
        v.push_back(cptr);
        //next space
        while (*cptr != ' ' && *cptr != 0)
          ++cptr;
        if (*cptr == ' ')
        {
          *cptr = 0;
          ++cptr;
        }
      }

    } while (*cptr != 0);
  }

  // Peek a new task from list
  task* peekTask(task* prev)
  {
    bool towait = false;    // if true means wait for completion, false return current task or null
    std::unique_lock<std::mutex> lock(mtx);
    if (prev != nullptr)
    {
      prev->status = done;
      cond_var.notify_all();
    }
    do
    {
      task* it = begin;
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
	//std::cout << 'T' << std::this_thread::get_id() << " W" << std::endl;
        cond_var.wait(lock);
      }
    } while (towait);
    return nullptr;
  }
  /*
   * Execute a command received as list of space separate parameters
   * wait for command to finish
   * no fork the current process
   */
  int execute(char* cmd, bool wait = true, bool nofork = false)
  {
    std::vector<char*> arg;
    arg.reserve(10);
    split(cmd,arg);
    arg.push_back(nullptr);
    return launch(wait, arg.data(), nofork);
  }
  // do not forget (char*) nullptr as last argument
  int launch(bool wait, const char * const * argv, bool nofork = false)
  {
    if (nofork)
    {
      execv(argv[0], (char* const *) argv);
      _exit(EXIT_FAILURE);
    }
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
    char tstr[255];
    task* t = nullptr;
    while ((t = peekTask(t)) != nullptr)
    {
      auto end = std::chrono::steady_clock::now();
      //snprintf(tmp_str, sizeof(tmp_str) - 1,"[%d] S %s\n",std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time).count(), getTaskName(t->id));
      std::cout 
         //<< 'T' << std::this_thread::get_id() 
         << " [" << std::chrono::duration_cast<std::chrono::milliseconds>(end - start_time).count() << "]" 
         << " S " << getTaskName(t->id) << std::endl;
      (this->*(t->fnc))();
      std::cout << '[' 
        << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time).count() 
        << "] E " << getTaskName(t->id) << " " 
        << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - end).count() << "ms" <<std::endl;
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

  void umountproc()
  {
    umount("/proc");
  }

  void readahead()
  {
    static char tstr[255];
    strcpy(tstr,"/etc/init.d/early-readahead start");
    execute(tstr,true);
  }

  void late_readahead()
  {
    static char tstr[255];
    strcpy(tstr,"/etc/init.d/later-readahead start");
    execute(tstr,true);
  }

  // Mount home, var remount root
  void mountfs()
  {
    testrc(mount("", "/sys", "sysfs", MS_NOATIME | MS_NODIRATIME | MS_NODEV | MS_NOEXEC | MS_SILENT | MS_NOSUID, ""));
    testrc(mount("/dev/sda5", "/", "ext4", MS_NOATIME | MS_NODIRATIME | MS_REMOUNT | MS_SILENT, ""));
    testrc(mount("run", "/run", "tmpfs", MS_NODEV | MS_NOEXEC | MS_SILENT | MS_NOSUID, ""));
    testrc(mount("lock", "/run/lock", "tmpfs", MS_NODEV | MS_NOEXEC | MS_SILENT | MS_NOSUID, ""));
    testrc(mount("shm", "/run/shm", "tmpfs", MS_NODEV | MS_NOEXEC | MS_SILENT | MS_NOSUID, ""));
    testrc(mount("tmp", "/tmp", "tmpfs", MS_NODEV | MS_NOEXEC | MS_SILENT | MS_NOSUID, ""));
    testrc(mount("/dev/sda7", "/home", "ext4", MS_NOATIME | MS_NODIRATIME | MS_SILENT, ""));
    testrc(mount("/dev/sda8", "/mnt/data", "ext4", MS_NOATIME | MS_NODIRATIME | MS_SILENT, ""));
  }

  void mountdevsubfs()
  {
    mkdir("/dev/pts",0755);
    testrc(mount("pts", "/dev/pts", "devpts", MS_SILENT | MS_NOSUID | MS_NOEXEC, "gid=5,mode=620"));
  }

  void procps()
  {
    char tstr[255];
    strcpy(tstr,"/sbin/sysctl -q --system");
    execute(tstr,true);
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
    char tstr[255];
    struct stat buf;
    if (stat("/sbin/MAKEDEV",&buf) == 0)
    {
      symlink("/dev/MAKEDEV","/sbin/MAKEDEV");
    }else
    {
      symlink("/dev/MAKEDEV","/bin/true");
    }
   /* strcpy(tstr,"/bin/mv /dev/.udev/ /run/udev/");
    execute(tstr,true);
*/


    FILE * pFile;

    pFile = fopen("/sys/kernel/uevent_helper", "w");
    if (pFile == NULL)
    {
      printf("Error opening file /sys/kernel/uevent_helper \n");
    } else
    {
      fwrite("", 0, 0, pFile);
      fclose(pFile);
    }
    strcpy(tstr,"udevadm info --cleanup-db");
    execute(tstr,true);
    strcpy(tstr,"/sbin/udevd --daemon");
    execute(tstr,true);
    strcpy(tstr,"/bin/udevadm trigger --action=add");
    execute(tstr,true);
    // do not wai
    //execute("/bin/udevadm settle",true);
  }

  void udev_trigger()
  {
    const char* arg[] = { "/sbin/udevadm", "trigger", "--action=add", (char*) nullptr };
    linux_init::launch(true, arg);
  }

  // do not execute
  void udev_finish()
  {
    const char* arg[] = {"/lib/udev/udev-finish",nullptr};
    launch(true,arg);
  }

  // execute some init script
  void init_d()
  {
    char tstr[255];
    strcpy(tstr,"/etc/init.d/hwclock start");
    execute(tstr,true);
    strcpy(tstr,"/etc/init.d/urandom start");
    execute(tstr,true);
    strcpy(tstr,"/etc/init.d/networking start");
    execute(tstr,true);
  }


  /*
   startxfc4 script c++ translation
   */

  void startxfce4()
  {
    char tmp_str[255];
    snprintf(tmp_str, sizeof(tmp_str) - 1,
        "/bin/su -l -c 'export %s=%s;export %s=:%d;exec /usr/bin/startxfce4' lester", env_authority,
        usr_auth_file, env_display, x_display_id);
    execute(tmp_str, false);
  }

  void startXserver()
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
    char tmp_str[255];
    const char* env;
    int r;

    mkdir("/tmp/.X11-unix",01777);
    chmod("/tmp/.X11-unix",01777);
    mkdir("/tmp/.ICE-unix",01777);
    chmod("/tmp/.ICE-unix",01777);
    unsetenv(env_dbus_session);
    unsetenv(env_session_manager);

    int auth_file_fd = mkstemp(srv_auth_file);		// create file	file has to be delete when everything is done, but for just one x server keep it in tmp is ok
    if (auth_file_fd != -1)
    {
      close(auth_file_fd);
    }
    // call xauth to add display 0 and cookie add :0 . xxxxxx
    auto fd = popen("/usr/bin/mcookie", "r");
    r = fread(mcookie, 1, sizeof(mcookie) - 1, fd);
    if (r > 0) mcookie[r] = 0;
    pclose(fd);

    // Server auth file
    snprintf(tmp_str, sizeof(tmp_str) - 1, "/usr/bin/xauth -q -f %s add :%d . %s", srv_auth_file, x_display_id, mcookie);
    execute(tmp_str);

    // Client auth file
    auth_file_fd = mkstemp(usr_auth_file);
    if (auth_file_fd != -1)
    {
      close(auth_file_fd);
    }

    snprintf(tmp_str, sizeof(tmp_str) - 1, "/usr/bin/xauth -q -f %s add :%d . %s", usr_auth_file, x_display_id, mcookie);
    execute(tmp_str);
    EXIT(chown(usr_auth_file, 1000, 1000), == -1);    // change owner to main user

    // Start X server and wait for it
    sigset_t sig_mask;
    sigset_t oldmask;

    sigemptyset(&sig_mask);
    sigaddset(&sig_mask, SIGUSR1);
    pthread_sigmask(SIG_BLOCK, &sig_mask, &oldmask);
    struct timespec sig_timeout = { 10, 0 };    // 5sec

    /* start x server and wait for signal */
    auto pid = fork();
    if (pid == 0)
    {
      // child
      /*
       * reset signal mask and set the X server sigchld to SIG_IGN, that's the
       * magic to make X send the parent the signal.
       */
      //sigprocmask(SIG_SETMASK, &oldmask, nullptr);
      signal(SIGUSR1, SIG_IGN);

      //-terminate
      snprintf(tmp_str, sizeof(tmp_str) - 1, "/usr/bin/X :%d  -audit 0 -quiet -nolisten tcp -auth %s vt0%d", x_display_id, srv_auth_file, x_vt_id);
      execute(tmp_str, false, true);
      exit(EXIT_FAILURE);
    }
    // wait for signal become pending, only blocked signal can be pending, otherwise the signal will be generated
    r = sigtimedwait(&sig_mask, nullptr, &sig_timeout);
    if (r != SIGUSR1)
    {
      printf("Error waiting for X server");
    }
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
      r = fread(buffer, 1, sizeof(buffer), pFile);
      if (r > 0)
      {
        buffer[r] = 0;
        name = buffer;
      }
      fclose(pFile);
    }
    sethostname(name, strlen(name));
    printf("Host:%s", name);
  }

  // Wait for all task in running state
  void waitall()
  {
    sleep(5);
  }
public:
  constexpr static const char* user_name = "lester";
  constexpr static const char* xauth = "/usr/bin/xauth";
  constexpr static const char* X = "/usr/bin/X";

  constexpr static const char* Xclient = "/usr/bin/xfce4";
  constexpr static const char* home = "/home/lester";
  constexpr static const char* env_mcookie = "init_mcookie";
  constexpr static const char* env_dbus_session = "DBUS_SESSION_BUS_ADDRESS";
  constexpr static const char* env_session_manager = "SESSION_MANAGER";
  constexpr static const char* env_authority = "XAUTHORITY";
  constexpr static const char* env_display = "DISPLAY";
  constexpr static const unsigned x_display_id = 1;
  constexpr static const unsigned x_vt_id = 8;

  const std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();
  std::mutex mtx;
  std::condition_variable cond_var;
  task* begin = nullptr, *end = nullptr;
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
  return lnx.main();
}
