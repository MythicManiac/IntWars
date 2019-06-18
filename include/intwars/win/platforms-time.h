/*
 * Copyright (C) 2010 Franz Leitl, Embedded Systems Lab
 *
 * This file is part of the CRN Toolbox.
 * The CRN Toolbox is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 * The CRN Toolbox is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 * You should have received a copy of the GNU Lesser General Public License
 * along with the CRN Toolbox; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

/**
 * This header contains all kind of tweaks and hacks to get the toolbox
 * running cross platform.
 */

/**
 * Cross-platform version for "sys/time.h".
 */

// On Unix platforms there is usually a sys/time header.
#if defined(PLATFORM_OPERATINGSYSTEM_UNIX)
#   include <sys/time.h>
#endif

#if defined(WIN32) || defined(_WIN32)
// TODO can we include <sys/time.h> here to? Maybe on MinGW or Cygwin at least?
// #   include <sys/time.h>
#   include <windows.h>


// HACK replace lower-case unix sleep with windows-like upper-case Sleep.
#   define sleep Sleep


// HACK ugly hack...
#   ifndef timeradd
// Implement the function timeradd
#       define timeradd(a, b, result) \
                  (result)->tv_sec = (a)->tv_sec + (b)->tv_sec; \
                  (result)->tv_usec = (a)->tv_usec + (b)->tv_usec; \
                  if ((result)->tv_usec >= 1000000) \
                  { \
                          ++(result)->tv_sec; \
                          (result)->tv_usec -= 1000000; \
                  } \

#   endif

// HACK ugly hack...
// Implement the function timersub
#   ifndef timersub
#       define timersub(a, b, result) \
                  (result)->tv_sec = (a)->tv_sec - (b)->tv_sec; \
                  (result)->tv_usec = (a)->tv_usec - (b)->tv_usec; \
                  if ((result)->tv_usec < 0) { \
                          --(result)->tv_sec; \
                          (result)->tv_usec += 1000000; \
                  } \

#   endif

void usleep(__int64 usec) 
{ 
    HANDLE timer; 
    LARGE_INTEGER ft; 

    ft.QuadPart = -(10*usec); // Convert to 100 nanosecond interval, negative value indicates relative time

    timer = CreateWaitableTimer(NULL, TRUE, NULL); 
    SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0); 
    WaitForSingleObject(timer, INFINITE); 
    CloseHandle(timer); 
}
#endif