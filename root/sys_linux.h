/*
 * sys_linux.h
 *  Class containing usefully functions to be use in linux
 *
 *  Created on: 19 Jun 2015
 *      Author: lester
 */

#ifndef SYS_LINUX_H_
#define SYS_LINUX_H_

//TODO add X server initialization

#include <vector>
#include <cstring>

#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/ptrace.h>
#include <asm/unistd.h>
#include <linux/sched.h>
 #include <pthread.h>
//#include <linux/syscalls.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sched.h>
#include "preload.h"
//#include <linux/ioprio.h>
//#include <ext2fs/ext2fs.h>
//#include <blkid/blkid.h>

#define TO_STRING(id)                 #id

// Execute system function and print error information if return code is not ecode
#define CHECK_EQUAL(fnc,ecode,msg)  if (fnc != ecode) perror(msg);
#define CHECK_ZERO(fnc,msg)         CHECK_EQUAL(fnc,0,msg)
#define CHECK_NOT(fnc,ecode,msg)    if (fnc == ecode) perror(msg);

#define EXIT(fnc,cnd) do \
  { \
    int r = fnc;  \
    if (r cnd) \
    { \
      perror(TO_STRING(fnc)); \
      exit(-1); \
    } \
  } while (0)

#define IOPRIO_WHO_PROCESS 1
#define IOPRIO_CLASS_IDLE 3
#define IOPRIO_CLASS_SHIFT 13
#define IOPRIO_IDLE_LOWEST (7 | (IOPRIO_CLASS_IDLE << IOPRIO_CLASS_SHIFT))

template<class T, std::size_t N>
T* end(T (&array)[N])
{
  return array + N - 1;
}

template<std::size_t N>
void inline cchar_to_vector(const char (&str)[N], std::vector<char>& v)
{
  v.assign(str, str + N);
}

class SysLinux
{
private:
  SysLinux()
  {
    cchar_to_vector("/tmp/.server.auth.XXXXXX", srv_auth_file);
    cchar_to_vector("/tmp/.user.auth.XXXXXX", usr_auth_file);
    *host = 0;
    *mcookie = 0;
  }
  SysLinux(const SysLinux&) = delete;
  SysLinux(const SysLinux&&) = delete;
  SysLinux& operator =(const SysLinux&) = delete;
  SysLinux& operator =(const SysLinux&&) = delete;
  // get a singleton.
  static SysLinux& get()
  {
    static SysLinux s;
    return s;
  }
public:
  /*
   * Split element delimiter by spaces
   */
  static void split(char* list, std::vector<char*>& v)
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
  /*
   * Execute a system command
   * wait if true wait for command to finish
   * no fork the current process
   */
  static int execute(char* cmd, bool wait = true, bool fork = true)
  {
    std::vector<char*> arg;
    arg.reserve(10);
    split(cmd, arg);
    arg.push_back(nullptr);
    return launch(wait, arg.data(), !fork);
  }
  /*
   * Execute command from const char*
   */
  static int execute_arg(std::initializer_list<const char*> list,bool wait = true,bool fork = true)
  {
    // max arguments are 20, i don want to use malloc after fork
    const char* arg[20];
    const char** p = arg;
    if (list.size() > 19)
    {
      printf("too many arguments");
      return -1;
    }
    for( const char* elem : list )
    {
      (*p) = elem;
      ++p;
    }
    (*p) = nullptr;
    return launch(wait,const_cast<char * const *>(arg),!fork);
    return 0;
  }
  // do not forget (char*) nullptr as last argument
  static int launch(bool wait, char * const * argv, bool nofork = false)
  {
    if (nofork)
    {
      execv(argv[0], (char* const *) argv);
      _exit(EXIT_FAILURE);
    }
    int status = 0;
    pid_t pid = fork();
    if (pid == -1)
    {
      perror(argv[0]);
      status = -1;
    } else if (pid > 0)
    {
      if (wait)
        waitpid(pid, &status, 0);
    }
    if (pid == 0)
    {
      // child
      execv(argv[0], (char* const *) argv);
      _exit(EXIT_FAILURE);
    }
    return status;
  }
  static void mount_root(void*)
  {
    CHECK_ZERO(mount("/dev/sda5", "/", "ext4", MS_NOATIME | MS_NODIRATIME | MS_REMOUNT | MS_SILENT, ""), "remount /");
  }
  // mount all filesystem in fstab
  static void mount_all(void*)
  {
    CHECK_ZERO(execute_arg({"/bin/mount","-a"}, true), "mount all");
  }
  static void mount_procfs(void*)
  {
    CHECK_ZERO(mount("", "/proc", "proc", MS_NOATIME | MS_NODIRATIME | MS_NODEV | MS_NOEXEC | MS_SILENT | MS_NOSUID, ""), "mount /proc");
  }
  static void mount_sysfs(void*)
  {
    CHECK_ZERO(mount("sys", "/sys", "sysfs", MS_NOATIME | MS_NODIRATIME | MS_NODEV | MS_NOEXEC | MS_SILENT | MS_NOSUID, ""), "mount sys");
  }
  static void mount_devfs(void*)
  {
    CHECK_ZERO(mount("dev", "/dev", "devtmpfs", MS_SILENT, ""), "mount dev");
    // depends on module devpts, do modules init before this
    CHECK_ZERO(mkdir("/dev/pts", 0755), "mkdir /dev/pts");
    CHECK_ZERO(mount("pts", "/dev/pts", "devpts", MS_SILENT | MS_NOSUID | MS_NOEXEC, "gid=5,mode=620"), "mount pts");
  }
  /*
   * Prepare all links and directories
   */
  static void mount_run(void*)
  {
    CHECK_ZERO(mount("run", "/run", "tmpfs", MS_NODEV | MS_NOEXEC | MS_SILENT | MS_NOSUID, ""), "mount /run ");
  }
  static void mount_tmp(void*)
  {
    CHECK_ZERO(mount("tmp", "/tmp", "tmpfs", MS_NODEV | MS_NOEXEC | MS_SILENT | MS_NOSUID, ""), "mount /tmp");
  }

