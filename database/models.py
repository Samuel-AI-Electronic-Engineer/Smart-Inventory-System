"""SQLAlchemy ORM models for inventory, products, and audit logs."""

from datetime import datetime
from sqlalchemy import Column, Integer, String, Float, DateTime, Boolean, ForeignKey
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy.orm import relationship

Base = declarative_base()


class Product(Base):
    """Product catalog entry."""

    __tablename__ = "products"

    id = Column(Integer, primary_key=True)
    sku = Column(String(50), unique=True, nullable=False, index=True)
    name = Column(String(255), nullable=False)
    category = Column(String(100))
    unit_price = Column(Float)
    reorder_threshold = Column(Integer, default=10)
    created_at = Column(DateTime, default=datetime.utcnow)
    updated_at = Column(DateTime, default=datetime.utcnow,
                        onupdate=datetime.utcnow)

    # Relationships
    inventory = relationship("Inventory", back_populates="product")
    audit_logs = relationship("AuditLog", back_populates="product")


class Inventory(Base):
    """Current inventory state by product and location."""

    __tablename__ = "inventory"

    id = Column(Integer, primary_key=True)
    product_id = Column(Integer, ForeignKey("products.id"), nullable=False)
    location = Column(String(100))  # e.g., "shelf-A1"
    quantity = Column(Integer, default=0)
    last_count_at = Column(DateTime)
    last_detected_at = Column(DateTime)
    updated_at = Column(DateTime, default=datetime.utcnow,
                        onupdate=datetime.utcnow)

    # Relationships
    product = relationship("Product", back_populates="inventory")


class AuditLog(Base):
    """Audit trail for all inventory changes."""

    __tablename__ = "audit_logs"

    id = Column(Integer, primary_key=True)
    product_id = Column(Integer, ForeignKey("products.id"), nullable=False)
    action = Column(String(50))  # e.g., "count_update", "reorder_triggered"
    previous_qty = Column(Integer)
    new_qty = Column(Integer)
    agent_name = Column(String(50))  # e.g., "InventoryAgent"
    detection_confidence = Column(Float)
    notes = Column(String(500))
    created_at = Column(DateTime, default=datetime.utcnow, index=True)

    # Relationships
    product = relationship("Product", back_populates="audit_logs")


class Alert(Base):
    """Reorder alerts triggered by AlertAgent."""

    __tablename__ = "alerts"

    id = Column(Integer, primary_key=True)
    product_id = Column(Integer, ForeignKey("products.id"), nullable=False)
    alert_type = Column(String(50))  # e.g., "low_stock", "missing_item"
    current_qty = Column(Integer)
    threshold = Column(Integer)
    acknowledged = Column(Boolean, default=False)
    acknowledged_at = Column(DateTime)
    acknowledged_by = Column(String(100))
    created_at = Column(DateTime, default=datetime.utcnow, index=True)


# TODO: Add detection history model
# TODO: Add model versioning/metadata tables
