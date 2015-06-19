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

#include <vector>

#include <sys_linux.h>

#include <stdlib.h>
#include <stdio.h>

#include <string.h>

#include <preload.h>

#define TASK_ID(x)	\
	x(none)\
  x(acpi)\
	x(hostname) \
	x(deferred) \
  x(e4rat) \
	x(fs)\
	x(bootchartd)\
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
	x(dbus)\
	x(procps)\
	x(xfce4)\
	x(init_d)\
	x(readahead)\
	x(late_readahead)\
	x(mountall) \
	x(preload)\
	x(max)\
	

#define TO_STRING(id)                 #id
#define TO_NAME(id)               TO_STRING(id),
#define TO_ID(id)                 id ## _id,

#define DEPENDS_0()
#define DEPENDS_1(a)
#define DEPENDS_2(a,b)      MOD_DEPENDENCY_ITEM(a,b),
#define DEPENDS_3(a,b,...)  DEPENDS_2(a,b) DEPENDS_2(a,__VA_ARGS__)
#define DEPENDS_4(a,b,...)  DEPENDS_2(a,b) DEPENDS_3(a,__VA_ARGS__)
#define DEPENDS_5(a,b,...)  DEPENDS_2(a,b) DEPENDS_4(a,__VA_ARGS__)
#define DEPENDS_6(a,b,...)  DEPENDS_2(a,b) DEPENDS_5(a,__VA_ARGS__)
#define DEPENDS_7(a,b,...)  DEPENDS_2(a,b) DEPENDS_6(a,__VA_ARGS__)
#define DEPENDS_8(a,b,...)  DEPENDS_2(a,b) DEPENDS_7(a,__VA_ARGS__)
#define DEPENDS_9(a,b,...)  DEPENDS_2(a,b) DEPENDS_8(a,__VA_ARGS__)
#define DEPENDS_10(a,b,...) DEPENDS_2(a,b) DEPENDS_9(a,__VA_ARGS__)

#define TASK_INFO_2(fnc,id)                 { fnc, id ## _id, none_id, none_id },
#define TASK_INFO_3(fnc,id,parent1)         { fnc, id ## _id, parent1 ## _id, none_id } ,
#define TASK_INFO_4(fnc,id,parent1,parent2) { fnc, id ## _id, parent1 ## _id, parent2 ## _id } ,

#define GET_10(fnc,n0,n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,...) fnc##n10
#define COUNT(fnc,...) GET_10(fnc,__VA_ARGS__,10,9,8,7,6,5,4,3,2,1)
#define CALL_FNC(fnc,...) COUNT(fnc,__VA_ARGS__)(__VA_ARGS__)

#define DEPENDS_BUILD(id,type,...) CALL_FNC(DEPENDS_,id,##__VA_ARGS__)

#define GET_DEPENDS_0()  none,none
#define GET_DEPENDS_1()  none,none

// Fill a table with all modules information
#define TASK_INFO(...)  CALL_FNC(TASK_INFO_,##__VA_ARGS__)

#define TASK_DATA(...)

typedef enum
{
  TASK_ID(TO_ID)
} task_id;

enum task_status
{
  disable = 0,    //
  waiting,    //
  running,    //
  done,
};

/*
 * This is the dynamic data of the task
 */
struct task_status_t
{
  unsigned child_count;
  enum task_status status;
  unsigned long ms;    // task spend time
};

//void print_error(int r, int no)
//{
//
//}

#define EXIT(fnc,cnd) do \
  { \
    int r = fnc;  \
    if (r cnd) \
    { \
      printf("operation %s failed errno %d",TO_STRING(fnc),errno); \
      exit(-1); \
    } \
  } while (0)

#define CHECK_ZERO(fnc,msg)   if (fnc != 0) perror(msg);

static const char* getTaskName(task_id id)
{
  static const char* const names[] = { TASK_ID(TO_NAME)"" };
  if (id >= sizeof(names) / sizeof(*names))
    return "";
  return names[id];
}

static char srv_auth_file[] = "/tmp/.server.auth.XXXXXX";
static char usr_auth_file[] = "/tmp/.user.auth.XXXXXX";
static char mcookie[40];

class linux_init
{
public:
  // define type to function
  typedef void (linux_init::*thread_fnc_typ)(void);
  struct task_t
  {
    thread_fnc_typ fnc;
    task_id id;
  };

