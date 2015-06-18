/*
 * preload_main.cpp
 *
 *  Created on: 17 Jun 2015
 *      Author: lester
 */

#include "preload.h"
#include <iostream>

/*
 * usage
 *  -- no arg --    // preload default file
 * sort <filename>  // load the file, sort it and write to console
 * init <init_app>  // load default file and fork to init_app
 * load <file>      // load the file
 *
 */

int main()
{
  std::cout << "Preload Started ..." << std::endl;
  preload_parser p;
  p.main("/var/lib/e4rat/startup.log");
}



