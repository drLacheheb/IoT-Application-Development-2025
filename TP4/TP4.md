# TP4: MQTT Clone with HTTP – Understanding Publish/Subscribe Patterns

This practical work demonstrates how MQTT-style publish/subscribe messaging works by implementing a simplified version using only HTTP. You'll create a broker server and client scripts that simulate MQTT's pub/sub model, helping you understand the underlying concepts without needing MQTT infrastructure.

## 1. Overview

In this TP, you will build a simple message broker system that mimics MQTT functionality using standard HTTP requests. This exercise deepens your understanding of how publish/subscribe systems work and provides a foundation for working with real MQTT brokers.

### System Architecture

The system consists of three components:

1.  **HTTP Broker Server (Python + Flask):**

    - Stores messages organized by topics
    - Provides `/publish` endpoint for publishers to send messages
    - Provides `/subscribe` endpoint for subscribers to retrieve messages
    - Maintains message history with timestamps
    - Supports multiple topics and subscribers

2.  **Publisher Script (Python):**

    - Sends messages to specific topics via HTTP POST
    - Simulates sensor data or commands
    - Doesn't need to know about subscribers

3.  **Subscriber Script (Python):**
    - Polls the broker for messages on subscribed topics
    - Processes received messages
    - Doesn't need to know about publishers

### Key MQTT Concepts Demonstrated

- **Topics:** Hierarchical message routing (e.g., `sensors/temperature`, `devices/led/control`)
- **Publish:** Sending messages to a topic
- **Subscribe:** Receiving messages from one or more topics
- **Broker:** Central server managing message distribution
- **Decoupling:** Publishers and subscribers don't know about each other
- **Message Persistence:** Storing messages for late-joining subscribers

## 2. MQTT vs This HTTP Implementation

### Similarities

- **Pub/Sub Pattern:** Same decoupling of message producers and consumers
- **Topics:** Hierarchical organization of message channels
- **Broker-Mediated:** Central server handles all routing
- **Multiple Subscribers:** One message can reach many subscribers

### Differences

| Feature               | Real MQTT                      | This HTTP Clone                |
| --------------------- | ------------------------------ | ------------------------------ |
| **Connection**        | Persistent TCP                 | Stateless HTTP requests        |
| **Push vs Pull**      | Broker pushes to subscribers   | Subscribers poll broker        |
| **QoS Levels**        | 0, 1, 2 (guaranteed delivery)  | Best effort only               |
| **Retained Messages** | Last message kept per topic    | All messages stored            |
| **Wildcards**         | `+` and `#` for topic patterns | Exact topic match only         |
| **Performance**       | Very efficient, low latency    | Higher overhead, polling delay |
| **Use Case**          | Production IoT systems         | Learning and simple prototypes |

## 3. Implementation Guide

## Part A: Broker Server

The broker manages topics, stores messages, and serves publishers and subscribers.

### Task 1: Broker Implementation

**File:** `TP4/broker_server.py`

```python
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
    TODO: Implement message publishing
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
    TODO: Implement message subscription
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
```

**Explanation:**

- `defaultdict(list)` creates a dictionary where each topic maps to a list of messages
- `threading.Lock()` ensures thread-safe access when multiple clients connect simultaneously
- We limit message history to prevent memory overflow
- The `since` parameter allows subscribers to get only new messages

## Part B: Publisher Script

The publisher sends messages to topics.

### Task 1: Basic Publisher

**File:** `TP4/publisher.py`

