/***************************************************************************//**
* @file ad9625.c
* @brief Implementation of AD9625 Driver.
* @author DBogdan (dragos.bogdan@analog.com)
********************************************************************************
* Copyright 2014(c) Analog Devices, Inc.
*
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
* - Redistributions of source code must retain the above copyright
* notice, this list of conditions and the following disclaimer.
* - Redistributions in binary form must reproduce the above copyright
* notice, this list of conditions and the following disclaimer in
* the documentation and/or other materials provided with the
* distribution.
* - Neither the name of Analog Devices, Inc. nor the names of its
* contributors may be used to endorse or promote products derived
* from this software without specific prior written permission.
* - The use of this software may or may not infringe the patent rights
* of one or more patent holders. This license does not release you
* from the requirement that you obtain separate licenses from these
* patent holders to use this software.
* - Use of the software either in source or binary form, must be run
* on or directly connected to an Analog Devices Inc. component.
*
* THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT,
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
* LIMITED TO, INTELLECTUAL PROPERTY RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include <stdint.h>
#include <xil_printf.h>
#include "platform_drivers.h"
#include "ad9625.h"

/******************************************************************************/
/************************ Variables Definitions *******************************/
/******************************************************************************/
uint8_t spi_is_initialized = 0;

/***************************************************************************//**
* @brief ad9625_spi_read
*******************************************************************************/
int32_t ad9625_spi_read(uint8_t slave_select,
						uint16_t reg_addr,
						uint8_t *reg_data)
{
	uint8_t buf[3];
	int32_t ret;

	buf[0] = 0x80 | (reg_addr >> 8);
	buf[1] = reg_addr & 0xFF;
	buf[2] = 0x00;

	ret = spi_write_and_read(slave_select, buf, 3);
	*reg_data = buf[2];

	return ret;
}

/***************************************************************************//**
* @brief ad9625_spi_write
*******************************************************************************/
int32_t ad9625_spi_write(uint8_t slave_select,
						 uint16_t reg_addr,
						 uint8_t reg_data)
{
	uint8_t buf[3];
	int32_t ret;

	buf[0] = reg_addr >> 8;
	buf[1] = reg_addr & 0xFF;
	buf[2] = reg_data;

	ret = spi_write_and_read(slave_select, buf, 3);

	return ret;
}

/***************************************************************************//**
* @brief ad9625_setup
*******************************************************************************/
int32_t ad9625_setup(uint32_t spi_device_id, uint8_t slave_select)
{
	uint8_t chip_id;
	uint8_t pll_stat;

	if (!spi_is_initialized) {
		spi_init(spi_device_id, 0, 0);
		spi_is_initialized = 1;
	}

	ad9625_spi_write(slave_select, AD9625_REG_CHIP_PORT_CONF, 0x18);
	ad9625_spi_write(slave_select, AD9625_REG_TRANSFER, 0x01);
	mdelay(10);

	ad9625_spi_write(slave_select, AD9625_REG_POWER_MODE, 0x00);
	ad9625_spi_write(slave_select, AD9625_REG_TRANSFER, 0x01);
	ad9625_spi_write(slave_select, AD9625_REG_JESD204B_LINK_CNTRL_1, 0x15);
	ad9625_spi_write(slave_select, AD9625_REG_JESD204B_LANE_POWER_MODE, 0x00);
	ad9625_spi_write(slave_select, AD9625_REG_DIVCLK_OUT_CNTRL, 0x11);
	ad9625_spi_write(slave_select, AD9625_REG_TEST_CNTRL, 0x00);
	ad9625_spi_write(slave_select, AD9625_REG_OUTPUT_MODE, 0x00);
	ad9625_spi_write(slave_select, AD9625_REG_OUTPUT_ADJUST, 0x10);
	ad9625_spi_write(slave_select, AD9625_REG_JESD204B_LINK_CNTRL_1, 0x14);
	ad9625_spi_write(slave_select, AD9625_REG_TRANSFER, 0x01);
	mdelay(10);

	ad9625_spi_read(slave_select, AD9625_REG_CHIP_ID, &chip_id);
	if(chip_id != AD9625_CHIP_ID)
	{
		xil_printf("Error: Invalid CHIP ID (0x%x).\n", chip_id);
		return -1;
	}

	ad9625_spi_read(slave_select, AD9625_REG_PLL_STATUS, &pll_stat);
	if((pll_stat & 0x80) != 0x80)
	{
		xil_printf("Error: AD9625 PLL is NOT locked (0x%x).\n", chip_id);
		return -1;
	}

	xil_printf("AD9625 successfully initialized.\n");

	return 0;
}
