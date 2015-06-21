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
#include <time.h>
#include <sys_linux.h>

#include <stdlib.h>
#include <stdio.h>

#include <string.h>

#include <preload.h>

#define TASK_ID(x)	\
	x(none)\
	x(root_fs) \
	x(sys_fs) \
	x(dev_fs)  \
	x(run_fs) \
	x(tmp_fs) \
	x(all_fs) \
  x(acpi)\
	x(hostname) \
	x(deferred) \
	x(udev)\
	x(X)\
	x(dev_subfs) \
	x(udev_trigger)\
	x(dbus)\
	x(procps)\
	x(xfce4)\
	x(init_d)\
	x(grp_none)  /* no group */ \
	x(grp_krn_fs) /* proc sys dev tmp run + setup directories */ \
  x(grp_fs)     /* all fs ready home, data and ... */ \
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

#define TASK_INFO_2(fnc,id)                        { fnc, id ## _id, grp_none_id, none_id, none_id },
#define TASK_INFO_3(fnc,id,grp_id)                 { fnc, id ## _id, grp_ ## grp_id ## _id, none_id, none_id },
#define TASK_INFO_4(fnc,id,grp_id,parent1)         { fnc, id ## _id, grp_ ## grp_id ## _id, parent1 ## _id, none_id } ,
#define TASK_INFO_5(fnc,id,grp_id,parent1,parent2) { fnc, id ## _id, grp_ ## grp_id ## _id, parent1 ## _id, parent2 ## _id } ,

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
  unsigned grp_ref;
  unsigned child_count;
  enum task_status status;
  struct timespec started;
  struct timespec ended;    // task spend time
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
  typedef void (*thread_fnc_typ)(void*);
  struct task_t
  {
    thread_fnc_typ fnc;
    task_id id;
  };

  struct task_info_t
  {
    thread_fnc_typ fnc;
    task_id id;
    task_id grp_id;
    // id of dependencies
    task_id parent_id;         // = none_id;
    task_id parent_id2;        // = none_id;
  };
  /*
   * Main fucntion as main entry point
   * mount proc and keep it, the application is enable only if fastboot kernel argument is supplied
   * single will disable fastboot
   */
  int main()
  {
    char tstr[255];
    SysLinux::mount_procfs(nullptr);
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
      // return -1;
    }
    // block SIGUSR1 on main thread
    sigset_t sig_mask;
    sigset_t oldmask;

    sigemptyset(&sig_mask);
    sigaddset(&sig_mask, SIGUSR1);
    sigprocmask(SIG_BLOCK, &sig_mask, &oldmask);
    //signal(SIGUSR1,SIG_IGN);

    memset(status, 0, sizeof(status));      //clear all status information
    for (auto &st : status)
    {
      //st.status =
    }

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
      if (it->grp_id != grp_none_id)
      {
        status[it->grp_id].grp_ref++;     // also use as group counter
        status[it->grp_id].status = waiting;
      }
    }
    status[none_id].status = done;
    status[grp_none_id].status = done;

    std::list<std::thread> threads;

    threads.emplace_back(sthread, this);
    threads.emplace_back(sthread, this);
    threads.emplace_back(sthread, this);
