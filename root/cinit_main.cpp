/*
 Small c++ application to initialize the system
 Running N threads using a simple dependency list
 it tend to be faster than alternative using bash interpreter
 g++ -std=c++11 -g -lpthread cinit.cpp -o cinit
 apt-get install inotify-tools

 SIGUSR1
 This signal is used quite differently from either of the above. When the server starts, it checks to see if it has inherited SIGUSR1 as SIG_IGN instead of the usual SIG_DFL. In this case, the server sends a SIGUSR1 to its parent process after it has set up the various connection schemes. Xdm uses this feature to recognize when connecting to the server is possible.

 */

#include <sys_linux.h>
#include <tasks.h>
#include <preload.h>


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
  //SysLinux::mount_procfs(nullptr);

  // static initialization of struct is faster than using object, the compiler will store a table and just copy over
  // using const all data will be in RO memory really fast

  return -1;
}
