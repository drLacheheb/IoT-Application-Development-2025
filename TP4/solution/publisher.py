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
		Publish a message to a topic
		"""
		url = f"{self.broker_url}/publish"

		data = {
			"topic": topic,
			"payload": payload,
			"publisher": self.publisher_id,
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

