/*
Copyright 2019 Owen Lyke

Permission is hereby granted, free of charge, to any person obtaining a copy of this 
software and associated documentation files (the "Software"), to deal in the Software 
without restriction, including without limitation the rights to use, copy, modify, 
merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit 
persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be 
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING 
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef _DAX0004_H_
#define _DAX0004_H_

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

//
// Enumerations

typedef enum {
  DAX0004_STAT_OK = 0x00,       // nominal operation
  DAX0004_STAT_ERR,             // general error
  DAX0004_STAT_ERR_INVALID_ARG,
  DAX0004_STAT_ERR_UNKNOWN_VER,

  DAX0004_STAT_NUM          // total number of status indicators
} dax0004_status_e; // status indicators

typedef enum {
  DAX0004_RW_WRITE = 0x00,
  DAX0004_RW_READ = 0x01,

  DAX0004_RW_NUM
} dax0004_rw_e;

typedef enum {
    DAX0004_ADD_A = 0x00,
    DAX0004_ADD_B,
    DAX0004_ADD_C,
    DAX0004_ADD_ALL = 0x0F,

    DAX0004_ADD_NUM
} dax0004_add_e;

typedef enum {
    DAX0004_PWR_UP = 0x00,          // power up selected channels (normal operation)
    DAX0004_PWR_1k_GND,             // power down - 1k ohm to GND
    DAX0004_PWR_100k_GND,           // power down - 100 k ohm to GND
    DAX0004_PWR_HI_Z,               // power down - HI-Z

    DAX0004_PWR_NUM
} dax0004_pwr_e;

typedef enum {
    DAX0004_CLM_ZERO = 0x00,        // clear to zero scale
    DAX0004_CLM_MID,                // clear to mid scale
    DAX0004_CLM_FULL,               // clear to full scale

    DAX0004_CLM_NUM
} dax0004_clm_e;

typedef enum {
    DAX0004_CMD_WRITEn = 0x00,
    DAX0004_CMD_UPDATEn,
    DAX0004_CMD_WRITEn_UPDATEa,      // software LDAC
    DAX0004_CMD_WRITEn_UPDATEn,      // write to buffer n and update DAC n
    DAX0004_CMD_PWRnc,               // power up/down DAC n (mode field specifies channel?)
    DAX0004_CMD_CLEAR_MODE_REG,      // clear mode register, CM0 and CM1 are used to determine what happens when DACs cleared
    DAX0004_CMD_LDAC_REGc,           // LDAC register ?
    DAX0004_CMD_SOFTWARE_RESET,      //
    DAX0004_CMD_DISABLE_SDO_REG,     //
    _res0_,                         // reserved
    DAX0004_CMD_SHRT_CIRC_LIM_REGc,  // 
    DAX0004_CMD_SOFTWARE_CLEAR,      //
    _res1_,                         // reserved
    DAX0004_CMD_STATUS_REG,          // read status register
    DAX0004_CMD_NOP,                 // NOP
    _res2_,                         // reserved

    DAX0004_CMD_NUM
} dax0004_cmd_e;  // possible values for the sr cmd field

typedef enum {
  DA80004 = 0x00,
  DA70004,
  DA60004,

  DAX0004_VER_NUM
} dax0004_ver_e;  // possible versions of the dax0004        

//
// Type Definitions

typedef struct _dax0004_if_t {
  dax0004_status_e (*shift_sr)  (uint8_t* pdat, uint32_t len, void* arg); // required. shift out len bytes from pdat array on the SCLK and SDIN (DAX0004 perspective) lines, use SYNC line to select the device
  dax0004_status_e (*set_sync)  (bool lvl, void* arg);                    // optional. set the level of the sync line (true for high, false for low) (only used at startup to deselect the device - after which the shift_sr function should handle the SYNC line)
  dax0004_status_e (*set_ldac)  (bool lvl, void* arg);                    // optional. set the level of the ldac line (true for high, false for low) (if not implemented the SYNC line should be tied low)
  dax0004_status_e (*set_clr)   (bool lvl, void* arg);                    // optional. set the level of the  clr line (true for high, false for low) (if not implemented the CLR line should be tied high)
} dax0004_if_t;       // interface abstraction

typedef struct _dax0004_dev_t {
  dax0004_ver_e _ver; // which version is this device
  dax0004_if_t* _if;  // the interface to use
  void*         _arg; // user assignable pointer to pass into if functions
} dax0004_dev_t;      // device handle

typedef struct {
  uint8_t     _dc :  3;   // d31-d29  : 3 dont care bits 
  uint8_t     Rw  :  1;   // d28      : Rw set 1 for R or 0 for w
  uint8_t     cmd :  4;   // d27-d24  : command
  uint8_t     add :  4;   // d23-d20  : channel address
  uint16_t    dat : 16;   // d16-d4   : data field for command
  uint8_t     mod :  4;   // d3-d0    : mode
}da80004_sr_t;          // command shift register map

dax0004_status_e dax0004_init_dev(dax0004_dev_t* pdev, dax0004_ver_e ver, dax0004_if_t* pif, void* arg);
dax0004_status_e dax0004_write_sr(dax0004_dev_t* pdev, da80004_sr_t sr);  
dax0004_status_e dax0004_format_sr(dax0004_dev_t* pdev, da80004_sr_t sr, uint8_t* dest, uint32_t len);    // fills a buffer 'dest' with the 4-byte sr representation for len bytes. sr will be truncated and repeated as necessary

#endif // _DAX0004_H_