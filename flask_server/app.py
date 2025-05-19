from flask import Flask, request, render_template, jsonify
from datetime import datetime

app = Flask(__name__)

# Store sensor readings with separate lists for each device and sensor type
sensor_data = {
    "transmitter1": {
        "temperature": [],
        "humidity": []
    },
    "transmitter2": {
        "temperature": [],
        "humidity": []
    }
}

@app.route('/')
def dashboard():
    return render_template('dashboard.html', data=sensor_data)

@app.route('/data', methods=['POST'])
def receive_data():
    global sensor_data
    data = request.get_json()

    if data and 'device' in data and 'type' in data and 'value' in data:
        device = data['device']
        sensor_type = data['type']
        value = data['value']
        timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')

        # Create entry
        entry = {
            "value": value,
            "timestamp": timestamp
        }

        # Add to appropriate list
        if device in sensor_data and sensor_type in sensor_data[device]:
            sensor_data[device][sensor_type].append(entry)
            # Keep only last 100 readings
            if len(sensor_data[device][sensor_type]) > 100:
                sensor_data[device][sensor_type].pop(0)
            
            print(f"Received: {device} - {sensor_type}: {value}")
            return "Data received", 200
    
    return "Invalid data", 400

@app.route('/data', methods=['GET'])
def get_data():
    return jsonify(sensor_data)

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
