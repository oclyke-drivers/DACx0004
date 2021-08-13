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

#include "dax0004_platform_esp32.h"

// ESP32-specific interface functions for the DAX0004
dax0004_status_e shift_sr_esp32(uint8_t* pdat, uint32_t len, void* arg);
dax0004_status_e set_sync_esp32(bool lvl, void* arg);
dax0004_status_e set_ldac_esp32(bool lvl, void* arg);
dax0004_status_e set_clr_esp32(bool lvl, void* arg);

dax0004_if_t dax_if_esp32 = {
  .shift_sr = shift_sr_esp32,
  .set_sync = NULL,           // sync is handled by SPI master
  .set_ldac = set_ldac_esp32,
  .set_clr = set_clr_esp32,
};

dax0004_status_e shift_sr_esp32(uint8_t* pdat, uint32_t len, void* arg){
  //if(arg == NULL){ return DAX0004_STAT_ERR; }
  static bool sr_initialized = false;
  dax_if_esp32_arg_t* if_args = (dax_if_esp32_arg_t*)arg;
  esp_err_t ret = ESP_OK;
  const uint8_t xfer_size = 4;    // DAX0004 requires 32-bit transfers
  spi_transaction_t trans = {     // Configure common transaction settings (also ensures that the transfer is zero-initialized in other entries)
    .length = xfer_size*8,      
  };
  if(!sr_initialized){
    spi_device_interface_config_t devcfg={
        .clock_speed_hz = if_args->clk_freq,
        .mode = 2,                                // SPI mode 2                 
        .spics_io_num = if_args->sync_pin,        // Let ESP32 SPI driver handle SYNC line
        .queue_size = if_args->spi_q_size,
    };
    spi_bus_add_device(if_args->host, &devcfg, &(if_args->spi));
    sr_initialized = true;
  }
  for(uint32_t ux = 0; ux < (len/xfer_size); ux++){                         // handle all full sized transactions from pdat
    trans.tx_buffer = (void*)(pdat + (ux*xfer_size));                       // offset tx_buffer for this transfer
    ret |= spi_device_queue_trans(if_args->spi, &trans, portMAX_DELAY);     // queue all transactions
  }

  return (ret == ESP_OK) ? DAX0004_STAT_OK : DAX0004_STAT_ERR;
}

dax0004_status_e set_sync_esp32(bool lvl, void* arg){
  // if(arg == NULL){ return DAX0004_STAT_ERR; }
  dax_if_esp32_arg_t* if_args = (dax_if_esp32_arg_t*)arg;
  static bool sync_initialized = false;
  if(!sync_initialized){
    gpio_pad_select_gpio(if_args->sync_pin);
    gpio_set_direction(if_args->sync_pin, GPIO_MODE_OUTPUT);
    sync_initialized = true;
  }
  gpio_set_level(if_args->sync_pin, lvl);
  return DAX0004_STAT_OK;
}

dax0004_status_e set_ldac_esp32(bool lvl, void* arg){
  // if(arg == NULL){ return DAX0004_STAT_ERR; }
  dax_if_esp32_arg_t* if_args = (dax_if_esp32_arg_t*)arg;
  static bool ldac_initialized = false;
  if(!ldac_initialized){
    gpio_pad_select_gpio(if_args->ldac_pin);
    gpio_set_direction(if_args->ldac_pin, GPIO_MODE_OUTPUT);
    ldac_initialized = true;
  }
  gpio_set_level(if_args->ldac_pin, lvl);
  return DAX0004_STAT_OK;
}

dax0004_status_e set_clr_esp32(bool lvl, void* arg){
  // if(arg == NULL){ return DAX0004_STAT_ERR; }
  dax_if_esp32_arg_t* if_args = (dax_if_esp32_arg_t*)arg;
  static bool clr_initialized = false;
  if(!clr_initialized){
    gpio_pad_select_gpio(if_args->clr_pin);
    gpio_set_direction(if_args->clr_pin, GPIO_MODE_OUTPUT);
    clr_initialized = true;
  }
  gpio_set_level(if_args->clr_pin, lvl);
  return DAX0004_STAT_OK;
}
