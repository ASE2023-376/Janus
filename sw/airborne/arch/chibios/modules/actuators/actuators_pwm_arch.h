/*
 * Copyright (C) 2013 AggieAir, A Remote Sensing Unmanned Aerial System for Scientific Applications
 * Utah State University, http://aggieair.usu.edu/
 *
 * Michal Podhradsky (michal.podhradsky@aggiemail.usu.edu)
 * Calvin Coopmans (c.r.coopmans@ieee.org)
 *
 *
 * This file is part of paparazzi.
 *
 * paparazzi is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * paparazzi is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with paparazzi; see the file COPYING.  If not, write to
 * the Free Software Foundation, 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
/**
 * @file arch/chibios/modules/actuators/actuators_pwm_arch.h
 * Interface from actuators to ChibiOS PWM driver
 *
 * PWM configuration files are defined in the board file,
 * so maximal architecture independence is ensured.
 */
#ifndef ACTUATORS_PWM_ARCH_H
#define ACTUATORS_PWM_ARCH_H

#include "std.h"
#include "hal.h"

#include BOARD_CONFIG

#ifndef ACTUATORS_PWM_NB
#define ACTUATORS_PWM_NB 8
#endif

extern int32_t actuators_pwm_values[ACTUATORS_PWM_NB];

extern void actuators_pwm_commit(void);

#define ActuatorPwmSet(_i, _v) { actuators_pwm_values[_i] = _v; }
#define ActuatorsPwmCommit  actuators_pwm_commit

#endif /* ACTUATORS_PWM_ARCH_H */
