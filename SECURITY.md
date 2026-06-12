# 🔐 Security Policy

## Supported Versions

| Version | Status | Support Until |
|---------|--------|---------------|
| 1.0.x   | ✅ Current | 2026-12-31 |
| 0.9.x   | ⚠️ Maintenance | 2026-06-30 |
| 0.8.x   | ❌ EOL | 2025-12-31 |

---

## Reporting Security Vulnerabilities

**Do NOT open a public GitHub issue for security vulnerabilities.**

### Disclosure Process

1. **Email the maintainer** at <security@smartinventoryai.com> with:
   - Description of vulnerability
   - Steps to reproduce (if applicable)
   - Potential impact
   - Suggested fix (if you have one)

2. **We will:**
   - Acknowledge receipt within 48 hours
   - Investigate and assess severity
   - Develop and test a patch
   - Release security update within 30 days
   - Notify you before public disclosure

3. **Public Disclosure:**
   - CVE request filed if applicable
   - Security advisory published
   - Version bump with security fix
   - Changelog updated

### Vulnerability Severity Levels

- **Critical:** Remote code execution, authentication bypass, data breach
- **High:** Privilege escalation, sensitive information disclosure
- **Medium:** Denial of service, configuration issues, weak cryptography
- **Low:** Minor security concerns, best practice improvements

---

## Security Practices

### Application Security

#### 1. Authentication & Authorization

**OAuth 2.0 / MSAL:**

```python
# agents/auth.py
from microsoft_identity_web import app_owns_data

@app_owns_data()
def get_current_user(token: str) -> User:
    # Validate JWT token
    # Extract user claims
    # Return authenticated user
    pass
```

**API Key Security:**

- Store in `.env` (never in code)
- Rotate keys regularly
- Use short expiration times
- Implement rate limiting

#### 2. Data Encryption

**In Transit (TLS 1.2+):**

```python
# api/main.py
from fastapi.middleware.trustedhost import TrustedHostMiddleware

app.add_middleware(TrustedHostMiddleware, allowed_hosts=["yourdomain.com"])

# HTTPS enforced in production
# Certificates from Let's Encrypt or Azure Key Vault
```

**At Rest (PostgreSQL):**

```sql
-- Enable SSL connections
ALTER SYSTEM SET ssl = on;
ALTER SYSTEM SET ssl_cert_file = '/path/to/server.crt';
ALTER SYSTEM SET ssl_key_file = '/path/to/server.key';
```

**Secrets Management:**

```python
# Use Azure Key Vault for sensitive data
from azure.identity import DefaultAzureCredential
from azure.keyvault.secrets import SecretClient

credential = DefaultAzureCredential()
client = SecretClient(vault_url=kv_uri, credential=credential)
secret = client.get_secret("db-password")
```

#### 3. Input Validation

```python
# api/schemas.py
from pydantic import BaseModel, Field, validator

class ImageDetectionRequest(BaseModel):
    image_path: str = Field(..., min_length=1, max_length=255)
    confidence_threshold: float = Field(0.45, ge=0.0, le=1.0)
    
    @validator('image_path')
    def validate_path(cls, v):
        # Prevent directory traversal
        if '..' in v or v.startswith('/'):
            raise ValueError('Invalid path')
        return v
```

#### 4. SQL Injection Prevention

```python
# database/queries.py
from sqlalchemy import text

# ✅ SAFE: Use parameterized queries
query = text("SELECT * FROM products WHERE sku = :sku")
result = db.execute(query, {"sku": user_input})

# ❌ DANGEROUS: String concatenation
query = f"SELECT * FROM products WHERE sku = '{user_input}'"  # Never!
```

#### 5. CSRF Protection

```python
# api/main.py
from fastapi_csrf_protect import CsrfProtect

@app.post("/api/inventory/update")
async def update_inventory(request: Request, csrf_protect: CsrfProtect = Depends()):
    await csrf_protect.validate_csrf(request)
    # Process request
```

---

### IoT Security

