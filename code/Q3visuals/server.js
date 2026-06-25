const express = require('express');
const http = require('http');
const { Server } = require('socket.io');
const dgram = require('dgram');
const tf = require('@tensorflow/tfjs'); // AI engine
const fs = require('fs');               // File system for loading model

const app = express();
const server = http.createServer(app);
const io = new Server(server);

// Ports and IP Setup
const WEB_PORT = 8080;
const UDP_PORT = 3333;
const UDP_HOST = '192.168.1.119'; // Ensure this matches your computer's IP

app.use(express.static('public'));

// Struct to hold all of the IPs
let User_IPs = [];

// --- 1. AI MODEL SETUP ---
let model1;
let model2;

// Add this custom loader block
const modelLoader = {
    load: async () => {
        const modelJSON = JSON.parse(fs.readFileSync('./model.json', 'utf8'));
        const weightBuffer = fs.readFileSync('./weights.bin');
        const weightData = new Uint8Array(weightBuffer).buffer;
        return {
            modelTopology: modelJSON.modelTopology,
            weightSpecs: modelJSON.weightsManifest[0].weights,
            weightData: weightData
        };
    }
};

// Update your loadAI function to use it
async function loadAI() {
    console.log("Loading AI Model (Pure JS)...");
    model1 = await tf.loadLayersModel(modelLoader);
    model2 = await tf.loadLayersModel(modelLoader);
    console.log("Model loaded successfully!");
}


// --- 2. THE ROLLING BUFFER ---
const WINDOW_SIZE = 240;
let rollingWindow1 = [];
let rollingWindow2 = [];
let tickCount1 = 0;
let tickCount2 = 0;

// Persistent state so the frontend doesn't blink between predictions
let currentState1 = undefined;
let currentConfidence1 = 0;
let currentState2 = undefined;
let currentConfidence2 = 0;


// --- 3. UDP Server Setup (Receiving from ESP32) ---
const udpServer = dgram.createSocket('udp4');

udpServer.on('listening', function () {
    const address = udpServer.address();
    console.log(`UDP Server listening for hardware on ${address.address}:${address.port}`);
});

udpServer.on('message', function (message, remote) {
    // Send Ok acknowledgment back to the ESP32
    udpServer.send("Ok!", remote.port, remote.address, function(error){
        if (error) console.error('Error sending Ack!');
    });

    // Add IP to the list if not in there
    if (!User_IPs.includes(remote.address)) {
        User_IPs.push(remote.address);
    }

    // Pull the device number from the IP
    index = User_IPs.indexOf(remote.address);

    try {
        const dataString = message.toString().trim();
        const values = dataString.split(' ').map(Number);
        // If device 1 sent it, send device 1 model:
        if(index == 0){
            if (values.length >= 3) {
            const ax = values[0];
            const ay = values[1];
            const az = values[2];

            // A. Push the newest reading onto the conveyor belt
            rollingWindow1.push([ax, ay, az]);

            // B. If the belt has more than 240 items, drop the oldest one
            if (rollingWindow1.length > WINDOW_SIZE) {
                rollingWindow1.shift();
            }

            // C. Once the belt is full, start predicting!
            if (rollingWindow1.length === WINDOW_SIZE && model1) {
                tickCount1++;

                if (tickCount1 % 12 === 0) {
                    const inputTensor = tf.tensor3d([rollingWindow1], [1, WINDOW_SIZE, 3]);
                    const predictionTensor = model1.predict(inputTensor);
                    const probs = predictionTensor.arraySync()[0];

                    inputTensor.dispose();
                    predictionTensor.dispose();

                    const winningIndex = probs.indexOf(Math.max(...probs));

                    currentState1 = winningIndex + 1;
                    // C. Send the raw decimal (e.g., 0.852) instead of a string
                    currentConfidence1 = probs[winningIndex];
                }
            }

            // D. Push the combined live data + AI state to the Web Frontend
            io.emit('sensor_data', {
                ax: ax,
                ay: ay,
                az: az,
                state: currentState1,
                confidence: currentConfidence1,
                idx: index
            });
            }
        }
        // Else, we are in index 2, so send that data
        else{
            if (values.length >= 3) {
            const ax = values[0];
            const ay = values[1];
            const az = values[2];

            // A. Push the newest reading onto the conveyor belt
            rollingWindow2.push([ax, ay, az]);

            // B. If the belt has more than 240 items, drop the oldest one
            if (rollingWindow2.length > WINDOW_SIZE) {
                rollingWindow2.shift();
            }

            // C. Once the belt is full, start predicting!
            if (rollingWindow2.length === WINDOW_SIZE && model2) {
                tickCount2++;

                if (tickCount2 % 12 === 0) {
                    const inputTensor = tf.tensor3d([rollingWindow2], [1, WINDOW_SIZE, 3]);
                    const predictionTensor = model2.predict(inputTensor);
                    const probs = predictionTensor.arraySync()[0];

                    inputTensor.dispose();
                    predictionTensor.dispose();

                    const winningIndex = probs.indexOf(Math.max(...probs));

                    currentState2 = winningIndex + 1;
                    // C. Send the raw decimal (e.g., 0.852) instead of a string
                    currentConfidence2 = probs[winningIndex];
                }
            }

            // D. Push the combined live data + AI state to the Web Frontend
            io.emit('sensor_data', {
                ax: ax,
                ay: ay,
                az: az,
                state: currentState2,
                confidence: currentConfidence2,
                idx: index
            });
            }
        }

    } catch (err) {
        console.error("Error parsing UDP packet:", err);
    }
});

udpServer.bind(UDP_PORT, UDP_HOST);

// --- 4. WebSocket Server Setup (Sending to Frontend) ---
io.on('connection', (socket) => {
    console.log('Web dashboard connected.');
    socket.on('disconnect', () => {
        console.log('Web dashboard disconnected.');
    });
});

loadAI().then(() => {
    server.listen(WEB_PORT, () => {
        console.log(`Live at http://localhost:${WEB_PORT}`);
    });
});
