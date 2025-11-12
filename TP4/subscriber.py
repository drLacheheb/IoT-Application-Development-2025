import requests
import time
import argparse


class HTTPSubscriber:
    def __init__(self, broker_url, subscriber_id):
        self.broker_url = broker_url
        self.subscriber_id = subscriber_id
        self.last_timestamp = {}

    def subscribe(self, topic):
        """
        TODO: Poll the broker for new messages
        """
        url = f"{self.broker_url}/subscribe"
        params = {"topic": topic}

        if topic in self.last_timestamp:
            params["since"] = self.last_timestamp[topic]

        # TODO: Send GET request and process messages

        return 0

    def on_message(self, topic, message):
        """Handle received message"""
        timestamp = message["timestamp"]
        publisher = message["publisher"]
        payload = message["payload"]

        print(f"[{timestamp}] {topic}")
        print(f"  From: {publisher}")
        print(f"  Data: {payload}\n")


def monitor_topic(subscriber, topic, poll_interval=1):
    """Monitor a topic continuously"""
    print(f"Subscribing to: {topic}\n")

    try:
        while True:
            new_messages = subscriber.subscribe(topic)
            if new_messages == 0:
                print(".", end="", flush=True)
            time.sleep(poll_interval)
    except KeyboardInterrupt:
        print("\nStopping subscriber...")


def main():
    parser = argparse.ArgumentParser(description="HTTP Subscriber")
    parser.add_argument("--broker", default="http://localhost:5000")
    parser.add_argument("--topic", default="sensors/temperature")
    parser.add_argument("--id", default="subscriber-01")
    parser.add_argument("--interval", type=int, default=1)

    args = parser.parse_args()
    subscriber = HTTPSubscriber(args.broker, args.id)
    monitor_topic(subscriber, args.topic, args.interval)


if __name__ == "__main__":
    main()