  static void setup_fs(void*) /* depends on all krn fs or ram fs */
  {
    struct stat sb;
    CHECK_ZERO(mkdir("/run/lock", 01777), "mkdir /run/lock");
    CHECK_ZERO(mkdir("/run/shm", 01777), "mkdir /run/shm");
    CHECK_ZERO(chmod("/run/shm", 01777), "chmod /run/shm");
    CHECK_ZERO(chmod("/run/lock", 01777), "chmod /run/lock");
    if (stat("/var/run", &sb) != 0)
    {
      CHECK_ZERO(symlink("/run", "/var/run"), "symlink /run /var/run");
    }
    if (stat("/var/lock", &sb) != 0)
    {
      CHECK_ZERO(symlink("/run/lock", "/var/lock"), "symlink /run/lock /var/lock");
    }
    CHECK_ZERO(symlink("/run/shm", "/dev/shm"), "symlink /run/shm /dev/shm");    // depends on dev_fs
    //X11
    mkdir("/tmp/.X11-unix", 01777);
    chmod("/tmp/.X11-unix", 01777);
    mkdir("/tmp/.ICE-unix", 01777);
    chmod("/tmp/.ICE-unix", 01777);
    //initctl
    CHECK_ZERO(mknod("/run/initctl",S_IRUSR | S_IWUSR | S_IFIFO,0),"/run/initctl");

  }
  // Set max priority for the current thread and return the old one, return value 0 means  everything was ok
  static int setPriorityMax(int& old)
  {
    int policy, s;
    struct sched_param param;
    s = pthread_getschedparam(pthread_self(), &policy, &param);
    if (s == 0)
    {
      old = param.sched_priority;
      pthread_setschedprio(pthread_self(),sched_get_priority_max(policy));
    }
    return s;
  }
  static int setPriorityMin(int& old)
  {
    int policy, s;
    struct sched_param param;
    s = pthread_getschedparam(pthread_self(), &policy, &param);
    if (s == 0)
    {
      old = param.sched_priority;
      pthread_setschedprio(pthread_self(),sched_get_priority_min(policy));
    }
    return s;
  }
  static int setPriority(int prio)
  {
    return pthread_setschedprio(pthread_self(),prio);
  }

  static void deferred_modules(void*)
  {
    const char* f = "/proc/deferred_initcalls";
    char name[30];
    int ret;
    int prio;
    int s = setPriorityMin(prio);
    auto fd = open(f, O_RDONLY);
    if (fd > 0)
    {
      while ((ret = read(fd, name, sizeof(name))) > 0)
      {
        usleep(10);
        //name[ret] = 0;
      }
      close(fd);
    } else
      perror(f);
    if (s == 0)
    {
      setPriority(prio);    // roll back to previous
    }
    SysLinux::execute_arg({"/bin/udevadm","trigger"},true);
  }
  static void readahead(void*)
  {
    int prio;
    int s = setPriorityMax(prio);
    preload_parser p;
    p.readahead("/var/lib/e4rat/startup.log");
    if (s == 0)
    {
      setPriority(prio);    // roll back to previous
    }
  }
  static void start_udev(void*)
  {
    execute_arg({"/etc/init.d/udev","start"},true);
    /*
     *  symlink("/dev/MAKEDEV", "/bin/true");
     *   SysLinux::execute_c("/sbin/udevd --daemon");  // move to the end be carefull with network cards
     */
  }
  static void set_disk_scheduler(const char* disk, const char* scheduler)
  {
    char tstr[255];
    sprintf(tstr, "/sys/block/%s/queue/scheduler", disk);
    auto fd = open(tstr, O_WRONLY);
    if (fd > 0)
    {
      sprintf(tstr, "%s\n", scheduler);
      write(fd, tstr, strlen(tstr));
      close(fd);
    } else
      perror(tstr);
  }
  static void set_cpu_governor(const char* gov)
  {
    char tstr[255];
    auto fd = open("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor", O_WRONLY);
    if (fd > 0)
    {
      sprintf(tstr, "%s\n", gov);
      write(fd, tstr, strlen(tstr));
      close(fd);
    } else
      perror(tstr);
  }
  static inline void ioprio_set(int, int, int)
  {
    //return syscall(SYS_ioprio_set, which, who, ioprio);
  }
  static void startXfce_s(void*)
  {
    get().startxfce4();
  }
  static void startX_s(void*)
  {
    get().startXserver();
  }
  static void procps(void*)
  {
    SysLinux::execute_arg({"/sbin/sysctl","-q","--system"}, true);
  }
  static void slim(void*)
  {
    SysLinux::execute_arg({"/etc/init.d/slim","start"}, true);
  }
  static void dbus(void*)
  {
    SysLinux::execute_arg({"/etc/init.d/dbus","start"}, true);
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
      perror("/sys/kernel/uevent_helper");
    } else
    {
      fwrite("", 0, 0, pFile);
      fclose(pFile);
    }

