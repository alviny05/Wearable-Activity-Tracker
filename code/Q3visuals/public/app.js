// app.js
const socket = io();

// --- 1. UI Elements & State Mapping ---
const stateDisplay1 = document.getElementById('predictionState1');
const confidenceDisplay1 = document.getElementById('confidenceValue1');
const stateDisplay2 = document.getElementById('predictionState2');
const confidenceDisplay2 = document.getElementById('confidenceValue2');

// Map the numerical states to text labels
const stateMap = {
    1: 'Standing',
    2: 'Walking',
    3: 'Jogging'
};

// --- 2. Chart.js Setup ---
const MAX_POINTS = 240; // 2 seconds of rolling data at 120Hz
const ctx1 = document.getElementById('accelChart1').getContext('2d');
const ctx2 = document.getElementById('accelChart2').getContext('2d');

const chart1 = new Chart(ctx1, {
    type: 'line',
    data: {
        labels: Array(MAX_POINTS).fill(''),
        datasets: [
            { label: 'Accel X', borderColor: '#ff4444', data: Array(MAX_POINTS).fill(0), tension: 0.1, pointRadius: 0, borderWidth: 1.5 },
            { label: 'Accel Y', borderColor: '#00ff88', data: Array(MAX_POINTS).fill(0), tension: 0.1, pointRadius: 0, borderWidth: 1.5 },
            { label: 'Accel Z', borderColor: '#00d2ff', data: Array(MAX_POINTS).fill(0), tension: 0.1, pointRadius: 0, borderWidth: 1.5 }
        ]
    },
    options: {
        responsive: true,
        maintainAspectRatio: false, // Allows chart to fill the flex container
        animation: false,           // Essential for high-frequency live data
        scales: { y: { suggestedMin: -5, suggestedMax: 5 } },
        plugins: { legend: { labels: { color: 'white' } } }
    }
});

const chart2 = new Chart(ctx2, {
    type: 'line',
    data: {
        labels: Array(MAX_POINTS).fill(''),
        datasets: [
            { label: 'Accel X', borderColor: '#ff4444', data: Array(MAX_POINTS).fill(0), tension: 0.1, pointRadius: 0, borderWidth: 1.5 },
            { label: 'Accel Y', borderColor: '#00ff88', data: Array(MAX_POINTS).fill(0), tension: 0.1, pointRadius: 0, borderWidth: 1.5 },
            { label: 'Accel Z', borderColor: '#00d2ff', data: Array(MAX_POINTS).fill(0), tension: 0.1, pointRadius: 0, borderWidth: 1.5 }
        ]
    },
    options: {
        responsive: true,
        maintainAspectRatio: false, // Allows chart to fill the flex container
        animation: false,           // Essential for high-frequency live data
        scales: { y: { suggestedMin: -5, suggestedMax: 5 } },
        plugins: { legend: { labels: { color: 'white' } } }
    }
});


// --- 3. Receive Streamed Data ---
let updateCounter1 = 0;
let updateCounter2 = 0;

socket.on('sensor_data', (data) => {
    // If device 1, update the top half
    if(data.idx == 0){
        // A. Update Accelerometer Chart
        chart1.data.datasets[0].data.push(data.ax);
        chart1.data.datasets[1].data.push(data.ay);
        chart1.data.datasets[2].data.push(data.az);

        // Maintain rolling window length
        chart1.data.datasets.forEach(dataset => dataset.data.shift());

        // Throttle chart rendering to ~60 FPS while keeping all 120Hz data points
        updateCounter1++;
        if (updateCounter1 % 2 === 0) {
            chart1.update();
        }

        // B. Update Prediction & Confidence (If provided by backend)
        // This expects the server to eventually send: { ax, ay, az, state, confidence }
        if (data.state !== undefined) {
            const stateText = stateMap[data.state] || 'Unknown';
            stateDisplay1.innerText = stateText;

            // Dynamic color coding based on state
            if (data.state === 1) stateDisplay1.style.color = '#aaaaaa'; // Grey for standing
            else if (data.state === 2) stateDisplay1.style.color = '#00ff88'; // Green for walking
            else if (data.state === 3) stateDisplay1.style.color = '#ff4444'; // Red for jogging
        }

        if (data.confidence !== undefined && data.confidence > 0) {
            // This will now correctly turn 0.852 into "85.2%"
            const confidencePercent = (data.confidence * 100).toFixed(1);
            confidenceDisplay1.innerText = `${confidencePercent}%`;
        } else {
            confidenceDisplay1.innerText = "Awaiting AI...";
        }
    }
    // If device 2, update the bottom half
    else{
        // A. Update Accelerometer Chart
        chart2.data.datasets[0].data.push(data.ax);
        chart2.data.datasets[1].data.push(data.ay);
        chart2.data.datasets[2].data.push(data.az);

        // Maintain rolling window length
        chart2.data.datasets.forEach(dataset => dataset.data.shift());

        // Throttle chart rendering to ~60 FPS while keeping all 120Hz data points
        updateCounter2++;
        if (updateCounter2 % 2 === 0) {
            chart2.update();
        }

        // B. Update Prediction & Confidence (If provided by backend)
        // This expects the server to eventually send: { ax, ay, az, state, confidence }
        if (data.state !== undefined) {
            const stateText = stateMap[data.state] || 'Unknown';
            stateDisplay2.innerText = stateText;

            // Dynamic color coding based on state
            if (data.state === 1) stateDisplay2.style.color = '#aaaaaa'; // Grey for standing
            else if (data.state === 2) stateDisplay2.style.color = '#00ff88'; // Green for walking
            else if (data.state === 3) stateDisplay2.style.color = '#ff4444'; // Red for jogging
        }

        if (data.confidence !== undefined && data.confidence > 0) {
            // This will now correctly turn 0.852 into "85.2%"
            const confidencePercent = (data.confidence * 100).toFixed(1);
            confidenceDisplay2.innerText = `${confidencePercent}%`;
        } else {
            confidenceDisplay2.innerText = "Awaiting AI...";
        }
    }

});