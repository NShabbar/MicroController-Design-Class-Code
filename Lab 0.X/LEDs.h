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

//Define initiation of LED ports.
#define LEDS_INIT() ({ TRISE = 0x00, LATE = 0x00;})

//Define to set LEDs to specific value for future labs.
#define LEDS_SET(newPattern) (LATE = newPattern)

//Define to retrieve value of LED, but does not change them.
#define LEDS_GET() LATE;

#endif // LEDS_H