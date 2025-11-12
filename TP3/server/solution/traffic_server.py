from flask import Flask, request, jsonify
from datetime import datetime

app = Flask(__name__)

current_command = {
    "state": "off",
    "timestamp": datetime.now().isoformat(),
    "source": "system",
}


@app.route("/")
def home():
    html = f"""
    <html>
    <head><title>Traffic Light Server</title></head>
    <body>
        <h1>Traffic Light Control Server</h1>
        <h2>Current State: {current_command["state"].upper()}</h2>
        <p>Last updated: {current_command["timestamp"]}</p>
        <p>Source: {current_command["source"]}</p>
        <hr>
        <h3>API Endpoints:</h3>
        <ul>
            <li>POST /update - Update traffic light command</li>
            <li>GET /command - Get current command</li>
        </ul>
    </body>
    </html>
    """
    return html


@app.route("/update", methods=["POST"])
def update_command():
    global current_command

    try:
        data = request.get_json()

        if not data or "state" not in data:
            return jsonify({"error": "Missing 'state' field"}), 400

        valid_states = ["red", "yellow", "green", "off"]
        if data["state"].lower() not in valid_states:
            return jsonify(
                {"error": f"Invalid state. Must be one of: {valid_states}"}
            ), 400

        current_command = {
            "state": data["state"].lower(),
            "timestamp": datetime.now().isoformat(),
            "source": data.get("source", "unknown"),
        }

        print(
            f"[UPDATE] New command: {current_command['state']} from {current_command['source']}"
        )

        return jsonify({"success": True, "command": current_command}), 200

    except Exception as e:
        return jsonify({"error": str(e)}), 500


@app.route("/command", methods=["GET"])
def get_command():
    return jsonify(current_command), 200


if __name__ == "__main__":
    print("Starting Traffic Light Control Server...")
    print("Publisher POST to: http://localhost:5000/update")
    print("Subscriber GET from: http://localhost:5000/command")
    app.run(host="0.0.0.0", port=5000, debug=True)
