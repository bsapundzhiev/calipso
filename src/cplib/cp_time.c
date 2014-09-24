/* cp_time.c
 *
 * Copyright (C) 2007 Borislav Sapundzhiev
 *         
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or (at
 * your option) any later version.
 *
 */

#include "calipso.h"
#include <time.h>

#define YEAR0           	1900             	/* the first year */
#define EPOCH_YR        	1970            	/* EPOCH = Jan 1 1970 00:00:00 */
#define SECS_DAY        	(24L * 60L * 60L)
#define LEAPYEAR(year)  	(!((year) % 4) && (((year) % 100) || !((year) % 400)))
#define YEARSIZE(year)  	(LEAPYEAR(year) ? 366U : 365U)
#define FIRSTSUNDAY(timp)	(((timp)->tm_yday - (timp)->tm_wday + 420) % 7)
#define FIRSTDAYOF(timp)   	(((timp)->tm_wday - (timp)->tm_yday + 420) % 7)

const unsigned int _ytab[2][12] = { { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }, {
		31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 } };

const char *wdays[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

const char *months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug",
		"Sep", "Oct", "Nov", "Dec" };

void cpo_gmtime(struct tm *tm, register const time_t *timer) {
	//static struct tm br_time;
	register struct tm *timep = tm;
	time_t time = *timer;
	register unsigned long dayclock, dayno;
	int year = EPOCH_YR;

	dayclock = (unsigned long) time % SECS_DAY;
	dayno = (unsigned long) time / SECS_DAY;

	timep->tm_sec = dayclock % 60;
	timep->tm_min = (dayclock % 3600) / 60;
	timep->tm_hour = dayclock / 3600;
	timep->tm_wday = (dayno + 4) % 7;

	while (dayno >= YEARSIZE(year)) {
		dayno -= YEARSIZE(year);
		year++;
	}

	timep->tm_year = year; //- YEAR0;
	timep->tm_yday = dayno;
	timep->tm_mon = 0;

	while (dayno >= _ytab[LEAPYEAR(year)][timep->tm_mon]) {
		dayno -= _ytab[LEAPYEAR(year)][timep->tm_mon];
		timep->tm_mon++;
	}

	timep->tm_mday = dayno + 1;
	timep->tm_isdst = 0;

	tm = timep;
	//return timep;
}

int cpo_http_time(char * buf, time_t *timer) {
	struct tm tm;
	cpo_gmtime(&tm, timer);

	sprintf(buf, "%s, %02d %s %d %02d:%02d:%02d GMT", wdays[tm.tm_wday],
			tm.tm_mday, months[tm.tm_mon], tm.tm_year, tm.tm_hour, tm.tm_min,
			tm.tm_sec);

	return OK;
}

int cpo_http_time2(char * buf, int len, time_t *timer) {
	struct tm tm;
	cpo_gmtime(&tm, timer);
	strftime(buf, len, "%a, %d %b %Y %H:%M:%S GMT", &tm);
	return OK;
}

