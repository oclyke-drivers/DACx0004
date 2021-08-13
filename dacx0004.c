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

#include "dax0004.h"

#define DACX0004_DAT0(sr) ((uint8_t)( ((sr.Rw & 0x01) << 4) | ((sr.cmd & 0x0F) << 0) ))

#define DA80004_DAT1(sr) ((uint8_t)( ((sr.add & 0x0F) << 4) | (((uint16_t)sr.dat & 0xF000) >> 12) ))
#define DA70004_DAT1(sr) ((uint8_t)( ((sr.add & 0x0F) << 4) | (((uint16_t)sr.dat & 0x3C00) >> 10) ))
#define DA60004_DAT1(sr) ((uint8_t)( ((sr.add & 0x0F) << 4) | (((uint16_t)sr.dat & 0x0F00) >> 8) ))

#define DA80004_DAT2(sr) ((uint8_t)( (((uint16_t)sr.dat & 0x0FF0) >> 4) ))
#define DA70004_DAT2(sr) ((uint8_t)( (((uint16_t)sr.dat & 0x03FC) >> 2) ))
#define DA60004_DAT2(sr) ((uint8_t)( (((uint16_t)sr.dat & 0x00FF) >> 0) ))

#define DA80004_DAT3(sr) ((uint8_t)( (((uint16_t)sr.dat & 0x000F) << 4) | ((sr.mod & 0x0F) << 0) ))
#define DA70004_DAT3(sr) ((uint8_t)( (((uint16_t)sr.dat & 0x0003) << 4) | ((sr.mod & 0x0F) << 0) ))
#define DA60004_DAT3(sr) ((uint8_t)( (        ( 0x00 << 0)              | ((sr.mod & 0x0F) << 0) )))



dax0004_status_e dax0004_init_dev(dax0004_dev_t* pdev, dax0004_ver_e ver, dax0004_if_t* pif, void* arg){
  if(pdev == NULL)          { return DACX0004_STAT_ERR_INVALID_ARG; }
  if(pif == NULL)           { return DACX0004_STAT_ERR_INVALID_ARG; }
  if(ver >= DACX0004_VER_NUM){ return DACX0004_STAT_ERR_UNKNOWN_VER; }

  pdev->_if = pif;
  pdev->_ver = ver;
  pdev->_arg = arg;

  // Set up control pins
  if(pdev->_if->set_clr  != NULL){ pdev->_if->set_clr(true, pdev->_arg); }
  if(pdev->_if->set_ldac != NULL){ pdev->_if->set_ldac(false, pdev->_arg); }
  if(pdev->_if->set_sync != NULL){ pdev->_if->set_sync(true, pdev->_arg); }

  return DACX0004_STAT_OK;
}


dax0004_status_e dax0004_write_sr(dax0004_dev_t* pdev, da80004_sr_t sr){
  if(pdev == NULL){ return DACX0004_STAT_ERR_INVALID_ARG; }
  if(pdev->_if == NULL){ return DACX0004_STAT_ERR_INVALID_ARG; }

  dax0004_status_e retval = DACX0004_STAT_OK;

  const uint8_t len = 4;
  uint8_t dat[len];

  dat[0] = DACX0004_DAT0(sr);
  switch(pdev->_ver){
    case DA80004 :
      dat[1] = DA80004_DAT1(sr);
      dat[2] = DA80004_DAT2(sr);
      dat[3] = DA80004_DAT3(sr);
      break;
    case DA70004 :
      dat[1] = DA70004_DAT1(sr);
      dat[2] = DA70004_DAT2(sr);
      dat[3] = DA70004_DAT3(sr);
      break;
    case DA60004 :
      dat[1] = DA60004_DAT1(sr);
      dat[2] = DA60004_DAT2(sr);
      dat[3] = DA60004_DAT3(sr);
      break;
    default :
      return DACX0004_STAT_ERR_UNKNOWN_VER;
      break;
  }

  retval |= pdev->_if->shift_sr(dat, len, pdev->_arg);

  return retval;
}

dax0004_status_e dax0004_format_sr(dax0004_dev_t* pdev, da80004_sr_t sr, uint8_t* dest, uint32_t len){
  if(pdev == NULL){ return DACX0004_STAT_ERR_INVALID_ARG; }
  if(dest == NULL){ return DACX0004_STAT_ERR_INVALID_ARG; }

  uint8_t d0 = DACX0004_DAT0(sr);
  uint8_t d1, d2, d3;
  uint32_t full_packs = 0;
  uint8_t remainder = len%4;

  switch(pdev->_ver){
    case DA80004 :
      d1 = DA80004_DAT1(sr);
      d2 = DA80004_DAT2(sr);
      d3 = DA80004_DAT3(sr);
      break;
    case DA70004 :
      d1 = DA70004_DAT1(sr);
      d2 = DA70004_DAT2(sr);
      d3 = DA70004_DAT3(sr);
      break;
    case DA60004 :
      d1 = DA60004_DAT1(sr);
      d2 = DA60004_DAT2(sr);
      d3 = DA60004_DAT3(sr);
      break;
    default :
      return DACX0004_STAT_ERR_UNKNOWN_VER;
      break;
  }

  for(full_packs = 0; full_packs < (len/4); full_packs++){
    *(dest + (4*full_packs) + 0) = d0;
    *(dest + (4*full_packs) + 1) = d1;
    *(dest + (4*full_packs) + 2) = d2;
    *(dest + (4*full_packs) + 3) = d3;
  }

  switch(remainder){  // fall-through intended
    case 3 :
      *(dest + (4*full_packs) + 2) = d2;
    case 2 :
      *(dest + (4*full_packs) + 1) = d1;
    case 1 :
      *(dest + (4*full_packs) + 0) = d0;
    default :
      break;
  }

  return DACX0004_STAT_OK;
}