#### 1. MQTT Authentication

**Configuration in ESP32-CAM firmware:**

```cpp
// Initial_Version.ino (lines 36-39)
const char* MQTT_USERNAME = "mqtt_user";      // Non-default username
const char* MQTT_PASSWORD = "mqtt_pass";      // Strong password (32+ chars)
const char* MQTT_BROKER = "192.168.1.100";    // Private network only
```

**Mosquitto broker configuration:**

```conf
# /etc/mosquitto/mosquitto.conf
allow_anonymous false
password_file /etc/mosquitto/passwd

listener 8883
protocol mqtt
cafile /etc/mosquitto/certs/ca.crt
certfile /etc/mosquitto/certs/server.crt
keyfile /etc/mosquitto/certs/server.key
tls_version tlsv1.2
```

#### 2. MQTT TLS/SSL

```bash
# Generate certificates (Raspberry Pi)
openssl genrsa -out server.key 2048
openssl req -new -x509 -days 365 -key server.key -out server.crt
openssl genrsa -out ca.key 2048
openssl req -new -x509 -days 365 -key ca.key -out ca.crt
```

**Arduino firmware with TLS:**

```cpp
// Use port 8883 for TLS
const int MQTT_PORT = 8883;
const char* CA_CERT = "-----BEGIN CERTIFICATE-----\n...";

WiFiClientSecure espClient;
espClient.setCACert(CA_CERT);
client.setClient(espClient);
```

#### 3. Device Identity Management

```cpp
// Initial_Version.ino (line 41)
const char* DEVICE_ID = "esp32-cam-shelf-001";  // Unique per device

// Change on every deployment
// Use hardware serial number: 
String getDeviceID() {
    uint8_t mac[6];
    WiFi.macAddress(mac);
    return String(mac[0], HEX) + String(mac[1], HEX) + ...;
}
```

#### 4. Firmware Updates

**Secure update mechanism:**

```cpp
void checkFirmwareUpdates() {
    // Check server for latest version
    // Verify digital signature
    // Use HTTPS only
    // Validate SHA256 checksum
    // Update via OTA (Over-The-Air)
}
```

---

### Cloud Security

#### 1. GCP Project Setup

```bash
# Enable only necessary APIs
gcloud services enable run.googleapis.com
gcloud services enable pubsub.googleapis.com
gcloud services enable storage-component.googleapis.com

# Create service account with minimal permissions
gcloud iam service-accounts create smartinventoryai-sa
gcloud projects add-iam-policy-binding $PROJECT_ID \
    --member="serviceAccount:smartinventoryai-sa@..." \
    --role="roles/run.invoker"
```

#### 2. Private Container Registry

```bash
# Push to private Artifact Registry (not public Docker Hub)
gcloud auth configure-docker us-central1-docker.pkg.dev

docker tag smartinventoryai:latest \
    us-central1-docker.pkg.dev/$PROJECT_ID/smartinventory/api:latest

docker push us-central1-docker.pkg.dev/$PROJECT_ID/smartinventory/api:latest

# Deploy to Cloud Run with private access
gcloud run deploy smartinventoryai-api \
    --image us-central1-docker.pkg.dev/$PROJECT_ID/smartinventory/api:latest \
    --ingress internal  # Private access only
```

#### 3. Network Security

```yaml
# Cloud Run security
- No public ingress (ingress: internal)
- Private IP access only
- VPC connectors for database access
- Firewall rules restrict traffic
```

---

### Database Security

#### 1. PostgreSQL Hardening

```sql
-- Create restricted user
CREATE USER smartinv_app WITH PASSWORD 'strong_password_32+_chars';
GRANT CONNECT ON DATABASE smartinventory TO smartinv_app;
GRANT USAGE ON SCHEMA public TO smartinv_app;
GRANT SELECT, INSERT, UPDATE ON ALL TABLES IN SCHEMA public TO smartinv_app;

-- Revoke dangerous permissions
REVOKE SUPERUSER ON smartinv_app;
REVOKE CREATEDB ON smartinv_app;

-- Enable SSL
-- Host-based authentication (pg_hba.conf)
hostssl  smartinventory  smartinv_app  192.168.1.0/24  md5
```

