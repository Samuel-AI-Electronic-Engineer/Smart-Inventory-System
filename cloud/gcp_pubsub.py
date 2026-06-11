"""Google Cloud Pub/Sub publisher and subscriber for IoT event streaming."""

import logging
from typing import Callable, Optional
from google.cloud import pubsub_v1

logger = logging.getLogger(__name__)


class PubSubClient:
    """Client for GCP Pub/Sub streaming."""

    def __init__(self, project_id: str):
        """
        Initialize Pub/Sub client.

        Args:
            project_id: GCP project ID
        """
        self.project_id = project_id
        self.publisher = pubsub_v1.PublisherClient()
        self.subscriber = pubsub_v1.SubscriberClient()
        logger.info(f"PubSubClient initialized for project: {project_id}")

    def publish(self, topic_name: str, data: bytes) -> str:
        """
        Publish message to Pub/Sub topic.

        Args:
            topic_name: Topic name (e.g., 'inventory-detections')
            data: Message payload (bytes)

        Returns:
            Message ID
        """
        topic_path = self.publisher.topic_path(self.project_id, topic_name)
        future = self.publisher.publish(topic_path, data)
        message_id = future.result()
        logger.debug(f"Published message {message_id} to {topic_name}")
        return message_id

    def subscribe(
        self, subscription_name: str, callback: Callable[[str, bytes], None]
    ):
        """
        Subscribe to Pub/Sub subscription.

        Args:
            subscription_name: Subscription name
            callback: Function to call on message (receives message_id, data)
        """
        subscription_path = self.subscriber.subscription_path(
            self.project_id, subscription_name
        )

        def message_callback(message):
            callback(message.message_id, message.data)
            message.ack()

        streaming_pull_future = self.subscriber.subscribe(
            subscription_path, callback=message_callback
        )
        logger.info(f"Subscribed to {subscription_name}")
        return streaming_pull_future

    def delete_topic(self, topic_name: str):
        """Delete a Pub/Sub topic."""
        topic_path = self.publisher.topic_path(self.project_id, topic_name)
        self.publisher.delete_topic(request={"topic": topic_path})
        logger.info(f"Deleted topic: {topic_name}")


# TODO: Implement streaming message handler
# TODO: Implement batch subscriber for high-throughput
