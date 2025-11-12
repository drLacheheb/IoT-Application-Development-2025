import requests
import time
import random
import argparse


class HTTPPublisher:
    def __init__(self, broker_url, publisher_id):
        self.broker_url = broker_url
        self.publisher_id = publisher_id

    def publish(self, topic, payload):
        """
        TODO: Publish a message to a topic
        """
        url = f"{self.broker_url}/publish"

        data = {"topic": topic, "payload": payload, "publisher": self.publisher_id}

        # TODO: Send POST request and handle response

        return False


def simulate_temperature_sensor(publisher, topic, interval=2):
    """Simulate a temperature sensor"""
    print(f"Publishing to topic: {topic}\n")

    try:
        while True:
            temperature = round(random.uniform(20.0, 30.0), 2)
            humidity = round(random.uniform(40.0, 70.0), 2)
            payload = f"temp:{temperature},humidity:{humidity}"

            publisher.publish(topic, payload)
            time.sleep(interval)

    except KeyboardInterrupt:
        print("\nStopping publisher...")


def main():
    parser = argparse.ArgumentParser(description="HTTP Publisher")
    parser.add_argument("--broker", default="http://localhost:5000")
    parser.add_argument("--topic", default="sensors/temperature")
    parser.add_argument("--id", default="publisher-01")
    parser.add_argument("--interval", type=int, default=2)
    parser.add_argument("--message", help="Single message mode")

    args = parser.parse_args()
    publisher = HTTPPublisher(args.broker, args.id)

    if args.message:
        publisher.publish(args.topic, args.message)
    else:
        simulate_temperature_sensor(publisher, args.topic, args.interval)


if __name__ == "__main__":
    main()