//    threads.emplace_back(sthread, this);

    for (auto &it : threads)
    {
      it.join();
    }
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
      if (it->grp_id != grp_none_id)
      {
        --status[it->grp_id].grp_ref;
        if (status[it->grp_id].grp_ref == 0)
        {
          if (status[it->grp_id].child_count > 1)
            notify = true;    // more than one task has been release
          status[it->grp_id].status = done;
        }
      }
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
        std::cout << 'W' << std::endl;
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

  static void print_statics(void* p)
  {
    linux_init* lnx = reinterpret_cast<linux_init*>(p);
    for (auto *t = lnx->begin; t != lnx->end; ++t)
    {
      auto &st = lnx->status[t->id];
      std::cout << getTaskName(t->id) << " "
          << (st.ended.tv_nsec / 1000000 + st.ended.tv_sec * 1000) - (st.started.tv_nsec / 1000000 + st.started.tv_sec * 1000) << " ms" << std::endl;
    }

  }
  static void sthread(linux_init* lnx)
  {
    const task_info_t* t = nullptr;
    for (t = lnx->peekTask(t); t != nullptr; t = lnx->peekTask(t))
    {
      std::cout << " S " << getTaskName(t->id) << std::endl;
      clock_gettime(CLOCK_MONOTONIC, &lnx->status[t->id].started);
      t->fnc(lnx);
      clock_gettime(CLOCK_MONOTONIC, &lnx->status[t->id].ended);
      std::cout << " E " << getTaskName(t->id) << " "
          << (lnx->status[t->id].ended.tv_nsec / 1000000 + lnx->status[t->id].ended.tv_sec * 1000)
           - (lnx->status[t->id].started.tv_nsec / 1000000 + lnx->status[t->id].started.tv_sec * 1000) << " ms" << std::endl;
    }
  }

  static void procps(void*)
  {
    SysLinux::execute_c("/sbin/sysctl -q --system");
  }

  /*
   * Depends on procfs
   */
  static void udev(void*)
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

   // SysLinux::execute_c("udevadm info --cleanup-db");    // it will be empty
    SysLinux::execute_c("/sbin/udevd --daemon");    // move to the end be carefull with network cards
    //SysLinux::execute_c("/bin/udevadm trigger --action=add");
    // SysLinux::execute_c("/bin/udevadm settle", true);   //wait for events
  }

  // do not execute
  void udev_finish()
  {
    SysLinux::execute_c("/lib/udev/udev-finish");
  }

  // execute some init script
  void init_d()
  {
    SysLinux::execute_c("/etc/init.d/hwclock start");
    SysLinux::execute_c("/etc/init.d/urandom start");
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

  static void hostname(void* p)
  {
    linux_init* lnx = reinterpret_cast<linux_init*>(p);
    //read etc/hostname if not empty the apply otherwise use localhost
    int r;
    FILE * pFile = fopen("/etc/hostname", "r");
    if (pFile != NULL)
    {
      r = fread(lnx->host, 1, sizeof(lnx->host), pFile);
      if (r > 0)
      {
        lnx->host[r - 1] = 0;
      } else
        strcpy(lnx->host, "localhost");
      fclose(pFile);
    } else
      perror("/etc/hostname ");
    sethostname(lnx->host, strlen(lnx->host));
  }

  void acpi_daemon()
  {
    SysLinux::execute_c("/etc/init.d/acpid start");
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
 procfs
 deferred in thread
 check cmdline parameters
 - dev  - (udev adm a,all fs with mount all)
 - sys,run,tmp - setup folders
 - then X and xfce
 - the system initialization
 */
int main()
{

  // static initialization of struct is faster than using object, the compiler will store a table and just copy over
  // using const all data will be in RO memory really fast
  static const linux_init::task_info_t tasks[] = {    ///
      { &SysLinux::mount_root, root_fs_id, grp_krn_fs_id, none_id, none_id },    //
          { &SysLinux::mount_sysfs, sys_fs_id, grp_krn_fs_id, none_id, none_id },    //
          { &SysLinux::mount_devfs, dev_fs_id, grp_krn_fs_id, run_fs_id, none_id },    //
          { &SysLinux::mount_tmp, tmp_fs_id, grp_krn_fs_id, none_id, none_id },    //
          { &SysLinux::mount_run, run_fs_id, grp_krn_fs_id, none_id, none_id },    //
          { &SysLinux::mount_all, all_fs_id, grp_krn_fs_id, dev_fs_id, none_id },    //
          { &linux_init::hostname, hostname_id, grp_none_id, none_id, none_id },    //
          { &linux_init::udev, udev_id, grp_none_id, dev_fs_id, none_id },    //
          { &linux_init::procps, procps_id, grp_none_id, udev_id, none_id },    //
      };
  linux_init lnx(tasks, tasks + sizeof(tasks) / sizeof(*tasks));
  return lnx.main();
}
