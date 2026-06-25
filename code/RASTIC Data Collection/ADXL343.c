/*
This file contains the code for initializing and collecting data from the ADXL343 Accelerometer
Sensor through I2C Communication.
Justin Nascimento (U42983905) and Alvin Yan ()
*/

#include "ADXL343.h"

// Static Variables for Distance Calculation
float dt = TIMER_INTERVAL_SEC * 5;
struct distanceVals prevDistance1;
struct velocityVals prevVelocity1;
struct velocityVals prevVelocity2;

// Variables for acceleration offset
float xAccelOffset;
float yAccelOffset;
float zAccelOffset;


// ADXL343 Functions ///////////////////////////////////////////////////////////
// Get Device ID
static int getDeviceID(uint8_t *data) {
  int ret;
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, ( ADXL343_ADDRESS << 1 ) | WRITE_BIT, ACK_CHECK_EN);
  i2c_master_write_byte(cmd, ADXL343_REG_DEVID, ACK_CHECK_EN);
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, ( ADXL343_ADDRESS << 1 ) | READ_BIT, ACK_CHECK_EN);
  i2c_master_read_byte(cmd, data, ACK_CHECK_DIS);
  i2c_master_stop(cmd);
  ret = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
  i2c_cmd_link_delete(cmd);
  return ret;
}

// Write one byte to register
static int writeRegister(uint8_t reg, uint8_t data) {
    int ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);

    i2c_master_write_byte(cmd, (ADXL343_ADDRESS << 1 ) | WRITE_BIT, ACK_CHECK_EN);     // Indicate we're writing
    i2c_master_write_byte(cmd, reg, ACK_CHECK_EN);                                    // Sends start address
    i2c_master_write_byte(cmd, data, ACK_CHECK_EN);                                // Writes byte to the register
    i2c_master_stop(cmd);

    ret = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

// Read register
static uint8_t readRegister(uint8_t reg) {
    // Defines return value
    uint8_t value;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    // Tell the device which register you'd like to read from
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ADXL343_ADDRESS << 1) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg, ACK_CHECK_EN);

    // Tactical restart to now tell the device we are reading from it
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ADXL343_ADDRESS << 1) | READ_BIT, ACK_CHECK_EN);
    i2c_master_read_byte(cmd, &value, NACK_VAL);

    i2c_master_stop(cmd);

    i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

    return value;
}

// read 16 bits (2 bytes)
static int16_t read16(uint8_t reg) {
    // Read values
    uint8_t lowerByte;
    uint8_t higherByte;
    uint16_t value;


    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);

    // Indicates we intend to read from reg
    i2c_master_write_byte(cmd, (ADXL343_ADDRESS << 1) | WRITE_BIT, ACK_CHECK_EN);       // Tell it we're writing to 0x10
    i2c_master_write_byte(cmd, reg, ACK_CHECK_EN);                                // Write 0x10

    // Tactical restart to now indicate we're reading
    i2c_master_start(cmd);  // Repeated start, to indicate we're reading
    i2c_master_write_byte(cmd, (ADXL343_ADDRESS << 1) | READ_BIT, ACK_CHECK_EN);
    i2c_master_read_byte(cmd, &lowerByte, ACK_VAL);
    i2c_master_read_byte(cmd, &higherByte, NACK_VAL);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

    // Format the value
    value = (higherByte * 256) + lowerByte;

    return value;
}

static void setRange(range_t range) {
  /* Red the data format register to preserve bits */
  uint8_t format = readRegister(ADXL343_REG_DATA_FORMAT);

  /* Update the data rate */
  format &= ~0x0F;
  format |= range;

  /* Make sure that the FULL-RES bit is enabled for range scaling */
  format |= 0x08;

  /* Write the register back to the IC */
  writeRegister(ADXL343_REG_DATA_FORMAT, format);

}

static range_t getRange(void) {
  /* Red the data format register to preserve bits */
  return (range_t)(readRegister(ADXL343_REG_DATA_FORMAT) & 0x03);
}

static dataRate_t getDataRate(void) {
  return (dataRate_t)(readRegister(ADXL343_REG_BW_RATE) & 0x0F);
}


