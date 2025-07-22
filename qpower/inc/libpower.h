
#pragma once

void nt_enable_standby(uint64_t sleep_time);
void nt_enable_indef_deepsleep(void);

int mcu_sleep_enter(void);
void ram_minimum_code(void);
void mcu_sleep_wakeup(void);