  // Using a class make brakes intialization easy
  struct task_info_t
  {
    thread_fnc_typ fnc;
    task_id id;
    // id of dependencies
    task_id parent_id;         // = none_id;
    task_id parent_id2;        // = none_id;
  };
  // Mount point details
  struct mount_point_t
  {
    const char* device;
    const char* path;
    const char* type;
    unsigned long int flags;
    const void* data;
    const char* err_msg;
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
    bool fast = false;
    std::vector<char*> cmdline;
    cmdline.reserve(15);
    *tstr = 0;
    auto fd = open("/proc/cmdline", O_RDONLY);
    if (fd > 0)
    {
      int r = read(fd, tstr, sizeof(tstr) - 1);
      if (r > 0)
        tstr[r - 1] = 0;     // remove ending \n
      close(fd);
      SysLinux::split(tstr, cmdline);
      for (auto p : cmdline)
      {
        if (strcmp(p, "cinit") == 0)
        {
          fast = true;
        } else if (strcmp(p, "single") == 0)
        {
          fast = false;
          break;
        }

      }
    } else
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

    memset(status, 0, sizeof(status));

    // update child counter
    for (const task_info_t* it = begin; it != end; ++it)
    {
      status[it->id].status = waiting;
      if (it->parent_id != none_id)
      {
        ++status[it->parent_id].child_count;
        if (it->parent_id2 != none_id)
          ++status[it->parent_id2].child_count;
      }
    }
    status[none_id].status = done;

    std::thread t1(sthread, this);
    std::thread t2(sthread, this);
    //std::thread t3(sthread, this);
    t1.join();
    t2.join();
    //t3.join();
    return 0;
  }

  // class methods
  linux_init(const task_info_t* begin, const task_info_t* end) :
      begin(begin), end(end)
  {

  }

  // Peek a new task from list , mark the incoming task as done
  const task_info_t* peekTask(const task_info_t* it)
  {
    bool towait;    // if true means wait for completion, false return current task or null
    bool notify = false;
    std::unique_lock<std::mutex> lock(mtx);
    if (it != nullptr)
    {
      status[it->id].status = done;
      // put back all done task
      if (it == begin)
      {
        while (begin != end && status[begin->id].status == done)
          ++begin;
      }
      if (status[it->id].child_count > 1)
        notify = true;    // more than one task has been release
    }
    for (;;)
    {
      towait = false;    // no task to waiting for
      for (it = begin; it != end; ++it)
      {
        // find any ready task
        if (status[it->id].status == waiting)
        {
          if (status[it->parent_id].status == done && status[it->parent_id2].status == done)
          {
            status[it->id].status = running;
            break;
          }
          towait = true;
        }
      }
      if (it != end)
        break;     // get out for ;;
      // we got nothing
      if (towait)    // wait and try again
      {
        cond_var.wait(lock);
      } else
      {
        it = nullptr;    // we done here
        notify = true;
        break;    // get out for ;;
      }
    }    // for ;; loop
    if (notify)
      cond_var.notify_all();    // more than one task has been release
    return it;
  }

