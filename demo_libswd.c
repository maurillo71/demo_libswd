/*
 * Demo libswd
 *
 * Copyright (c) 2013  Mauro Gamba
*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 * Cross-compile with cross-gcc -I/path/to/cross-kernel/include
 */

#include <libswd.h>
#include <stdio.h>

 #define LOG_DEBUG 		printf

/**
 * This function sets interface buffers to MOSI direction.
 * MOSI (Master Output Slave Input) is a SWD Write operation.
 * OpenOCD use global "struct jtag_interface" pointer as interface driver.
 * OpenOCD driver must support "RnW" signal to drive output buffers for TRN.
 *
 * \param *libswdctx is the swd context to work on.
 * \param bits specify how many clock cycles must be used for TRN.
 * \return number of bits transmitted or negative LIBSWD_ERROR code on failure.
 */
int libswd_drv_mosi_trn(libswd_ctx_t *libswdctx, int bits)
{
	LOG_DEBUG("OpenOCD's libswd_drv_mosi_trn(libswdctx=@%p, bits=%d)\n", (void *)libswdctx, bits);
	if (bits < LIBSWD_TURNROUND_MIN_VAL && bits > LIBSWD_TURNROUND_MAX_VAL)
		return LIBSWD_ERROR_TURNAROUND;

	int res, val = 0;
	static char buf[LIBSWD_TURNROUND_MAX_VAL];
	/* Use driver method to set low (write) signal named RnW. */
	// res = jtag_interface->bitbang(NULL, "RnW", 0, &val);
	// if (res < 0)
	// 	return LIBSWD_ERROR_DRIVER;

	/* Clock specified number of bits for proper TRN transaction. */
	// res = jtag_interface->transfer(NULL, bits, buf, buf, 0);
	// if (res < 0)
	// 	return LIBSWD_ERROR_DRIVER;

	return bits;
}


/**
 * This function sets interface buffers to MISO direction.
 * MISO (Master Input Slave Output) is a SWD Read operation.
 * OpenOCD use global "struct jtag_interface" pointer as interface driver.
 * OpenOCD driver must support "RnW" signal to drive output buffers for TRN.
 *
 * \param *libswdctx is the swd context to work on.
 * \param bits specify how many clock cycles must be used for TRN.
 * \return number of bits transmitted or negative LIBSWD_ERROR code on failure.
 */
int libswd_drv_miso_trn(libswd_ctx_t *libswdctx, int bits)
{
	LOG_DEBUG("OpenOCD's libswd_drv_miso_trn(libswdctx=@%p, bits=%d)\n", (void *)libswdctx, bits);
	if (bits < LIBSWD_TURNROUND_MIN_VAL && bits > LIBSWD_TURNROUND_MAX_VAL)
		return LIBSWD_ERROR_TURNAROUND;

	static int res, val = 1;
	static char buf[LIBSWD_TURNROUND_MAX_VAL];

	/* Use driver method to set high (read) signal named RnW. */
	// res = jtag_interface->bitbang(NULL, "RnW", 0xFFFFFFFF, &val);
	// if (res < 0)
	// 	return LIBSWD_ERROR_DRIVER;

	/* Clock specified number of bits for proper TRN transaction. */
	// res = jtag_interface->transfer(NULL, bits, buf, buf, 0);
	// if (res < 0)
	// 	return LIBSWD_ERROR_DRIVER;

	return bits;
}

/**
 * Driver code to read 32-bit data (int type).
 * MISO (Master Input Slave Output) is a SWD Read Operation.
 *
 * \param *libswdctx swd context to work on.
 * \param *cmd point to the actual command being sent.
 * \param *data points to the char buffer array.
 * \bits tells how many bits to send (at most 32).
 * \bits nLSBfirst tells the shift direction: 0 = LSB first, other MSB first.
 * \return data count transferred, or negative LIBSWD_ERROR code on failure.
 */
