#include "iwdg_driver.h"
#include "iwdg.h"


void iwdg_feeddog(void)
{
    hiwdg.Instance = IWDG;
	HAL_IWDG_Refresh(&hiwdg);
}

