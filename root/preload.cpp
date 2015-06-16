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

#include <cstring>

/*
 * Receive one block
 * and ask for more.
 * block has reference counter to be able to release it
 */
int processBlock(char* blk_s, char* blk_e)
{
  char* dt_s = blk_s; // when data start
  char* dt_e; // when data end.

  // firt valid char
  while (dt_s != blk_e && *dt_s <= ' ')
    ++dt_s;
  // check for nothing found
  if (dt_s == blk_e)
    return -1; // no more data
  dt_e = dt_s;
  while (dt_e != blk_e && *dt_e != ' ')
    ++dt_e;
  if (dt_e == blk_e)
  {
    // find how much data are in the new block. check if there is enough space to move the
    char* nblk_s = nullptr; // next block
    char* nblk_e;
    if (nblk_s == nullptr)
    {
      //no more blocks move down data
      if (dt_s == blk_s)
      {
        return -4;  // to long
      }
      memmove(dt_s,dt_s-1,dt_e - dt_s);
      --dt_s;
    }
    else
    {
      dt_e = nblk_s;
      while (dt_e != nblk_e && *dt_e != ' ')
          ++dt_e;
      if (dt_e == nblk_e)
        return -2;    // two blocks crossing
      if (dt_s - blk_s < dt_e - nblk_s + 1)
        return -3;  // no space to modve down data
      memmove(dt_s,dt_s - ( dt_e - nblk_s + 1),blk_e - dt_s);
      dt_s -= (dt_e - nblk_s + 1);
      memcpy(blk_e - (dt_e - nblk_s + 1),nblk_s,dt_e - nblk_s);
      *blk_e = 0;
      blk_e = nblk_e;
      blk_s = nblk_s;
    }
  }
  else
  {
    *dt_e = 0;
  }
  // use data and repeat

}

int main()
{
  char test[]="kjk jkj jkj j kj j kjk";
  processBlock(test,test+sizeof(test));
}


