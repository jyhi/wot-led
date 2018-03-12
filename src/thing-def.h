#ifndef _THING_H
#define _THING_H

/**
 * Define name of the Thing
 *
 * This should be consistent with the one defined in thing.jsn
 */
#ifndef THING_NAME
#define THING_NAME "wot"
#endif

/**
 * Define description of the Thing
 */
#ifndef THING_DESCRIPTION
#define THING_DESCRIPTION "A WoT Thing"
#endif

/**
 * Define the SS pin of the SD card (which stores static contents)
 */
#ifndef SD_SS
#define SD_SS 4
#endif

/**
 * Define the LED pin
 */
#ifndef LED_PIN
#define LED_PIN 8
#endif

#endif /* end of include guard: _THING_H */