int libswd_drv_miso_32(libswd_ctx_t *libswdctx, libswd_cmd_t *cmd, int *data, int bits, int nLSBfirst)
{
	if (data == NULL)
		return LIBSWD_ERROR_NULLPOINTER;
	if (bits < 0 && bits > 8)
		return LIBSWD_ERROR_PARAM;
	if (nLSBfirst != 0 && nLSBfirst != 1)
		return LIBSWD_ERROR_PARAM;

	static int i;
	static signed int res;
	static char misodata[32], mosidata[32];

	// res = jtag_interface->transfer(NULL, bits, mosidata, misodata, LIBSWD_DIR_LSBFIRST);
	// if (res < 0)
	// 	return LIBSWD_ERROR_DRIVER;
	/* Now we need to reconstruct the data byte from shifted in LSBfirst byte array. */
	*data = 0;
	for (i = 0; i < bits; i++)
		*data |= (misodata[(nLSBfirst == LIBSWD_DIR_LSBFIRST) ? i : (bits - 1 - i)] ? (1 << i) : 0);
	// LOG_DEBUG("OpenOCD's libswd_drv_miso_32(libswdctx=@%p, cmd=@%p, data=@%p, bits=%d, nLSBfirst=0x%02X) reads: 0x%08X",
	// 	(void *)libswdctx, (void *)cmd, (void *)data, bits, nLSBfirst, *data);
	// LOG_DEBUG("OpenOCD's libswd_drv_miso_32() reads: 0x%08X\n", *data);
	return res;
}

/** We will use OpenOCD's logging mechanisms to show LibSWD messages.
 * SWD can have different loglevel set than the OpenOCD itself, so we need to
 * log all messages at OpenOCD level that will not block swd messages.
 * It is also possible to 'inherit' loglevel to swd from openocd.
 *
 * \param *libswdctx is the pointer to the libswd context to work with.
 * \param loglevel is the desired log level to show message at.
 * \param *msg, ... is the printf like message to be logged.
 * \return LIBSWD_OK on success, or error code otherwise.
 */
int libswd_log(libswd_ctx_t *libswdctx, libswd_loglevel_t loglevel, char *msg, ...)
{
	if (libswdctx == NULL)
		return LIBSWD_ERROR_NULLCONTEXT;
	if (loglevel > LIBSWD_LOGLEVEL_MAX)
		return LIBSWD_ERROR_PARAM;

	if (loglevel > libswdctx->config.loglevel)
		return LIBSWD_OK;
	va_list ap;
	va_start(ap, msg);
	/* Calling OpenOCD log functions here will cause program crash (va recurrent). */
	vprintf(msg, ap);
	va_end(ap);
	return LIBSWD_OK;
}

/**
 * Driver code to write 32-bit data (int type).
 * MOSI (Master Output Slave Input) is a SWD Write Operation.
 *
 * \param *libswdctx swd context to work on.
 * \param *cmd point to the actual command being sent.
 * \param *data points to the char buffer array.
 * \bits tells how many bits to send (at most 32).
 * \bits nLSBfirst tells the shift direction: 0 = LSB first, other MSB first.
 * \return data count transferred, or negative LIBSWD_ERROR code on failure.
 */
int libswd_drv_mosi_32(libswd_ctx_t *libswdctx, libswd_cmd_t *cmd, int *data, int bits, int nLSBfirst)
{
	LOG_DEBUG("OpenOCD's libswd_drv_mosi_32(libswdctx=@%p, cmd=@%p, data=0x%08X, bits=%d, nLSBfirst=0x%02X)",
		(void *)libswdctx, (void *)cmd, *data, bits, nLSBfirst);
	if (data == NULL)
		return LIBSWD_ERROR_NULLPOINTER;
	if (bits < 0 && bits > 8)
		return LIBSWD_ERROR_PARAM;
	if (nLSBfirst != 0 && nLSBfirst != 1)
		return LIBSWD_ERROR_PARAM;

	static unsigned int i;
	static signed int res;
	static char misodata[32], mosidata[32];

	/* UrJTAG drivers shift data LSB-First. */
	for (i = 0; i < 32; i++)
		mosidata[(nLSBfirst == LIBSWD_DIR_LSBFIRST) ? i : (bits - 1 - i)] = ((1 << i) & (*data)) ? 1 : 0;
	// res = jtag_interface->transfer(NULL, bits, mosidata, misodata, 0);
	// if (res < 0)
	// 	return LIBSWD_ERROR_DRIVER;
	return res;
}