```python
import requests
import time
import random
import argparse
from datetime import datetime

class HTTPPublisher:
    def __init__(self, broker_url, publisher_id):
        self.broker_url = broker_url
        self.publisher_id = publisher_id

    def publish(self, topic, payload):
        """
        TODO: Publish a message to a topic
        """
        url = f"{self.broker_url}/publish"

        data = {
            "topic": topic,
            "payload": payload,
            "publisher": self.publisher_id
        }

        try:
            response = requests.post(url, json=data, timeout=5)

            if response.status_code == 200:
                print(f"✓ Published to '{topic}': {payload}")
                return True
            else:
                print(f"✗ Failed to publish: {response.text}")
                return False

        except requests.exceptions.RequestException as e:
            print(f"✗ Connection error: {e}")
            return False

def simulate_temperature_sensor(publisher, topic, interval=2):
    """
    Simulate a temperature sensor publishing data
    """
    print(f"Starting temperature sensor simulation...")
    print(f"Publishing to topic: {topic}")
    print(f"Interval: {interval} seconds\n")

    try:
        while True:
            # Generate fake temperature data
            temperature = round(random.uniform(20.0, 30.0), 2)
            humidity = round(random.uniform(40.0, 70.0), 2)

            # Publish temperature
            payload = f"temp:{temperature},humidity:{humidity}"
            publisher.publish(topic, payload)

            time.sleep(interval)

    except KeyboardInterrupt:
        print("\nStopping publisher...")

def main():
    parser = argparse.ArgumentParser(description='MQTT-like HTTP Publisher')
    parser.add_argument('--broker', default='http://localhost:5000',
                        help='Broker URL (default: http://localhost:5000)')
    parser.add_argument('--topic', default='sensors/temperature',
                        help='Topic to publish to (default: sensors/temperature)')
    parser.add_argument('--id', default='publisher-01',
                        help='Publisher ID (default: publisher-01)')
    parser.add_argument('--interval', type=int, default=2,
                        help='Publish interval in seconds (default: 2)')
    parser.add_argument('--message', help='Publish a single message and exit')

    args = parser.parse_args()

    publisher = HTTPPublisher(args.broker, args.id)

    if args.message:
        # Single message mode
        publisher.publish(args.topic, args.message)
    else:
        # Continuous simulation mode
        simulate_temperature_sensor(publisher, args.topic, args.interval)

if __name__ == '__main__':
    main()
```

**Explanation:**

- Command-line arguments allow flexible testing
- Simulates a temperature sensor with random data
- Can also send single messages for testing
- Handles connection errors gracefully

## Part C: Subscriber Script

The subscriber polls the broker for new messages.

### Task 1: Basic Subscriber

**File:** `TP4/subscriber.py`

```python
import requests
import time
import argparse
from datetime import datetime

class HTTPSubscriber:
    def __init__(self, broker_url, subscriber_id):
        self.broker_url = broker_url
        self.subscriber_id = subscriber_id
        self.last_timestamp = {}  # Track last seen timestamp per topic

    def subscribe(self, topic):
        """
        TODO: Poll the broker for new messages on a topic
        """
        url = f"{self.broker_url}/subscribe"
        params = {"topic": topic}

        # Include timestamp to get only new messages
        if topic in self.last_timestamp:
            params["since"] = self.last_timestamp[topic]

        try:
            response = requests.get(url, params=params, timeout=5)

            if response.status_code == 200:
                data = response.json()
                messages = data.get('messages', [])

                # Process new messages
                for msg in messages:
                    self.on_message(topic, msg)
                    self.last_timestamp[topic] = msg['timestamp']

                return len(messages)
            else:
                print(f"✗ Failed to subscribe: {response.text}")
                return 0

        except requests.exceptions.RequestException as e:
            print(f"✗ Connection error: {e}")
            return 0

    def on_message(self, topic, message):
        """
        TODO: Handle received message (customize this)
        """
        timestamp = message['timestamp']
        publisher = message['publisher']
        payload = message['payload']

        print(f"[{timestamp}] {topic}")
        print(f"  From: {publisher}")
        print(f"  Data: {payload}\n")

def monitor_topic(subscriber, topic, poll_interval=1):
    """
    Continuously monitor a topic for new messages
    """
    print(f"Subscribing to topic: {topic}")
    print(f"Poll interval: {poll_interval} seconds\n")

    try:
        while True:
            new_messages = subscriber.subscribe(topic)

            if new_messages == 0:
                # No new messages, just show a dot
                print(".", end="", flush=True)

            time.sleep(poll_interval)

    except KeyboardInterrupt:
        print("\nStopping subscriber...")

def main():
    parser = argparse.ArgumentParser(description='MQTT-like HTTP Subscriber')
    parser.add_argument('--broker', default='http://localhost:5000',
                        help='Broker URL (default: http://localhost:5000)')
    parser.add_argument('--topic', default='sensors/temperature',
                        help='Topic to subscribe to (default: sensors/temperature)')
    parser.add_argument('--id', default='subscriber-01',
                        help='Subscriber ID (default: subscriber-01)')
    parser.add_argument('--interval', type=int, default=1,
                        help='Poll interval in seconds (default: 1)')

    args = parser.parse_args()

    subscriber = HTTPSubscriber(args.broker, args.id)
    monitor_topic(subscriber, args.topic, args.interval)

if __name__ == '__main__':
    main()
```

