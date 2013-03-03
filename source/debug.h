/*
 * Debug.h
 *
 *  Created on: 2013-02-13
 *      Author: sam
 */

#ifndef DEBUG_H_
#define DEBUG_H_

#include <string>
#include <iostream>

#define ISDEBUG 1
#define DEBUG(stuff) if (ISDEBUG) {std::cout << stuff;}

#endif /* DEBUG_H_ */
