const tf = require('@tensorflow/tfjs-node');
const fs = require('fs');

// --- Configuration ---
const CSV_FILE = 'trainingData.csv';
const WINDOW_SIZE = 240; // How many rows of data per prediction
const STEP_SIZE = 12;   // Overlap windows by shifting 10 rows at a time (Data augmentation)
const NUM_CLASSES = 3;  // Standing, Walking, Jogging
const NUM_FEATURES = 3; // Accel X, Y, Z

async function run() {
  console.log("1. Loading and parsing CSV data...");
  const rawData = fs.readFileSync(CSV_FILE, 'utf8');
  const lines = rawData.trim().split('\n').slice(1); // Skip header row

  // Create separate buckets for each activity
  let classData = { 0: [], 1: [], 2: [] };

  // Parse lines into numbers and put them in their specific bucket
  lines.forEach(line => {
    const parts = line.split(',');
    if (parts.length >= 7) {
      const ax = parseFloat(parts[0]);
      const ay = parseFloat(parts[1]);
      const az = parseFloat(parts[2]);
      const label = parseInt(parts[6].trim()) - 1; // 0=Standing, 1=Walking, 2=Jogging

      // Only push if the data is valid and the label is 0, 1, or 2
      if (!isNaN(ax) && !isNaN(ay) && !isNaN(az) && classData[label]) {
        classData[label].push([ax, ay, az]);
      }
    }
  });

  console.log("2. Grouping data into pure rolling windows...");
  let windows = [];
  let windowLabels = [];

  // Slide the window across each class independently!
  for (let classIndex = 0; classIndex < NUM_CLASSES; classIndex++) {
    const features = classData[classIndex];
    console.log(`- Class ${classIndex} has ${features.length} raw data points.`);

    for (let i = 0; i <= features.length - WINDOW_SIZE; i += STEP_SIZE) {
      const window = features.slice(i, i + WINDOW_SIZE);
      
      windows.push(window);
      windowLabels.push(classIndex); // We know with 100% certainty it is this class
    }
  }

  console.log(`Created ${windows.length} pure training windows.`);

  // Convert JavaScript arrays to TensorFlow Tensors
  const xTrain = tf.tensor3d(windows, [windows.length, WINDOW_SIZE, NUM_FEATURES]);
  const yTrain = tf.oneHot(tf.tensor1d(windowLabels, 'int32'), NUM_CLASSES);

  console.log("3. Building the Neural Network...");
  const model = tf.sequential();

  // 1D Convolutional Layer looks for wave patterns over time
  model.add(tf.layers.conv1d({
    filters: 16,
    kernelSize: 3,
    activation: 'relu',
    inputShape: [WINDOW_SIZE, NUM_FEATURES]
  }));
  model.add(tf.layers.maxPooling1d({ poolSize: 2 }));
  
  model.add(tf.layers.flatten());
  
  model.add(tf.layers.dense({ units: 16, activation: 'relu' }));
  model.add(tf.layers.dense({ units: NUM_CLASSES, activation: 'softmax' }));

  model.compile({
    optimizer: 'adam',
    loss: 'categoricalCrossentropy',
    metrics: ['accuracy']
  });

  model.summary();

  console.log("4. Starting Training...");
  await model.fit(xTrain, yTrain, {
    epochs: 100,         
    batchSize: 32,      
    shuffle: true,      
    validationSplit: 0.2
  });

  console.log("5. Saving Model...");
  // Saving the model
  await model.save(tf.io.withSaveHandler(async (artifacts) => {
    // 1. Save the actual learned math (weights)
    fs.writeFileSync('weights.bin', Buffer.from(artifacts.weightData));

    // 2. Format the architecture file (model.json) so TF.js knows how to read it later
    const modelJSON = {
      modelTopology: artifacts.modelTopology,
      weightsManifest: [{
        paths: ['weights.bin'],
        weights: artifacts.weightSpecs
      }]
    };

    // 3. Save the architecture file
    fs.writeFileSync('model.json', JSON.stringify(modelJSON));

    return {
      modelArtifactsInfo: { dateSaved: new Date(), modelTopologyType: 'JSON' }
    };
  }));
  console.log("Training complete. Saving model.");
}

run();