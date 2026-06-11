"""pytest configuration and shared fixtures for tests."""

import pytest
import logging

# Configure logging for tests
logging.basicConfig(level=logging.DEBUG)


@pytest.fixture
def mock_detector():
    """Mock YOLOv8 detector for testing."""
    # TODO: Implement mock detector
    pass


@pytest.fixture
def mock_pubsub():
    """Mock GCP Pub/Sub client for testing."""
    # TODO: Implement mock Pub/Sub
    pass


@pytest.fixture
def mock_db_session():
    """Mock database session for testing."""
    # TODO: Implement mock database session
    pass


@pytest.fixture
def orchestrator(mock_pubsub, mock_db_session):
    """Create orchestrator instance for testing."""
    from agents.orchestrator import InventoryOrchestrator

    return InventoryOrchestrator(
        pubsub_client=mock_pubsub, db_session=mock_db_session
    )
