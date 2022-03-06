/*
 * mscounter.h
 *
 *  Created on: Mar 4, 2022
 *      Author: vitya
 */

#ifndef SRC_MSCOUNTER_H_
#define SRC_MSCOUNTER_H_

extern volatile unsigned g_mscounter;

void      mscounter_init();
inline unsigned  mscounter() { return g_mscounter; }

#endif /* SRC_MSCOUNTER_H_ */
