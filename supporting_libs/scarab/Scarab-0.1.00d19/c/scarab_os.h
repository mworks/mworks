#ifndef	SCARAB_OS_H
#define	SCARAB_OS_H

/*
 * OS abstraction API for Scarab-C.
 *
 * Scarab will call these methods, which can be implemented by a number
 * of operating systems.
 */

/*
 * Initializes the implementing operating system subsystem
 */
extern void scarab_os_init(void);

/*
 * Return the string form of an os error code.
 */
extern const char* scarab_os_strerror(int err);

/*
 * Return the OS specific error code.
 */
extern int scarab_os_geterr();

#endif