  // thread function
  void thread()
  {
    //char tstr[255];
    const task_info_t* t = nullptr;
    for (t = peekTask(t); t != nullptr; t = peekTask(t))
    {
      auto end = std::chrono::steady_clock::now();
      //snprintf(tmp_str, sizeof(tmp_str) - 1,"[%d] S %s\n",std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time).count(), getTaskName(t->id));
      std::cout
      //<< 'T' << std::this_thread::get_id()
      << " [" << std::chrono::duration_cast<std::chrono::milliseconds>(end - start_time).count() << "]" << " S " << getTaskName(t->id) << std::endl;
      (this->*(t->fnc))();
      std::cout << '[' << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time).count() << "] E "
          << getTaskName(t->id) << " " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - end).count()
          << "ms" << std::endl;
    }
  }
  static void sthread(linux_init* lnx)
  {
    return lnx->thread();
  }
  inline void testrc(int)
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
    SysLinux::execute_c("/etc/init.d/early-readahead start");
  }

  void late_readahead()
  {
    SysLinux::execute_c("/etc/init.d/later-readahead start");
  }

  // Mount home, var remount root
  void mountfs()
  {

    static const struct mount_point_t mounts[] = {    //
        { "/dev/sda5", "/", "ext4", MS_NOATIME | MS_NODIRATIME | MS_REMOUNT | MS_SILENT, "", "remount /" },    //
            { "sys", "/sys", "sysfs", MS_NOATIME | MS_NODIRATIME | MS_NODEV | MS_NOEXEC | MS_SILENT | MS_NOSUID, "", "mount sys" },    //
            { "run", "/run", "tmpfs", MS_NODEV | MS_NOEXEC | MS_SILENT | MS_NOSUID, "", "mount /run " },    //
            { "dev", "/dev", "devtmpfs", MS_SILENT, "", "mount dev" },    //
            { "tmp", "/tmp", "tmpfs", MS_NODEV | MS_NOEXEC | MS_SILENT | MS_NOSUID, "", "mount /tmp" },    //
            { "/dev/sda7", "/home", "ext4", MS_NOATIME | MS_NODIRATIME | MS_SILENT, "", "mount /home" },    //
            { "/dev/sda8", "/mnt/data", "ext4", MS_NOATIME | MS_NODIRATIME | MS_SILENT, "", "mount /mnt/data" },    //
        };

    for (const auto &mnt : mounts)
    {
      CHECK_ZERO(mount(mnt.device, mnt.path, mnt.type, mnt.flags, mnt.data), mnt.err_msg);
    }
    CHECK_ZERO(mkdir("/run/lock", 01777), "mkdir /run/lock");
    CHECK_ZERO(mkdir("/run/shm", 01777), "mkdir /run/shm");
    CHECK_ZERO(chmod("/run/shm", 01777), "chmod /run/shm");
    CHECK_ZERO(chmod("/run/lock", 01777), "chmod /run/lock");
    CHECK_ZERO(symlink("/run", "/var/run"), "symlink /run /var/run");
    CHECK_ZERO(symlink("/run/lock", "/var/lock"), "symlink /run/lock /var/lock");
    CHECK_ZERO(symlink("/run/shm", "/dev/shm"), "symlink /run/shm /dev/shm");

    CHECK_ZERO(mkdir("/dev/pts", 0755), "mkdir /dev/pts");
    CHECK_ZERO(mount("pts", "/dev/pts", "devpts", MS_SILENT | MS_NOSUID | MS_NOEXEC, "gid=5,mode=620"), "mount pts");
    /*
     https://wiki.debian.org/ReleaseGoals/RunDirectory
     Stage #2: After system reboot

     A tmpfs is mounted on /run
     (Optional) A tmpfs is mounted on /run/lock if RAMLOCK is configured
     (Optional) A tmpfs is mounted on /run/shm if RAMSHM is configured
     (Optional) A tmpfs is mounted on /tmp if RAMTMP is configured
     A symlink /var/run /run is created (falls back to bind mount if symlink failed)
     A symlink /var/lock /run/lock is created (falls back to bind mount if symlink failed)
     A symlink /dev/shm  /run/shm is created (falls back to bind mount if symlink failed)
     */
  }

