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
		Poll the broker for new messages on a topic
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
		Handle received message (customize this)
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

