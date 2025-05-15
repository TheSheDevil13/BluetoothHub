from flask import Flask, request, render_template, jsonify
from datetime import datetime

app = Flask(__name__)

# Store all temperature readings
temperature_log = []

@app.route('/')
def dashboard():
    return render_template('dashboard.html', data=temperature_log)

@app.route('/data', methods=['POST'])
def receive_data():
    global temperature_log
    data = request.get_json()

    if data and 'temperature' in data:
        entry = {
            "temperature": data['temperature'],
            "timestamp": datetime.now().strftime('%Y-%m-%d %H:%M:%S')
        }
        temperature_log.append(entry)
        print(f"Received: {entry}")
        return "Data received", 200
    else:
        return "Invalid data", 400

@app.route('/data', methods=['GET'])
def get_data():
    return jsonify(temperature_log)

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