// Global Funcitons
// Function to initialize the accelerometer
void initializeADXL343(void){
  // Set PreviousDistance and previousVelocity values to zero
  prevDistance1.distX = 0;
  prevDistance1.distY = 0;
  prevDistance1.distZ = 0;

  prevVelocity1.velX = 0;
  prevVelocity1.velY = 0;
  prevVelocity1.velZ = 0;

  prevVelocity2.velX = 0;
  prevVelocity2.velY = 0;
  prevVelocity2.velZ = 0;

  // Check for ADXL343
  uint8_t deviceID;
  getDeviceID(&deviceID);
  if (deviceID == 0xE5) {
    printf("\n>> Found ADAXL343\n");
  }

  // Disable interrupts
  writeRegister(ADXL343_REG_INT_ENABLE, 0);

  // Set range
  setRange(ADXL343_RANGE_16_G);
  // Display range
  printf  ("- Range:         +/- ");
  switch(getRange()) {
    case ADXL343_RANGE_16_G:
      printf  ("16 ");
      break;
    case ADXL343_RANGE_8_G:
      printf  ("8 ");
      break;
    case ADXL343_RANGE_4_G:
      printf  ("4 ");
      break;
    case ADXL343_RANGE_2_G:
      printf  ("2 ");
      break;
    default:
      printf  ("?? ");
      break;
  }
  printf(" g\n");

  // Display data rate
  printf ("- Data Rate:    ");
  switch(getDataRate()) {
    case ADXL343_DATARATE_3200_HZ:
      printf  ("3200 ");
      break;
    case ADXL343_DATARATE_1600_HZ:
      printf  ("1600 ");
      break;
    case ADXL343_DATARATE_800_HZ:
      printf  ("800 ");
      break;
    case ADXL343_DATARATE_400_HZ:
      printf  ("400 ");
      break;
    case ADXL343_DATARATE_200_HZ:
      printf  ("200 ");
      break;
    case ADXL343_DATARATE_100_HZ:
      printf  ("100 ");
      break;
    case ADXL343_DATARATE_50_HZ:
      printf  ("50 ");
      break;
    case ADXL343_DATARATE_25_HZ:
      printf  ("25 ");
      break;
    case ADXL343_DATARATE_12_5_HZ:
      printf  ("12.5 ");
      break;
    case ADXL343_DATARATE_6_25HZ:
      printf  ("6.25 ");
      break;
    case ADXL343_DATARATE_3_13_HZ:
      printf  ("3.13 ");
      break;
    case ADXL343_DATARATE_1_56_HZ:
      printf  ("1.56 ");
      break;
    case ADXL343_DATARATE_0_78_HZ:
      printf  ("0.78 ");
      break;
    case ADXL343_DATARATE_0_39_HZ:
      printf  ("0.39 ");
      break;
    case ADXL343_DATARATE_0_20_HZ:
      printf  ("0.20 ");
      break;
    case ADXL343_DATARATE_0_10_HZ:
      printf  ("0.10 ");
      break;
    default:
      printf  ("???? ");
      break;
  }
  printf(" Hz\n\n");

  // Enable measurements
  writeRegister(ADXL343_REG_POWER_CTL, 0x08);

  // Calibrate the sensor, recording stationary offsets
  struct accelerationVals acceleration;

  for(int i=0;i<1000;i++){

    // Don't use getAccel function here so offsets do not stack up
    acceleration.accelX = read16(ADXL343_REG_DATAX0) * ADXL343_MG2G_MULTIPLIER * SENSORS_GRAVITY_STANDARD;
    acceleration.accelY = read16(ADXL343_REG_DATAY0) * ADXL343_MG2G_MULTIPLIER * SENSORS_GRAVITY_STANDARD;
    acceleration.accelZ = read16(ADXL343_REG_DATAZ0) * ADXL343_MG2G_MULTIPLIER * SENSORS_GRAVITY_STANDARD;

    xAccelOffset += acceleration.accelX;
    yAccelOffset += acceleration.accelY;
    zAccelOffset += acceleration.accelZ;
  }

  xAccelOffset /= 1000;
  yAccelOffset /= 1000;
  zAccelOffset /= 1000;
}


// Function to collect data from the Accelerometer
struct accelerationVals getAccel(){
  // Define acceleration struct
  struct accelerationVals acceleration;

  // Obtain values
  acceleration.accelX = read16(ADXL343_REG_DATAX0) * ADXL343_MG2G_MULTIPLIER * SENSORS_GRAVITY_STANDARD;
  acceleration.accelY = read16(ADXL343_REG_DATAY0) * ADXL343_MG2G_MULTIPLIER * SENSORS_GRAVITY_STANDARD;
  acceleration.accelZ = read16(ADXL343_REG_DATAZ0) * ADXL343_MG2G_MULTIPLIER * SENSORS_GRAVITY_STANDARD;

