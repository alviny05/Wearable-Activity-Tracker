# Code Readme

## File Structure

### Within the ```Q3visuals``` Folder:

The visualization portion of the project consists of a few primary components: a Node.js data processing backend server, a jsDelivr data visualizer, and the TensorFlow ML model. These components work together to provide real-time motion detection and display to a dashboard.

```server.js```: This file acts as the central hub for data processing and machine learning operations. It initializes a WebSocket server to receive accelerometer data, pass this data through a TensorFlow ML classifier, and output the most likely activity being performed.

```train.js```: This file takes the dataset collected from RASTIC and trains the neuro network (see the main readme for more information).

```trainingData.csv```: The RASTIC dataset.

```training.txt```: This is an example of the output from one TensorFlow training session.

```model.json```: The "skeleton" of the neuro network.

```weights.bin```: Node weight definitions of the neuro network.

#### The ```public``` Sub-Folder:

```app.js```: This file contains the frontend logic that displays the following: triple-axis accelerometer data; ML activity classification; ML confidence.

```index.html```: Defines the user interface and dashboard layout. Includes in-line styling to improve appearance.

### The ```ESP32 Firmware``` Folder:

The ESP hardware portion of this project only needed to perform one very simple function: collect data from the ADXL343 accelerometer and send said data via UDP to the backend Node.js server.

```ADXL343.c and ADXL343.h```: Contains the global variables, definitions, libraries, and functions needed to initialize, calibrate, filter, and collect data from the accelerometer.

```i2c_functions.c and i2c_functions.h```: Contains the global variables, definitions, libraries, and functions needed to initialize and communicate over I2C.

```timer_interrupt.c and timer_interrupt.h```: Contains the global variables, definitions, libraries, and functions needed to initialize the timers and their hardware interrupts.

```WiFi.c``` and ```WiFi.h```: Contains the global variables, definitions, libraries, and functions needed to initialize and connect to the defined WiFi network.

```main.c```: This file uses all the files mentioned above as libraries to initialize I2C communication, to initialize the accelerometer, to initialize a timer, and spin an RTOS task to send accelerometer data via the UDP protocol over the WiFi network to the Node.js backend server.

### The ```RASTIC Data Collection``` Folder:
This folder contains all the files used to collect the dataset used to train the machine learning classifier through utilizing an accelerometer and RASTIC's Motion Capture Setup.

```app_main.c```: Contains the logic to subscribe to the topic that RASTIC's Motion Capture setup publishes to over MQTT, collect data from our accelerometer through an RTOS task, and publish the combined data to a topic on RASTIC's MQTT network.

```i2c_functions.c and i2c_functions.h```: Contain the global variables, definitions, libraries, and functions needed to initialize and communicate over I2C.

```ADXL343.c and ADXL343.h```: Contain the global variables, definitions, libraries, and functions needed to initialize, calibrate, filter, and collect data from the accelerometer.

```WiFi.c and WiFi.h```: Contain the global variables, definitions, libraries, and functions needed to initialize and connect to WiFi, a prerequisite of connecting to RASTIC's MQTT broker.

```jsmn.h```: Library for processing JSON objects into C data types, which is needed as the Motion Capture setup publishes data in a JSON format.

```read_mqtt_data.py```: Python script ran by our laptop to subscribe to the MQTT topic published by our ESP32 and write the data to the CSV file.

```trainingData.csv```: The RASTIC dataset.

## AI Declaration
- ChatGPT and Gemini was used mostly for bug fixes and setting up the TensorFlow classifier model.
