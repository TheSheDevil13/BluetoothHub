from flask import Flask, request, jsonify
import json
from datetime import datetime
import os

app = Flask(__name__)

# Create data directory if it doesn't exist
data_dir = 'temperature_data'
if not os.path.exists(data_dir):
    os.makedirs(data_dir)

# Store temperature data in memory for quick access
temperature_data = []

@app.route('/')
def home():
    """Display recent temperature data"""
    current_time = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    
    # Generate table rows
    rows = ""
    for entry in reversed(temperature_data[-20:]):  # Show last 20 entries
        rows += f"<tr><td>{entry['timestamp']}</td><td>{entry['device']}</td><td>{entry['temperature']} °C</td></tr>"
    
    # Create HTML with simple formatting
    html = f"""<!DOCTYPE html>
<html>
<head>
    <title>ESP32 Temperature Monitor</title>
    <meta http-equiv="refresh" content="10">
    <style>
        body {{
            font-family: Arial, sans-serif;
            margin: 40px;
        }}
        h1 {{
            color: #333;
        }}
        table {{
            border-collapse: collapse;
            width: 80%;
            margin-top: 20px;
        }}
        th, td {{
            border: 1px solid #ddd;
            padding: 8px;
            text-align: left;
        }}
        th {{
            background-color: #f2f2f2;
        }}
        tr:nth-child(even) {{
            background-color: #f9f9f9;
        }}
    </style>
</head>
<body>
    <h1>ESP32 Temperature Monitor</h1>
    <p>Last updated: {current_time}</p>
    <table>
        <tr>
            <th>Time</th>
            <th>Device</th>
            <th>Temperature</th>
        </tr>
        {rows}
    </table>
</body>
</html>"""
    
    return html

@app.route('/temperature', methods=['POST'])
def receive_temperature():
    """Receive temperature data from ESP32"""
    data = {}
    
    # Handle both JSON and form data
    if request.is_json:
        # Handle JSON data
        data = request.get_json()
    else:
        # Handle form data
        data = {
            'device': request.form.get('device'),
            'temperature': request.form.get('temperature')
        }
    
    print(f"Received data: {data}")
    
    # Validate data
    if not data.get('device') or not data.get('temperature'):
        return jsonify({"status": "error", "message": "Missing required fields"}), 400
    
    # Make sure temperature is a float
    try:
        data['temperature'] = float(data['temperature'])
    except ValueError:
        pass  # Keep as string if conversion fails
    
    # Add timestamp
    timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    data['timestamp'] = timestamp
    
    # Store in memory
    temperature_data.append(data)
    
    # Store in file
    filename = os.path.join(data_dir, f"temperature_log_{datetime.now().strftime('%Y%m%d')}.json")
    with open(filename, 'a') as f:
        f.write(json.dumps(data) + '\n')
    
    return jsonify({"status": "success"}), 200

@app.route('/api/data', methods=['GET'])
def get_data():
    """Return all data as JSON for API clients"""
    return jsonify(temperature_data)

if __name__ == '__main__':
    # Bind to all network interfaces on port 5000
    app.run(host='0.0.0.0', port=5000, debug=True) 