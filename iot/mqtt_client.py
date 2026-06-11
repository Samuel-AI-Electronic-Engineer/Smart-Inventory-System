"""MQTT broker and client for IoT edge communication."""

import logging
from typing import Callable, Optional

logger = logging.getLogger(__name__)


class MQTTClient:
    """MQTT client for publishing/subscribing to IoT topics."""

    def __init__(
        self,
        broker_host: str,
        broker_port: int,
        username: str,
        password: str,
        qos: int = 1,
    ):
        """
        Initialize MQTT client.

        Args:
            broker_host: MQTT broker hostname/IP
            broker_port: MQTT broker port (default: 1883)
            username: MQTT authentication username
            password: MQTT authentication password
            qos: Quality of Service level (0, 1, or 2)
        """
        self.broker_host = broker_host
        self.broker_port = broker_port
        self.username = username
        self.password = password
        self.qos = qos
        self.client = None
        logger.info(f"MQTTClient initialized for {broker_host}:{broker_port}")

    def connect(self):
        """Establish connection to MQTT broker."""
        try:
            import paho.mqtt.client as mqtt

            self.client = mqtt.Client()
            self.client.username_pw_set(self.username, self.password)
            self.client.connect(
                self.broker_host, self.broker_port, keepalive=60)
            self.client.loop_start()
            logger.info("Connected to MQTT broker")
        except Exception as e:
            logger.error(f"Failed to connect to MQTT broker: {e}")
            raise

    def publish(self, topic: str, payload: bytes, retain: bool = False):
        """Publish message to MQTT topic."""
        if self.client is None:
            self.connect()

        self.client.publish(topic, payload, qos=self.qos, retain=retain)
        logger.debug(f"Published to {topic}")

    def subscribe(self, topic: str, callback: Callable):
        """Subscribe to MQTT topic with callback handler."""
        if self.client is None:
            self.connect()

        def on_message_wrapper(client, userdata, msg):
            callback(msg.topic, msg.payload)

        self.client.message_callback_add(topic, on_message_wrapper)
        self.client.subscribe(topic, qos=self.qos)
        logger.info(f"Subscribed to {topic}")

    def disconnect(self):
        """Disconnect from MQTT broker."""
        if self.client:
            self.client.loop_stop()
            self.client.disconnect()
            logger.info("Disconnected from MQTT broker")


# TODO: Implement LocalMQTTBroker for Raspberry Pi
# TODO: Implement ESP32-CAM client wrapper
