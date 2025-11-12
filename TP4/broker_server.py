from flask import Flask, request, jsonify
from datetime import datetime
from collections import defaultdict
import threading

app = Flask(__name__)

messages = defaultdict(list)
message_lock = threading.Lock()
MAX_MESSAGES_PER_TOPIC = 100


@app.route("/")
def home():
    with message_lock:
        topic_stats = {topic: len(msgs) for topic, msgs in messages.items()}

    html = """
    <html>
    <head><title>MQTT-Like Broker</title></head>
    <body>
        <h1>🔗 HTTP-Based MQTT Clone Broker</h1>
        <h2>Active Topics</h2>
    """

    if topic_stats:
        html += "<ul>"
        for topic, count in topic_stats.items():
            html += f"<li><strong>{topic}</strong>: {count} messages</li>"
        html += "</ul>"
    else:
        html += "<p>No messages yet</p>"

    html += """
        <hr>
        <h3>API Endpoints:</h3>
        <ul>
            <li><strong>POST /publish</strong> - Publish a message</li>
            <li><strong>GET /subscribe?topic=...</strong> - Get messages</li>
            <li><strong>DELETE /clear?topic=...</strong> - Clear topic</li>
        </ul>
    </body>
    </html>
    """
    return html


@app.route("/publish", methods=["POST"])
def publish():
    # TODO: Implement message publishing
    try:
        data = request.get_json()

        if not data or "topic" not in data or "payload" not in data:
            return jsonify({"error": "Missing 'topic' or 'payload'"}), 400

        topic = data["topic"]
        message = {
            "payload": data["payload"],
            "timestamp": datetime.now().isoformat(),
            "publisher": data.get("publisher", "anonymous"),
        }

        # TODO: Store message with thread safety

        return jsonify({"success": True}), 200
    except Exception as e:
        return jsonify({"error": str(e)}), 500


@app.route("/subscribe", methods=["GET"])
def subscribe():
    # TODO: Implement message subscription
    try:
        topic = request.args.get("topic")
        since = request.args.get("since")

        if not topic:
            return jsonify({"error": "Missing 'topic'"}), 400

        # TODO: Retrieve and filter messages

        return jsonify({"topic": topic, "messages": []}), 200
    except Exception as e:
        return jsonify({"error": str(e)}), 500


@app.route("/clear", methods=["DELETE"])
def clear_topic():
    topic = request.args.get("topic")
    if not topic:
        return jsonify({"error": "Missing 'topic'"}), 400

    with message_lock:
        if topic in messages:
            count = len(messages[topic])
            del messages[topic]
            return jsonify({"success": True, "cleared": count}), 200

    return jsonify({"error": "Topic not found"}), 404


if __name__ == "__main__":
    print("Starting MQTT-Like Broker Server...")
    app.run(host="0.0.0.0", port=5000, debug=True)