  // Subtract the offsets
  acceleration.accelX -= xAccelOffset;
  acceleration.accelY -= yAccelOffset;
  acceleration.accelZ -= zAccelOffset;

  // Return struct
  return acceleration;
}

// Function to calculate roll and pitch from acceleration
void calcRP(struct accelerationVals acceleration, float *roll, float *pitch){
    // Roll and Pitch formula
    *roll = atan2(acceleration.accelY, acceleration.accelZ);
    *pitch = atan2(-acceleration.accelX, sqrt(pow(acceleration.accelY,2) + pow(acceleration.accelZ,2)));

    // Convert Roll and Pitch to degrees
    *roll  *= 180/M_PI;
    *pitch *= 180/M_PI;

    printf("roll: %.2f \t pitch: %.2f \n", *roll, *pitch);
}

// Uses double integration to calculate distance from acceleration
struct distanceVals doubleIntegrationDistance(struct accelerationVals acceleration){
  // Initialize distanceValues and velocityVals
  struct distanceVals distance;
  struct velocityVals velocity;

  // Calculates velocity
  velocity.velX = prevVelocity1.velX + (acceleration.accelX * dt);
  velocity.velY = prevVelocity1.velY + (acceleration.accelY * dt);
  velocity.velZ = prevVelocity1.velZ + (acceleration.accelZ * dt);

  // Performs Temporal Filtering on Velocity with an LPF
  velocity.velX = (temporalThreshold * velocity.velX) + ((1-temporalThreshold) * prevVelocity1.velX);
  velocity.velY = (temporalThreshold * velocity.velY) + ((1-temporalThreshold) * prevVelocity1.velY);
  velocity.velZ = (temporalThreshold * velocity.velZ) + ((1-temporalThreshold) * prevVelocity1.velZ);

  // Filter the velocity
  velocityThresholdFilter(&velocity);

  // Calculates Distance
  distance.distX = prevDistance1.distX + (prevVelocity1.velX * dt) + (0.5 * acceleration.accelX * pow(dt,2));
  distance.distY = prevDistance1.distY + (prevVelocity1.velY * dt) + (0.5 * acceleration.accelY * pow(dt,2));
  distance.distZ = prevDistance1.distZ + (prevVelocity1.velZ * dt) + (0.5 * acceleration.accelZ * pow(dt,2));

  // Sets previous values to current values
  prevDistance1 = distance;
  prevVelocity1 = velocity;

  // Returns
  return distance;
}

// Uses single integration to calculate velocity from acceleration
struct velocityVals singleIntegrationVelocity(struct accelerationVals acceleration){
  // Initialize distanceValues and velocityVals
  struct velocityVals velocity;

  // Calculates velocity
  velocity.velX = prevVelocity2.velX + (acceleration.accelX * dt);
  velocity.velY = prevVelocity2.velY + (acceleration.accelY * dt);
  velocity.velZ = prevVelocity2.velZ + (acceleration.accelZ * dt);

  // Performs Temporal Filtering on Velocity with an LPF
  velocity.velX = (temporalThreshold * velocity.velX) + ((1-temporalThreshold) * prevVelocity1.velX);
  velocity.velY = (temporalThreshold * velocity.velY) + ((1-temporalThreshold) * prevVelocity1.velY);
  velocity.velZ = (temporalThreshold * velocity.velZ) + ((1-temporalThreshold) * prevVelocity1.velZ);

  // Filter the velocity
  velocityThresholdFilter(&velocity);

  // Sets previous values to current values
  prevVelocity2 = velocity;

  // Returns
  return velocity;
}

// Threshold filter
// If the acceleration is below a threshold, set it to zero
void accelerationThresholdFilter(struct accelerationVals *acceleration){
  if(fabs(acceleration->accelX) < accelerationThreshold){
      acceleration->accelX = 0;
  }
  if(fabs(acceleration->accelY) < accelerationThreshold){
      acceleration->accelY = 0;
  }
  if(fabs(acceleration->accelZ) < accelerationThreshold){
      acceleration->accelZ = 0;
  }
}

void velocityThresholdFilter(struct velocityVals *velocity){
  if(fabs(velocity->velX) < VelocityThreshold){
      velocity->velX = 0;
  }
  if(fabs(velocity->velY) < VelocityThreshold){
      velocity->velY = 0;
  }
  if(fabs(velocity->velZ) < VelocityThreshold){
      velocity->velZ = 0;
  }
}


// TODO
// ADD FILTER SIGNAL IN SOFTWARE -> LOW PASS FILTER
// SET OFFSET IN THE ACCELEROMETER