//  void mountall()
//  {
//
//  }
//
//  void mountdevsubfs()
//  {
//
//  }

  void procps()
  {
    SysLinux::execute_c("/sbin/sysctl -q --system");
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

  /*
   * Depends on procfs
   */
  void udev()
  {
    //char tstr[255];
    struct stat buf;

    // When udev starts, your real /dev is bind mounted to /.dev

    // If kernel boot with udev mounted and .udev folder then move to run before mount udev
    /* strcpy(tstr,"/bin/mv /dev/.udev/ /run/udev/");
     execute(tstr,true);
     */

    if (stat("/sbin/MAKEDEV", &buf) == 0)
    {
      symlink("/dev/MAKEDEV", "/sbin/MAKEDEV");
    } else
    {
      symlink("/dev/MAKEDEV", "/bin/true");
    }

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

    SysLinux::execute_c("udevadm info --cleanup-db");
    SysLinux::execute_c("/sbin/udevd --daemon");
    //SysLinux::execute_c("/bin/udevadm trigger --action=add");
    SysLinux::execute_c("/bin/udevadm settle", true);
  }

  void udev_trigger()
  {
    SysLinux::execute_c("/sbin/udevadm trigger");
  }

  // do not execute
  void udev_finish()
  {
    SysLinux::execute_c("/lib/udev/udev-finish");
  }

  // execute some init script
  void init_d()
  {
    SysLinux::execute_c("/etc/init.d/hwclock start", true);
    SysLinux::execute_c("/etc/init.d/urandom start", true);
    SysLinux::execute_c("/etc/init.d/networking start");
  }

  /*
   startxfc4 script c++ translation
   */

  void startxfce4()
  {
    char tmp_str[255];
    snprintf(tmp_str, sizeof(tmp_str) - 1, "/bin/su -l -c 'export %s=%s;export %s=:%d;exec /usr/bin/startxfce4' lester", env_authority, usr_auth_file,
        env_display, x_display_id);
    SysLinux::execute(tmp_str, false);
  }

  void startXserver()
  {
    char tmp_str[255];
    //const char* env;
    char* cptr;
    int r;
    mkdir("/tmp/.X11-unix", 01777);
    chmod("/tmp/.X11-unix", 01777);
    mkdir("/tmp/.ICE-unix", 01777);
    chmod("/tmp/.ICE-unix", 01777);
    unsetenv(env_dbus_session);
    unsetenv(env_session_manager);

    int auth_file_fd = mkstemp(srv_auth_file);    // create file	file has to be delete when everything is done, but for just one x server keep it in tmp is ok
    if (auth_file_fd != -1)
    {
      close(auth_file_fd);
    }
    // call xauth to add display 0 and cookie add :0 . xxxxxx
    auto fd = popen("/usr/bin/mcookie", "r");
    r = fread(mcookie, 1, sizeof(mcookie) - 1, fd);
    if (r > 0)
      mcookie[r] = 0;
    pclose(fd);

    // Server auth file
    snprintf(tmp_str, sizeof(tmp_str) - 1, "/usr/bin/xauth -q -f %s add :%d . %s", srv_auth_file, x_display_id, mcookie);
    SysLinux::execute(tmp_str);

    // Client auth file
    auth_file_fd = mkstemp(usr_auth_file);
    if (auth_file_fd != -1)
    {
      close(auth_file_fd);
    }

    snprintf(tmp_str, sizeof(tmp_str) - 1, "/usr/bin/xauth -q -f %s add :%d . %s", usr_auth_file, x_display_id, mcookie);
    SysLinux::execute(tmp_str);
    EXIT(chown(usr_auth_file, 1000, 1000), == -1);    // change owner to main user

    // Start X server and wait for it
    sigset_t sig_mask;
    sigset_t oldmask;

    sigemptyset(&sig_mask);
    sigaddset(&sig_mask, SIGUSR1);
    pthread_sigmask(SIG_BLOCK, &sig_mask, &oldmask);
    struct timespec sig_timeout = { 15, 0 };    // 5sec

    snprintf(tmp_str, sizeof(tmp_str) - 1, "/etc/X11/xorg.conf.%s", host);
    r = access(tmp_str, F_OK);

    //-terminate -quiet
    cptr = tmp_str;
    cptr += snprintf(cptr, tmp_str + sizeof(tmp_str) - cptr - 1, "/usr/bin/X :%d ", x_display_id);
    if (r == 0)
      cptr += snprintf(cptr, tmp_str + sizeof(tmp_str) - cptr - 1, " -config /etc/X11/xorg.conf.%s", host);

    snprintf(cptr, tmp_str + sizeof(tmp_str) - cptr - 1, "  -audit 0 -logfile /dev/kmsg -nolisten tcp -auth %s vt0%d", srv_auth_file, x_vt_id);

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
      SysLinux::execute(tmp_str, false, true);
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
    int r;
    FILE * pFile = fopen("/etc/hostname", "r");
    if (pFile != NULL)
    {
      r = fread(host, 1, sizeof(host), pFile);
      if (r > 0)
      {
        host[r - 1] = 0;
      } else
        strcpy(host, "localhost");
      fclose(pFile);
    } else
      perror("/etc/hostname ");
    sethostname(host, strlen(host));
  }

  void e4rat_load()
  {
    SysLinux::execute_c("/sbin/e4rat-preload /var/lib/e4rat/startup.log", true);
  }
  void acpi_daemon()
  {
    SysLinux::execute_c("/etc/init.d/acpid start");
  }
  /*
   * Test time needed to parse preload file
   */
  void preload()
  {
    preload_parser p;
    p.loadFile("/var/lib/e4rat/startup.log");
    p.preload();
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
  const task_info_t* begin = nullptr, * const end = nullptr;
  // status of all tasks
  struct task_status_t status[task_id::max_id];
  char host[100];    // Host name
};

/*
 Execution list plus dependencies.
 */
int main()
{
  // static initialization of struct is faster than using object, the compiler will store a table and just copy over
  // using const all data will be in RO memory really fast
  static const linux_init::task_info_t tasks[] = {
//      TASK_INFO( &linux_init::preload, preload)    //
      TASK_INFO( &linux_init::mountfs, fs)    //
      //TASK_INFO( &linux_init::e4rat_load, e4rat,preload)    //
      TASK_INFO( &linux_init::hostname, hostname)    //
      // TASK_INFO( &linux_init::deferred, deferred,init_d)    //
      TASK_INFO( &linux_init::udev, udev, hostname,fs )    //
      //TASK_INFO( &linux_init::mountall,mountall,fs, udev)    //
      //TASK_INFO( &linux_init::mountdevsubfs, dev_subfs, udev )    //
      TASK_INFO( &linux_init::procps, procps )    //
      //TASK_INFO( &linux_init::udev_trigger, udev_trigger,udev)    //
      //TASK_INFO( &linux_init::acpi_daemon, acpi,e4rat,mountall)    //
      //TASK_INFO( &linux_init::startXserver, X, hostname,acpi)    //
      //TASK_INFO( &linux_init::startxfce4, xfce4, X )     //
      TASK_INFO( &linux_init::init_d, init_d, procps )    //
      };
  linux_init lnx(tasks, tasks + sizeof(tasks) / sizeof(*tasks));
  return lnx.main();
}
