/*
This file contains the code for Initializing the I2C Communication Bus
Justin Nascimento (U42983905) and Alvin Yan ()
*/

#ifndef I2C_FUNCTIONS_H
#define I2C_FUNCTIONS_H

// Include Libraries
#include <stdio.h>
#include "driver/i2c.h"

// Definitons
// Master I2C
#define I2C_EXAMPLE_MASTER_SCL_IO          22   // gpio number for i2c clk
#define I2C_EXAMPLE_MASTER_SDA_IO          23   // gpio number for i2c data
#define I2C_EXAMPLE_MASTER_NUM             I2C_NUM_0  // i2c port
#define I2C_EXAMPLE_MASTER_TX_BUF_DISABLE  0    // i2c master no buffer needed
#define I2C_EXAMPLE_MASTER_RX_BUF_DISABLE  0    // i2c master no buffer needed
#define I2C_EXAMPLE_MASTER_FREQ_HZ         100000     // i2c master clock freq
#define WRITE_BIT                          I2C_MASTER_WRITE // i2c master write
#define READ_BIT                           I2C_MASTER_READ  // i2c master read
#define ACK_CHECK_EN                       true // i2c master will check ack
#define ACK_CHECK_DIS                      false// i2c master will not check ack
#define ACK_VAL                            0x00 // i2c ack value
#define NACK_VAL                           0x01 // i2c nack value (Was FF)


// Function Prototypes
void i2c_scanner(void);
void i2c_master_init(void);

#endif