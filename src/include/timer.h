/* timer.h - timers
 *
 * Copyright (C) 2012 Borislav Sapundzhiev
 *         
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at
 * your option) any later version.
 *
 */

#ifndef _TIMER_H
#define _TIMER_H

#include "calipso.h"

int wait_fd(int fd, short stimeout);

timer_t tmr_alrm_create(calipso_client_t *client, int sec);
void tmr_alrm_reset(calipso_client_t *client, int sec);
void tmr_alrm_kill(timer_t timerid);


#endif /* _TIMER_H */

