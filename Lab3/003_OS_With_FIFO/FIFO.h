#ifndef FIFO_H_
#define FIFO_H_

/********************************************************/
/*										Include														*/
/********************************************************/
#include "stdint.h"

/********************************************************/
/*										Typedef 													*/
/********************************************************/




/********************************************************/
/*										Prototypes												*/
/********************************************************/
uint32_t FIFO_GET(void);
void FIFO_PUT(uint32_t data);

#endif /* FIFO_H_ */