#### 2. Secrets Rotation

```bash
# Rotate database password every 90 days
# Use Azure Key Vault automatic rotation
az keyvault secret rotate --name db-password

# Update connection strings across services
# Verify all services can connect with new password
```

---

### Code Security

#### 1. Dependencies Management

```bash
# Scan for vulnerable dependencies
pip install safety
safety check

# Use pinned versions in requirements.txt
# Update regularly but carefully
pip list --outdated
```

#### 2. Secrets Scanning

```bash
# Pre-commit hook to prevent secrets leak
pip install detect-secrets
detect-secrets scan --all-files
```

#### 3. SAST (Static Application Security Testing)

```bash
# Code quality and security analysis
pip install bandit
bandit -r . -f json -o bandit-report.json

# Run on CI/CD
# Block merge if critical issues found
```

---

### Compliance & Audit

#### 1. Audit Logging

```python
# database/models.py
class AuditLog(Base):
    __tablename__ = "audit_logs"
    
    id = Column(Integer, primary_key=True)
    user_id = Column(String)
    action = Column(String)  # CREATE, UPDATE, DELETE
    table_name = Column(String)
    record_id = Column(Integer)
    old_value = Column(String)
    new_value = Column(String)
    timestamp = Column(DateTime, default=datetime.utcnow)
    
    # Immutable: never update/delete audit logs
```

#### 2. Data Retention

- Production backups: 7 days
- Archive backups: 90 days
- Deleted data: purged after 30 days
- Audit logs: retained for 2 years

#### 3. Compliance Standards

- ✅ GDPR: Data processing agreements, right to be forgotten
- ✅ HIPAA: If processing health data
- ✅ SOC 2: Security controls documented
- ✅ ISO 27001: Information security management

---

## Incident Response Plan

### Response Timeline

1. **T+0-1 hour:** Acknowledge, confirm, assess severity
2. **T+1-4 hours:** Investigate, identify root cause, develop fix
3. **T+4-24 hours:** Test fix, prepare patch release
4. **T+24-48 hours:** Release security update, notify users
5. **T+48+ hours:** Post-incident review, preventive measures

### Incident Notification

```
From: security@smartinventoryai.com
Subject: Security Advisory - [CVE-XXXX-XXXXX]

SmartInventoryAI has identified and patched a security vulnerability.

Affected Versions: 1.0.0 - 1.0.5
Fixed Version: 1.0.6

Severity: HIGH
CVSS Score: 7.5

Summary: [Description]
Impact: [Affected users/data]
Action Required: Update to version 1.0.6
```

---

## Security Checklist

### Before Production Deployment

- [ ] All dependencies updated and audited
- [ ] HTTPS/TLS enabled
- [ ] Database encrypted at rest and in transit
- [ ] API keys/secrets in Key Vault (not in code)
- [ ] Input validation on all endpoints
- [ ] MQTT using TLS 1.2+ with authentication
- [ ] Firewall rules restrict network access
- [ ] DDoS protection enabled (Azure DDoS Protection)
- [ ] Backup strategy implemented
- [ ] Incident response plan documented
- [ ] Security audit completed
- [ ] Penetration testing completed
- [ ] User authentication enforced
- [ ] Audit logging enabled
- [ ] Rate limiting configured

---

## Resources

- **OWASP Top 10:** <https://owasp.org/www-project-top-ten/>
- **CWE:** <https://cwe.mitre.org/>
- **NIST Cybersecurity Framework:** <https://www.nist.gov/cyberframework>
- **Azure Security:** <https://learn.microsoft.com/en-us/azure/security/>
- **GCP Security:** <https://cloud.google.com/security>

---

**Last Updated:** 2026-06-11
**Maintainer:** Samuel Alarcón Hernández
