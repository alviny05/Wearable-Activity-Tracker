# Justin Nascimento and Alvin Yan

import paho.mqtt.client as mqtt
import csv

CSVFile = 'testData.csv'

# Callback for when the client receives a CONNACK response from the server
def on_connect(client, userdata, flags, rc):
    print(f"Connected with result code {rc}")
    client.subscribe("esp32/group4/mocap")

# Callback for when a PUBLISH message is received
def on_message(client, userdata, msg):
    #print(f"Received `{msg.payload.decode()}` on topic `{msg.topic}`")

    # Store message into CSV File
    with open(CSVFile, 'a', newline='') as f:
        writer = csv.writer(f)
        writer.writerow(msg.payload.decode().split(','))

client = mqtt.Client(client_id= "EC444_Group4_Laptop")
client.on_connect = on_connect
client.on_message = on_message

# Connect to a public broker
client.connect("rasticvm.internal", 1883, 60)

# Initialize the CSV file
csvHeaders = ['Accel X', 'Accel Y', 'Accel Z', 'MoCap X', 'Mocap Y', 'Mocap Z']

with open(CSVFile, 'w', newline='') as f:
    writer = csv.writer(f)
    writer.writerow(csvHeaders)

# Start the network loop
client.loop_forever()