/**
 * Use UrJTAG's driver to read 8-bit data (char type).
 * MISO (Master Input Slave Output) is a SWD Read Operation.
 *
 * \param *libswdctx swd context to work on.
 * \param *cmd point to the actual command being sent.
 * \param *data points to the char buffer array.
 * \bits tells how many bits to send (at most 8).
 * \bits nLSBfirst tells the shift direction: 0 = LSB first, other MSB first.
 * \return data count transferred, or negative LIBSWD_ERROR code on failure.
 */
int libswd_drv_miso_8(libswd_ctx_t *libswdctx, libswd_cmd_t *cmd, char *data, int bits, int nLSBfirst)
{
	if (data == NULL)
		return LIBSWD_ERROR_NULLPOINTER;
	if (bits < 0 && bits > 8)
		return LIBSWD_ERROR_PARAM;
	if (nLSBfirst != 0 && nLSBfirst != 1)
		return LIBSWD_ERROR_PARAM;

	static int i;
	static signed int res;
	static char misodata[8], mosidata[8];

	// res = jtag_interface->transfer(NULL, bits, mosidata, misodata, LIBSWD_DIR_LSBFIRST);
	// if (res < 0)
	// 	return LIBSWD_ERROR_DRIVER;
	/* Now we need to reconstruct the data byte from shifted in LSBfirst byte array. */
	*data = 0;
	for (i = 0; i < bits; i++)
		*data |= misodata[(nLSBfirst == LIBSWD_DIR_LSBFIRST) ? i : (bits - 1 - i)] ? (1 << i) : 0;
	LOG_DEBUG("OpenOCD's libswd_drv_miso_8(libswdctx=@%p, cmd=@%p, data=@%p, bits=%d, nLSBfirst=0x%02X) reads: 0x%02X",
			(void *)libswdctx, (void *)cmd, (void *)data, bits, nLSBfirst, *data);
	return res;
}


/******************************************************************************
 * @{ oocd_transport_swd_libswd_drv
 * Driver bridge between OpenOCD and LibSWD.
 */

/**
 * Driver code to write 8-bit data (char type).
 * MOSI (Master Output Slave Input) is a SWD Write Operation.
 *
 * \param *libswdctx swd context to work on.
 * \param *cmd point to the actual command being sent.
 * \param *data points to the char data.
 * \bits tells how many bits to send (at most 8).
 * \bits nLSBfirst tells the shift direction: 0 = LSB first, other MSB first.
 * \return data count transferred, or negative LIBSWD_ERROR code on failure.
ar)*/
int libswd_drv_mosi_8(libswd_ctx_t *libswdctx, libswd_cmd_t *cmd, char *data, int bits, int nLSBfirst)
{
	LOG_DEBUG("OpenOCD's libswd_drv_mosi_8(libswdctx=@%p, cmd=@%p, data=0x%02X, bits=%d, nLSBfirst=0x%02X)",
			(void *)libswdctx, (void *)cmd, *data, bits, nLSBfirst);
	if (data == NULL)
		return LIBSWD_ERROR_NULLPOINTER;
	if (bits < 0 && bits > 8)
		return LIBSWD_ERROR_PARAM;
	if (nLSBfirst != 0 && nLSBfirst != 1)
		return LIBSWD_ERROR_PARAM;

	static unsigned int i;
	static signed int res;
	static char misodata[8], mosidata[8];

	/* Split output data into char array. */
	for (i = 0; i < 8; i++)
		mosidata[(nLSBfirst == LIBSWD_DIR_LSBFIRST) ? i : (bits - 1 - i)] = ((1 << i) & (*data)) ? 1 : 0;
	/* Then send that array into interface hardware. */
	// res = jtag_interface->transfer(NULL, bits, mosidata, misodata, 0);
	// if (res < 0)
	// 	return LIBSWD_ERROR_DRIVER;

	return res;
}

int main()
{
	libswd_ctx_t *libswdctx;
	int res, *idcode;
	libswdctx=libswd_init();
	if (libswdctx==NULL) return -1;
	//we might need to pass external driver structure to libswd_drv* functions
	//libswdctx->driver->device=...
	res=libswd_dap_detect(libswdctx, LIBSWD_OPERATION_EXECUTE, &idcode);
	if (res<0) {
		printf("ERROR: %s\n", libswd_error_string(res));
		return res;
	} else printf("IDCODE: 0x%X (%s)\n", *idcode, libswd_bin32_string(idcode));
	libswd_deinit(libswdctx);
	return 0;
}
