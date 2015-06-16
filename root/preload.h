/*
 * preload.cpp
 *
 *  Created on: 15 Jun 2015
 *      Author: lester
 *
 *  Read ahead a list files stored in a file.
 *  Try to order reads base on physical position on disk.
 *  Check if squeduler can be change to be more agresive and not allow multiple operations
 *  using noop scheduler seam to be a good option
 *
 * cat /sys/block/{DEVICE-NAME}/queue/scheduler
 * cat /sys/block/sda/queue/scheduler
 * echo {SCHEDULER-NAME} > /sys/block/{DEVICE-NAME}/queue/scheduler
 * echo noop > /sys/block/hda/queue/scheduler
 *
 * kernel cmd line elevator=noop
 */

#ifndef PRELOAD_H_
#define PRELOAD_H_


#include <cstring>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <cstdlib>

/*
 * map a full file in memory
 */
/**
 * Map wrapper for map object
 */
class map
{
  void* ptr_ = nullptr;
  __off_t size_ = 0;
public:
  map(const map&) = delete;
  map& operator =(const map&) = delete;
  map& operator =(map&& m)
  {
    if (ptr_)
      ::munmap(ptr_, size_);
    size_ = m.size_;
    ptr_ = m.ptr_;
    m.ptr_ = nullptr;
    return *this;
  }
public:
  map()
  {
  }
  map(void* ptr, __off_t size) :
      ptr_(ptr), size_(size)
  {
  }
  map(map&& m) :
      ptr_(m.ptr_), size_(m.size_)
  {
    m.ptr_ = nullptr;
  }
  ~map()
  {
    if (ptr_)
      ::munmap(ptr_, size_);
  }
  operator bool() const
  {
    return (ptr_ != nullptr);
  }
  void* begin()
  {
    return ptr_;
  }
  void* end()
  {
    return ((char*) ptr_) + size_;
  }
};

class fd
{
public:
  bool open(const char* path, int flag, mode_t mode = S_IRUSR | S_IWUSR)
  {
    fd_ = ::open(path, flag, mode);
    if (fd_ == -1)
      perror(path);
    return (fd_ != -1);
  }
  ~fd()
  {
    if (fd_ != -1)
      ::close(fd_);
  }
  operator bool() const
  {
    return (fd_ != -1);
  }
  map getFullMap()
  {
    void* ptr;
    struct stat buf;
    ::fstat(fd_, &buf);
    ptr = mmap(nullptr, buf.st_size, PROT_WRITE | PROT_READ, MAP_PRIVATE, fd_, 0);
    if (ptr == MAP_FAILED)
    {
      perror("map");
      ptr = nullptr;
    }
    return
    { ptr, buf.st_size};
  }
private:
  int fd_;
};

class preload_parser
{
private:
  fd file;
  map m;
  char* blk_s;
  char* blk_e;
  char* dt_s;    // when data start
  char* dt_e;    // when data end.
public:
  int main(const char* fname)
  {
    file.open(fname, 0, S_IRUSR);
    if (file)
    {
      m = file.getFullMap();
      if (m)
      {
        blk_s = reinterpret_cast<char*>(m.begin());
        blk_e = reinterpret_cast<char*>(m.end());
        dt_e = blk_s;    // when data end.
        processBlock();
      }
    }
    return 0;
  }
  void processBlock()
  {
    char* dt;
    unsigned long int v1;
    unsigned long int v2;
    char* endptr;
    do
    {
      dt = nextToken(' ');
      if (dt != nullptr)
      {
        v1 = strtoul (dt,&endptr,10);
        //printf("%s ",dt);
        dt = nextToken(' ');
      }
      if (dt != nullptr)
      {
        v2 = strtoul (dt,&endptr,10);
        //printf("%s ",dt);
        dt = nextToken('\n');
      }
      if (dt != nullptr)
      {
        //printf("%s\n",dt);
      }
    } while (dt != nullptr);
  }
  /*
   * Receive one block
   * and ask for more.
   * block has reference counter to be able to release it
   */
  char* nextToken(char delimiter)
  {
    dt_s = dt_e;
    // firt valid char
    while (dt_s != blk_e && *dt_s <= ' ')
      ++dt_s;
    // check for nothing found
    if (dt_s == blk_e)
      return nullptr;    // no more data
    dt_e = dt_s;
    while (dt_e != blk_e && *dt_e != delimiter)
      ++dt_e;
    if (dt_e == blk_e)
    {
      // find how much data are in the new block. check if there is enough space to move the
      char* nblk_s = nullptr;    // next block
      char* nblk_e = nullptr;
      //nextBlock(nblk_s, nblk_e);
      if (nblk_s == nullptr)
      {
        //no more blocks move down data
        if (dt_s == blk_s)
        {
          return nullptr;    // to long
        }
        memmove(dt_s - 1, dt_s, dt_e - dt_s);
        --dt_s;
        --dt_e;    // it will bie increment later
      } else
      {
        dt_e = nblk_s;
        while (dt_e != nblk_e && *dt_e != ' ')
          ++dt_e;
        if (dt_e == nblk_e)
          return nullptr;    // two blocks crossing
        if (dt_s - blk_s < dt_e - nblk_s + 1)
          return nullptr;    // no space to modve down data
        memmove(dt_s - (dt_e - nblk_s + 1), dt_s, blk_e - dt_s);
        dt_s -= (dt_e - nblk_s + 1);
        memcpy(blk_e - (dt_e - nblk_s + 1), nblk_s, dt_e - nblk_s);
        *blk_e = 0;
        blk_e = nblk_e;
        blk_s = nblk_s;
      }
    } else
    {
      *dt_e = 0;
    }
    // use data and repeat
    ++dt_e;
    return dt_s;
  }

};
#endif


