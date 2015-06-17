/*
 * preload_main.cpp
 *
 *  Created on: 17 Jun 2015
 *      Author: lester
 */

#include "preload.h"

int main()
{
  printf("Preload Started ...\n");
  preload_parser p;
  p.main("/var/lib/e4rat/startup.log");
}



