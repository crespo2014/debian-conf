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
  static int execute(char* cmd, bool wait = true, bool nofork = false)
  {
    std::vector<char*> arg;
    arg.reserve(10);
    split(cmd, arg);
    arg.push_back(nullptr);
    return launch(wait, arg.data(), nofork);
  }
  /*
   * Execute command from const char*
   */
  static int execute_c(const char* ccmd, bool wait = false, bool fork = true)
  {
    char cmd[512];
    strcpy(cmd, ccmd);
    return SysLinux::execute(cmd, wait, !fork);
  }

  // do not forget (char*) nullptr as last argument
  static int launch(bool wait, char * const * argv, bool nofork = false)
  {
    if (nofork)
    {
      execv(argv[0], (char* const *) argv);
      _exit (EXIT_FAILURE);
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
      _exit (EXIT_FAILURE);
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
    CHECK_ZERO(execute_c("/bin/mount -a", true), "mount all");
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
    CHECK_ZERO(mkdir("/dev/pts", 0755), "mkdir /dev/pts");
    CHECK_ZERO(mount("pts", "/dev/pts", "devpts", MS_SILENT | MS_NOSUID | MS_NOEXEC, "gid=5,mode=620"), "mount pts");
  }
  /*
   * Prepare all links and directories
   */
  static void mount_run(void*)
  {
    CHECK_ZERO(mount("run", "/run", "tmpfs", MS_NODEV | MS_NOEXEC | MS_SILENT | MS_NOSUID, ""), "mount /run ");
    CHECK_ZERO(mkdir("/run/lock", 01777), "mkdir /run/lock");
    CHECK_ZERO(mkdir("/run/shm", 01777), "mkdir /run/shm");
    CHECK_ZERO(chmod("/run/shm", 01777), "chmod /run/shm");
    CHECK_ZERO(chmod("/run/lock", 01777), "chmod /run/lock");
    CHECK_ZERO(symlink("/run", "/var/run"), "symlink /run /var/run");
    CHECK_ZERO(symlink("/run/lock", "/var/lock"), "symlink /run/lock /var/lock");
    CHECK_ZERO(symlink("/run/shm", "/dev/shm"), "symlink /run/shm /dev/shm");    // depends on dev_fs
  }
  static void mount_tmp(void*)
  {
    CHECK_ZERO(mount("tmp", "/tmp", "tmpfs", MS_NODEV | MS_NOEXEC | MS_SILENT | MS_NOSUID, ""), "mount /tmp");
    mkdir("/tmp/.X11-unix", 01777);
    chmod("/tmp/.X11-unix", 01777);
    mkdir("/tmp/.ICE-unix", 01777);
    chmod("/tmp/.ICE-unix", 01777);
  }
  static void deferred_modules(void*)
  {
    FILE * pFile;
    pFile = fopen("/proc/deferred_initcalls", "r");
    if (pFile == NULL)
      perror("/proc/deferred_initcalls");
    else
    {
      fclose(pFile);
    }
  }
  static void start_udev(void*)
  {
    execute_c("/etc/init.d/udev start", true);
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
    }
    perror(tstr);
  }
  static void set_cpu_governor(const char*)
  {

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
    SysLinux::execute_c("/sbin/udevd --daemon",true);    // move to the end be carefull with network cards
    //SysLinux::execute_c("/bin/udevadm trigger --action=add");
    // SysLinux::execute_c("/bin/udevadm settle", true);   //wait for events
  }

  // do not execute
  static void udev_finish()
  {
    SysLinux::execute_c("/lib/udev/udev-finish",true);
  }

  // execute some init script
  static void init_d()
  {
    //todo list files in /etc/rcS/ and run everything except for a block list of script that we are not run
    SysLinux::execute_c("/etc/init.d/hwclock start",true);
    SysLinux::execute_c("/etc/init.d/urandom start",true);
    SysLinux::execute_c("/etc/init.d/networking start",true);
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
      SysLinux::execute(tmp_str, false, true);
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
    SysLinux::execute_c("/etc/init.d/acpid start",true);
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