    // SysLinux::execute_c("udevadm info --cleanup-db");    // it will be empty
    SysLinux::execute_arg({"/sbin/udevd","--daemon"},true);    // move to the end be carefull with network cards
    SysLinux::execute_arg({"/bin/udevadm","trigger","--action=add"},true);
    // SysLinux::execute_c("/bin/udevadm settle", true);   //wait for events
  }

  // do not execute
  static void udev_finish()
  {
    // SysLinux::execute_c("/lib/udev/udev-finish",true);
  }

  // execute some init script
  static void init_d()
  {
    //todo list files in /etc/rcS/ and run everything except for a block list of script that we are not run
    SysLinux::execute_arg({"/etc/init.d/keyboard-setup","start"});
    SysLinux::execute_arg({"/etc/init.d/kbd","start"});
    SysLinux::execute_arg({"/etc/init.d/console-setup","start"});
    SysLinux::execute_arg({"/lib/udev/udev-finish"});
    SysLinux::execute_arg({"/etc/init.d/hwclock","start"});
    SysLinux::execute_arg({"/etc/init.d/urandom","start"});
    SysLinux::execute_arg({"/etc/init.d/networking","start"});
  }
  /*
   startxfc4 script c++ translation
   */

  void startxfce4()
  {
    char tmp_str[255];
    snprintf(tmp_str, sizeof(tmp_str) - 1, "/bin/su -l -c 'export %s=%s;export %s=:%d;exec /usr/bin/startxfce4' lester", env_authority,
        usr_auth_file.data(), env_display, x_display_id);
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

    int auth_file_fd = mkstemp(srv_auth_file.data());    // create file  file has to be delete when everything is done, but for just one x server keep it in tmp is ok
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
    snprintf(tmp_str, sizeof(tmp_str) - 1, "/usr/bin/xauth -q -f %s add :%d . %s", srv_auth_file.data(), x_display_id, mcookie);
    SysLinux::execute(tmp_str);

    // Client auth file
    auth_file_fd = mkstemp(usr_auth_file.data());
    if (auth_file_fd != -1)
    {
      close(auth_file_fd);
    }

    snprintf(tmp_str, sizeof(tmp_str) - 1, "/usr/bin/xauth -q -f %s add :%d . %s", usr_auth_file.data(), x_display_id, mcookie);
    SysLinux::execute(tmp_str);
    EXIT(chown(usr_auth_file.data(), 1000, 1000), == -1);    // change owner to main user

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

    snprintf(cptr, tmp_str + sizeof(tmp_str) - cptr - 1, "  -audit 0 -logfile /dev/kmsg -nolisten tcp -auth %s vt0%d", srv_auth_file.data(), x_vt_id);

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
      SysLinux::execute(tmp_str, false, false);
    }
    // wait for signal become pending, only blocked signal can be pending, otherwise the signal will be generated
    r = sigtimedwait(&sig_mask, nullptr, &sig_timeout);
    if (r != SIGUSR1)
    {
      printf("Error waiting for X server");
    }
  }
  static void hostname_s(void*)
  {
    get().hostname();
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

  static void acpi_daemon(void*)
  {
    SysLinux::execute_arg({"/etc/init.d/acpid","start"}, true);
  }

  static void session_manager(void*)
  {
    /*
    start session manager using console-kit ck-launch-session
    root   /usr/sbin/console-kit-daemon --no-daemon
    root   /usr/lib/policykit-1/polkitd --no-debug
    lester /usr/bin/ck-launch-session /usr/bin/dbus-launch --exit-with-session x-session-manager
    lester /usr/bin/ssh-agent /usr/bin/ck-launch-session /usr/bin/dbus-launch --exit-with-session
    lester /bin/sh /etc/xdg/xfce4/xinitrc -- /etc/X11/xinit/xserverrc

    dbus-send --system --print-reply --dest="org.freedesktop.ConsoleKit" /org/freedesktop/ConsoleKit/Manager org.freedesktop.ConsoleKit.Manager.Stop
    Error org.freedesktop.ConsoleKit.Manager.NotPrivileged: Not Authorized
    */
  }
private:
  // system initialization information
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

  char host[100];    // Host name
  std::vector<char> srv_auth_file;
  std::vector<char> usr_auth_file;
  char mcookie[40];
};

#endif /* SYS_LINUX_H_ */
