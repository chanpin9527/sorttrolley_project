/*
 * This file is part of the EasyFlash Library.
 *
 * Copyright (c) 2015-2019, Armink, <armink.ztl@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Function: Portable interface for each platform.
 * Created on: 2015-01-16
 */

#include <easyflash.h>
#include <stdarg.h>

/*add*/
#include "sfud.h"
#include "cmsis_os.h"
#include "data_manager.h"

static uint32_t set_default_lock = SET_DEFAULT_FLASH_LOCK;

All_flash_param_st default_config_para = {
    .rail_operating_para.orbit_length = DEFAULT_ORBIT_LENGTH,
    .rail_operating_para.package_desk_length = DEFAULT_PACKAGE_DESK_LENGTH,
    .rail_operating_para.normal_speed = DEFAULT_NORMAL_SPEED,
    .rail_operating_para.enter_desk_speed = DEFAULT_ENTER_DESK_SPEED,
    .rail_operating_para.drum_parabola_speed = DEFAULT_DRUM_PARABOLA_SPEED,
    .rail_operating_para.drum_parabola_distance = DEFAULT_DRUM_PARABOLA_DISTANCE,
    
    .record_data.cur_location = 0,
    .record_data.connect_times = 0x00,
};

/* default environment variables set for user */
static const ef_env default_env_set[] = {
    {"set_default_ver",&set_default_lock,sizeof(set_default_lock)},
	{"package_info00", &default_config_para.package_info[0], sizeof(Set_package_informtion_t)},
	{"package_info01", &default_config_para.package_info[1], sizeof(Set_package_informtion_t)},
	{"package_info02", &default_config_para.package_info[2], sizeof(Set_package_informtion_t)},
	{"package_info03", &default_config_para.package_info[3], sizeof(Set_package_informtion_t)},
	{"package_info04", &default_config_para.package_info[4], sizeof(Set_package_informtion_t)},
	{"package_info05", &default_config_para.package_info[5], sizeof(Set_package_informtion_t)},
	{"package_info06", &default_config_para.package_info[6], sizeof(Set_package_informtion_t)},
	{"package_info07", &default_config_para.package_info[7], sizeof(Set_package_informtion_t)},
	{"package_info08", &default_config_para.package_info[8], sizeof(Set_package_informtion_t)},
	{"package_info09", &default_config_para.package_info[9], sizeof(Set_package_informtion_t)},
	{"package_info10", &default_config_para.package_info[10], sizeof(Set_package_informtion_t)},
	{"package_info11", &default_config_para.package_info[11], sizeof(Set_package_informtion_t)},
	{"package_info12", &default_config_para.package_info[12], sizeof(Set_package_informtion_t)},
	{"package_info13", &default_config_para.package_info[13], sizeof(Set_package_informtion_t)},
	{"package_info14", &default_config_para.package_info[14], sizeof(Set_package_informtion_t)},
	{"package_info15", &default_config_para.package_info[15], sizeof(Set_package_informtion_t)},
    
	{"orbit_length", &default_config_para.rail_operating_para.orbit_length, sizeof(default_config_para.rail_operating_para.orbit_length)},
	{"package_desk_length", &default_config_para.rail_operating_para.package_desk_length, sizeof(default_config_para.rail_operating_para.package_desk_length)},
	{"normal_speed", &default_config_para.rail_operating_para.normal_speed, sizeof(default_config_para.rail_operating_para.normal_speed)},
	{"enter_desk_speed", &default_config_para.rail_operating_para.enter_desk_speed, sizeof(default_config_para.rail_operating_para.enter_desk_speed)},
	{"drum_parabola_speed", &default_config_para.rail_operating_para.drum_parabola_speed, sizeof(default_config_para.rail_operating_para.drum_parabola_speed)},
	{"drum_parabola_distance", &default_config_para.rail_operating_para.drum_parabola_distance, sizeof(default_config_para.rail_operating_para.drum_parabola_distance)},
    
	{"cur_location", &default_config_para.record_data.cur_location, sizeof(default_config_para.record_data.cur_location)},
	{"connect_times", &default_config_para.record_data.connect_times, sizeof(default_config_para.record_data.connect_times)},
};


