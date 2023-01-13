/* ************************************************************************** */
/** Descriptive File Name

  @Company
    Company Name

  @File Name
    filename.h

  @Summary
    Brief description of the file.

  @Description
    Describe the purpose of this file.
 */
/* ************************************************************************** */

#ifndef LEDS_H
#define LEDS_H

//Libraries
#include <xc.h>

//Define LED ports
#define LEDS_INIT() ({ TRISE = 0x00, LATE = 0x00;})

#endif // LEDS_H