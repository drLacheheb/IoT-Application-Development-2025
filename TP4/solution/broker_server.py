from flask import Flask, request, jsonify
from datetime import datetime
from collections import defaultdict
import threading

app = Flask(__name__)

# Data structure: topic -> list of messages
# Each message: {"payload": str, "timestamp": str, "publisher": str}
messages = defaultdict(list)
message_lock = threading.Lock()

# Configuration
MAX_MESSAGES_PER_TOPIC = 100  # Limit memory usage


@app.route('/')
def home():
	"""
	Broker status page
	"""
	with message_lock:
		topic_stats = {
			topic: len(msgs) for topic, msgs in messages.items()
		}

	html = """
	<html>
	<head><title>MQTT-Like Broker</title></head>
	<body>
		<h1>HTTP-Based MQTT Clone Broker</h1>
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
			<li><strong>POST /publish</strong> - Publish a message
				<br>Body: {"topic": "...", "payload": "...", "publisher": "..."}</li>
			<li><strong>GET /subscribe?topic=...</strong> - Get messages from a topic
				<br>Optional: &since=timestamp (get only new messages)</li>
			<li><strong>DELETE /clear?topic=...</strong> - Clear messages from a topic</li>
		</ul>
	</body>
	</html>
	"""
	return html


@app.route('/publish', methods=['POST'])
def publish():
	"""
	Implement message publishing
	Expected JSON: {
		"topic": "sensors/temperature",
		"payload": "25.5",
		"publisher": "sensor-01"
	}
	"""
	try:
		data = request.get_json()

		# Validate required fields
		if not data or 'topic' not in data or 'payload' not in data:
			return jsonify({"error": "Missing 'topic' or 'payload'"}), 400

		topic = data['topic']
		payload = data['payload']
		publisher = data.get('publisher', 'anonymous')

		# Create message object
		message = {
			"payload": payload,
			"timestamp": datetime.now().isoformat(),
			"publisher": publisher
		}

		# Store message (thread-safe)
		with message_lock:
			messages[topic].append(message)

			# Limit message history
			if len(messages[topic]) > MAX_MESSAGES_PER_TOPIC:
				messages[topic].pop(0)  # Remove oldest

		print(f"[PUBLISH] {publisher} -> {topic}: {payload}")

		return jsonify({
			"success": True,
			"topic": topic,
			"timestamp": message['timestamp']
		}), 200

	except Exception as e:
		return jsonify({"error": str(e)}), 500


@app.route('/subscribe', methods=['GET'])
def subscribe():
	"""
	Implement message subscription
	Query params:
		topic: Required - topic to subscribe to
		since: Optional - only get messages after this timestamp
	"""
	try:
		topic = request.args.get('topic')
		since = request.args.get('since')

		if not topic:
			return jsonify({"error": "Missing 'topic' parameter"}), 400

		with message_lock:
			topic_messages = messages.get(topic, [])

			# Filter by timestamp if provided
			if since:
				topic_messages = [
					msg for msg in topic_messages
					if msg['timestamp'] > since
				]

		return jsonify({
			"topic": topic,
			"count": len(topic_messages),
			"messages": topic_messages
		}), 200

	except Exception as e:
		return jsonify({"error": str(e)}), 500


@app.route('/clear', methods=['DELETE'])
def clear_topic():
	"""
	Clear all messages from a topic
	"""
	topic = request.args.get('topic')

	if not topic:
		return jsonify({"error": "Missing 'topic' parameter"}), 400

	with message_lock:
		if topic in messages:
			count = len(messages[topic])
			del messages[topic]
			return jsonify({
				"success": True,
				"topic": topic,
				"cleared": count
			}), 200
		else:
			return jsonify({
				"success": False,
				"error": "Topic not found"
			}), 404


if __name__ == '__main__':
	print("Starting MQTT-Like Broker Server...")
	print("Broker running at: http://localhost:5000")
	app.run(host='0.0.0.0', port=5000, debug=True)

