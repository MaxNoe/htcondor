/***************************Copyright-DO-NOT-REMOVE-THIS-LINE**
  *
  * Condor Software Copyright Notice
  * Copyright (C) 1990-2004, Condor Team, Computer Sciences Department,
  * University of Wisconsin-Madison, WI.
  *
  * This source code is covered by the Condor Public License, which can
  * be found in the accompanying LICENSE.TXT file, or online at
  * www.condorproject.org.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  * AND THE UNIVERSITY OF WISCONSIN-MADISON "AS IS" AND ANY EXPRESS OR
  * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  * WARRANTIES OF MERCHANTABILITY, OF SATISFACTORY QUALITY, AND FITNESS
  * FOR A PARTICULAR PURPOSE OR USE ARE DISCLAIMED. THE COPYRIGHT
  * HOLDERS AND CONTRIBUTORS AND THE UNIVERSITY OF WISCONSIN-MADISON
  * MAKE NO MAKE NO REPRESENTATION THAT THE SOFTWARE, MODIFICATIONS,
  * ENHANCEMENTS OR DERIVATIVE WORKS THEREOF, WILL NOT INFRINGE ANY
  * PATENT, COPYRIGHT, TRADEMARK, TRADE SECRET OR OTHER PROPRIETARY
  * RIGHT.
  *
  ****************************Copyright-DO-NOT-REMOVE-THIS-LINE**/

#include "condor_common.h"
#include "condor_debug.h"
#include "sysapi.h"
#include "sysapi_externs.h"
#include "stdio.h"
#include "time.h"

int idle_time_test(int trials, int interval, int tolerance, double warn_ok_ratio) {
	int		foo = 0;
	int		foo2 = 0;
	int		bar = 0;
	int		bar2 = 0;
	int		i, j;
	time_t	raw_t1, raw_t2, cook_t1, cook_t2;
	time_t	raw_t3, raw_t4, cook_t3, cook_t4;
	int		raw_diff, cook_diff;
	int		unslept;
	FILE *	stream;
	char	filename[512];
	int		return_val = 0;
	int		num_warnings = 0;
	int		num_tests = 0;

	sysapi_idle_time_raw(&raw_t1, &raw_t2);
	dprintf(D_ALWAYS,"SysAPI: Initial sysapi_idle_time_raw() -> (%f,%f)\n",(float)raw_t1,
					(float)raw_t2);
	sysapi_idle_time(&cook_t1, &cook_t2);
	dprintf(D_ALWAYS, "SysAPI: Initial sysapi_idle_time() -> (%f,%f)\n", (float)cook_t1, 
					(float)cook_t2);

	if ((float)raw_t1 < 0 || (float)raw_t2 < 0 || (float)cook_t1 < 0 || (float)cook_t2 < 0) {
			dprintf(D_ALWAYS, "SysAPI: ERROR! Idle time should never be negative.\n");
			return_val = return_val || 1;
	}

	dprintf(D_ALWAYS, "SysAPI: Doing %d trials by testing idle time every %d seconds.\n", trials, 
					interval);
	dprintf(D_ALWAYS, "        If the new idle time is something other than the old idle time "
					"+/- %d seconds,\n", tolerance);
	dprintf(D_ALWAYS, "        a warning is issued. If warnings are issued more than %d%% of the "
					"time this test fails.\n");
	for (i=0; i<trials; i++) {
		// Make sure to sleep for interval seconds
		unslept = interval;
		while (unslept > 0) 
			unslept = sleep(unslept);

		// old		new	(wait x)				diff
		// 10		0-15	warn (user?)		-10 -> (5-tol)
		// 10		15		ok					(5-tol)-(5+tol)
		// 10		>15		warn (sleep prob)	(5+tol)
		
		sysapi_idle_time_raw(&raw_t3, &raw_t4);
			dprintf(D_ALWAYS,"SysAPI: After sleeping %d seconds, sysapi_idle_time_raw() -> "
							"(%d,%d)\n", interval, (int)raw_t3,(int)raw_t4);
		sysapi_idle_time(&cook_t3, &cook_t4);
			dprintf(D_ALWAYS, "SysAPI: After sleeping %d seconds, sysapi_idle_time() -> "
							"(%d,%d)\n", interval, (int)cook_t3, (int)cook_t4);

		// Raw system idle time
		raw_diff = raw_t3 - raw_t1;
		num_tests++;
		if (raw_diff < (interval - tolerance)) {
			dprintf(D_ALWAYS, "SysAPI: WARNING! The raw system idle time changed from %d to %d; "
							"did another process start?\n", (int)raw_t1, (int)raw_t3);
		}
		else if (raw_diff > (interval+tolerance)) {
			dprintf(D_ALWAYS, "SysAPI: WARNING! The raw system idle time changed from %d to %d "
							"(%d > %d) - this was more that we slept for.\n", (int)raw_t1, 
							(int)raw_t3, (int)raw_diff, interval-tolerance);
			num_warnings++;
		}

		// Cooked system idle time
		cook_diff = cook_t3 - cook_t1;
		num_tests++;
		if (cook_diff < (interval - tolerance)) {
			dprintf(D_ALWAYS, "SysAPI: WARNING! The cooked system idle time changed from %d to "
							"%d; did another process start?\n", (int)cook_t1, (int)cook_t3);
		}
		else if (cook_diff > (interval+tolerance)) {
			dprintf(D_ALWAYS, "SysAPI: WARNING! The cooked system idle time changed from %d to "
							"%d - this was more that we slept for.\n", (int)cook_t1, (int)cook_t3);
			num_warnings++;
		}


		// Raw console idle time
		raw_diff = raw_t4 - raw_t2;
		num_tests++;
		if (raw_diff < (interval - tolerance)) {
			dprintf(D_ALWAYS, "SysAPI: WARNING! The raw console idle time changed from %d to "
							"%d; was someone using the console?\n", (int)raw_t2, (int)raw_t4);
		}
		else if (raw_diff > (interval+tolerance)) {
			dprintf(D_ALWAYS, "SysAPI: WARNING! The raw console idle time changed from %d to "
							"%d - this was more that we slept for.\n", (int)raw_t2, (int)raw_t4);
			num_warnings++;
		}

		// Cooked console idle time
		cook_diff = cook_t4 - cook_t2;
		num_tests++;
		if (cook_diff < (interval - tolerance)) {
			dprintf(D_ALWAYS, "SysAPI: WARNING! The cooked console idle time changed from %d to "
							"%d; was someone using the console?\n", (int)cook_t2, (int)cook_t4);
		}
		else if (cook_diff > (interval+tolerance)) {
			dprintf(D_ALWAYS, "SysAPI: WARNING! The cooked console idle time changed from %d to "
							"%d - this was more that we slept for.\n", (int)cook_t2, (int)cook_t4);
			num_warnings++;
		}


		raw_t1 = raw_t3;
		raw_t2 = raw_t4;
		cook_t1 = cook_t3;
		cook_t2 = cook_t4;
	}

	if (((double)num_warnings/(double)num_tests) >= warn_ok_ratio) {
			// It is very common for the system idle time to decrease - this probably 
			// shouldn't cause failure. Console time, maybe...
			dprintf(D_ALWAYS, "SysAPI: ERROR! Warning tolerance exceeded (%2f\% warnings > %2f\% "
							"tolerance) .\n", ((double)num_warnings/(double)num_tests)*100, 
							warn_ok_ratio*100);
			return_val = return_val || 1;
	}
	dprintf(D_ALWAYS, "return_val = %d\n", return_val);
	return return_val;
}