sfud_flash *w25q64_handle;
osMutexId xMutexEF;

/**
 * Flash port for hardware initialize.
 *
 * @param default_env default ENV set for user
 * @param default_env_size default ENV size
 *
 * @return result
 */
EfErrCode ef_port_init(ef_env const **default_env, size_t *default_env_size) {
    EfErrCode result = EF_NO_ERR;
	
	/*inital sfud*/
	sfud_init(); //初始化sfud
	w25q64_handle = sfud_get_device(SFUD_W25Q64JV_DEVICE_INDEX);
	
	osMutexDef(myMutex); //创建互斥量
	xMutexEF = osMutexCreate(osMutex(myMutex));

    *default_env = default_env_set;
    *default_env_size = sizeof(default_env_set) / sizeof(default_env_set[0]);

    return result;
}

/**
 * Read data from flash.
 * @note This operation's units is word.
 *
 * @param addr flash address
 * @param buf buffer to store read data
 * @param size read bytes size
 *
 * @return result
 */
EfErrCode ef_port_read(uint32_t addr, uint32_t *buf, size_t size) {
    EfErrCode result = EF_NO_ERR;

    /* You can add your code under here. */
	sfud_read(w25q64_handle, addr, size, (uint8_t *)buf);
    return result;
}

/**
 * Erase data on flash.
 * @note This operation is irreversible.
 * @note This operation's units is different which on many chips.
 *
 * @param addr flash address
 * @param size erase bytes size
 *
 * @return result
 */
EfErrCode ef_port_erase(uint32_t addr, size_t size) {
    EfErrCode result = EF_NO_ERR;

    /* make sure the start address is a multiple of EF_ERASE_MIN_SIZE */
    EF_ASSERT(addr % EF_ERASE_MIN_SIZE == 0);

    /* You can add your code under here. */
	sfud_erase(w25q64_handle, addr, size);
    return result;
}
/**
 * Write data to flash.
 * @note This operation's units is word.
 * @note This operation must after erase. @see flash_erase.
 *
 * @param addr flash address
 * @param buf the write data buffer
 * @param size write bytes size
 *
 * @return result
 */
EfErrCode ef_port_write(uint32_t addr, const uint32_t *buf, size_t size) {
    EfErrCode result = EF_NO_ERR;
    
    /* You can add your code under here. */
	sfud_write(w25q64_handle, addr, size, (uint8_t *)buf);
    return result;
}

/**
 * lock the ENV ram cache
 */
void ef_port_env_lock(void) {
    
    /* You can add your code under here. */
    osMutexWait(xMutexEF, portMAX_DELAY);
}

/**
 * unlock the ENV ram cache
 */
void ef_port_env_unlock(void) {
    
    /* You can add your code under here. */
    osMutexRelease(xMutexEF);
}


/**
 * This function is print flash debug info.
 *
 * @param file the file which has call this function
 * @param line the line number which has call this function
 * @param format output format
 * @param ... args
 *
 */
void ef_log_debug(const char *file, const long line, const char *format, ...) {

#ifdef PRINT_DEBUG

    va_list args;

    /* args point to the first variable parameter */
    va_start(args, format);

    /* You can add your code under here. */
    
    va_end(args);

#endif

}

/**
 * This function is print flash routine info.
 *
 * @param format output format
 * @param ... args
 */
void ef_log_info(const char *format, ...) {
    va_list args;

    /* args point to the first variable parameter */
    va_start(args, format);

    /* You can add your code under here. */
    
    va_end(args);
}
/**
 * This function is print flash non-package info.
 *
 * @param format output format
 * @param ... args
 */
void ef_print(const char *format, ...) {
    va_list args;

    /* args point to the first variable parameter */
    va_start(args, format);

    /* You can add your code under here. */
    
    va_end(args);
}