**Explanation:**

- Tracks `last_timestamp` to avoid re-processing old messages
- Polling interval balances responsiveness vs network load
- Custom `on_message()` method can be extended for specific processing

## 4. Testing Your Implementation

### Step 1: Start the Broker

```powershell
cd TP4
python broker_server.py
```

### Step 2: Start a Subscriber

Open a new terminal:

```powershell
cd TP4
python subscriber.py --topic sensors/temperature
```

### Step 3: Start a Publisher

Open another terminal:

```powershell
cd TP4
python publisher.py --topic sensors/temperature --interval 3
```

### Step 4: Observe Message Flow

- Publisher sends temperature/humidity data every 3 seconds
- Subscriber receives and displays new messages
- Broker web interface shows active topics

### Step 5: Test Multiple Subscribers

Start another subscriber for the same topic:

```powershell
python subscriber.py --topic sensors/temperature --id subscriber-02
```

Both subscribers will receive all messages (pub/sub pattern).

### Step 6: Test Different Topics

```powershell
# Publish to a different topic
python publisher.py --topic devices/led/control --message "ON" --id controller-01

# Subscribe to that topic
python subscriber.py --topic devices/led/control
```

### Step 7: Manual Testing

Use PowerShell or curl to test endpoints directly:

```powershell
# Publish a message
Invoke-RestMethod -Uri "http://localhost:5000/publish" -Method POST -ContentType "application/json" -Body '{"topic":"test/manual","payload":"Hello World","publisher":"manual-test"}'

# Subscribe to a topic
Invoke-RestMethod -Uri "http://localhost:5000/subscribe?topic=test/manual" -Method GET
```

## 5. Exercises and Extensions

### Exercise 1: Topic Wildcards

Add support for wildcard subscriptions (e.g., `sensors/#` to get all sensor topics).

### Exercise 2: Message Filtering

Add filters to the subscriber to process only messages matching certain criteria.

### Exercise 3: QoS Simulation

Implement a simple "at-least-once" delivery guarantee using message acknowledgments.

### Exercise 4: Web Dashboard

Create a web page that displays real-time messages from all topics.

### Exercise 5: Performance Comparison

Measure latency differences between this HTTP implementation and real MQTT.

## ✅ Deliverables

Submit the following:

1. **Code Files:**

   - `TP4/broker_server.py`
   - `TP4/publisher.py`
   - `TP4/subscriber.py`
   - `TP4/requirements.txt`

2. **Documentation (PDF or Markdown):**

   - Explanation of how pub/sub pattern works
   - Comparison table: HTTP polling vs MQTT push
   - Screenshots showing multiple publishers/subscribers
   - Latency measurements (time from publish to receive)
   - Discussion of scalability limitations

3. **Testing Results:**
   - Terminal outputs from broker, publisher, and subscriber
   - Evidence of multiple subscribers receiving the same messages
   - Test with at least 3 different topics

## 🧩 Grading Criteria

- **Broker Implementation (30%):** Correct message storage and routing
- **Publisher (25%):** Successful message publishing, error handling
- **Subscriber (25%):** Correct polling, message processing, timestamp tracking
- **System Testing (10%):** Multiple clients, different topics
- **Documentation (10%):** Clear comparison with MQTT, analysis

## 6. Requirements File

**File:** `TP4/requirements.txt`

```
Flask==3.0.0
requests==2.31.0
```

Install dependencies:

```powershell
pip install -r requirements.txt
```
