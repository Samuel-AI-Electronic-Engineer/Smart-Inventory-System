# 🤝 Contributing to SmartInventoryAI

Thank you for your interest in contributing to **SmartInventoryAI**! This document provides guidelines and instructions for contributing to the project.

---

## 🎯 Code of Conduct

By participating in this project, you agree to maintain a respectful and inclusive environment. Please be kind, professional, and constructive in all interactions.

---

## 🚀 Getting Started

### Prerequisites

- **Python 3.11+** (3.12 also supported)
- **Git** and **GitHub** account
- **Docker** (optional, for containerized development)
- **ESP32-CAM + Arduino IDE** (for IoT contributions)

### Development Setup

1. **Fork the repository** on GitHub
2. **Clone your fork:**

   ```bash
   git clone https://github.com/your-username/Smart-Inventory-System.git
   cd Smart-Inventory-System
   ```

3. **Create a virtual environment:**

   ```bash
   python -m venv .venv
   source .venv/bin/activate  # On Windows: .venv\Scripts\activate
   ```

4. **Install dependencies:**

   ```bash
   pip install -r requirements.txt
   pip install -r requirements-dev.txt  # Dev tools: pytest, black, isort, mypy
   ```

5. **Set up pre-commit hooks (optional but recommended):**

   ```bash
   pip install pre-commit
   pre-commit install
   ```

6. **Configure environment variables:**

   ```bash
   cp .env.example .env
   # Edit .env with your local settings
   ```

---

## 📝 Development Workflow

### 1. Create a Feature Branch

```bash
git checkout -b feature/your-feature-name
# Or for bug fixes:
git checkout -b fix/bug-description
```

**Branch naming convention:**

- `feature/` — New features
- `fix/` — Bug fixes
- `refactor/` — Code refactoring
- `docs/` — Documentation updates
- `test/` — Test additions
- `chore/` — Maintenance tasks

### 2. Make Your Changes

Work on your feature/fix in the appropriate directory:

```
agents/           # LangGraph agents (InventoryAgent, AlertAgent, etc.)
vision/           # YOLOv8 detection pipeline
iot/              # ESP32-CAM firmware and MQTT client
cloud/            # GCP integration
database/         # PostgreSQL models and queries
api/              # FastAPI endpoints
mlops/            # Training and evaluation
tests/            # Test suite
```

### 3. Code Style & Formatting

**All Python code must follow these standards:**

#### Black (Code Formatting)

```bash
black . --exclude venv,.venv
```

#### isort (Import Sorting)

```bash
isort . --skip venv --skip .venv
```

#### mypy (Type Checking)

```bash
mypy agents/ vision/ cloud/ database/ --ignore-missing-imports
```

#### flake8 (Linting)

```bash
flake8 . --count --select=E9,F63,F7,F82
```

**Run all checks:**

```bash
# Run before committing
black . --exclude venv,.venv
isort . --skip venv
mypy agents/ vision/ cloud/ database/ --ignore-missing-imports
flake8 . --max-line-length=100
pytest tests/ -v
```

### 4. Commit Message Convention

Follow **Conventional Commits** format:

```
<type>(<scope>): <subject>

<body>

<footer>
```

**Types:**

- `feat:` — New feature
- `fix:` — Bug fix
- `refactor:` — Code refactoring
- `docs:` — Documentation
- `test:` — Test additions
- `perf:` — Performance improvement
- `chore:` — Maintenance

**Examples:**

```bash
git commit -m "feat(vision): add YOLOv8 batch inference support"
git commit -m "fix(iot): handle MQTT disconnection gracefully"
git commit -m "refactor(database): optimize query performance"
git commit -m "docs: update setup guide for ESP32-CAM"
```

### 5. Write Tests

All new features must include tests:

```bash
# Unit tests
pytest tests/test_detector.py -v

# With coverage
pytest tests/ --cov=. --cov-report=html

# Specific test
pytest tests/test_detector.py::test_yolov8_inference -v
```

**Test structure:**

```python
# tests/test_detector.py
import pytest
from vision.detector import YOLOv8Detector

class TestYOLOv8Detector:
    @pytest.fixture
    def detector(self):
        return YOLOv8Detector(model_path='models/yolov8s.pt')
    
    def test_load_model(self, detector):
        assert detector.model is not None
    
    def test_detect_single_image(self, detector):
        results = detector.detect('test_image.jpg')
        assert len(results) > 0
```

### 6. Update Documentation

- Update **README.md** if adding new features
- Update **docstrings** in Python files
- Add comments for complex logic
- Update **.md files** in relevant directories

**Docstring format (Google style):**

```python
def detect(self, image_path: str) -> List[Dict]:
    """
    Detect objects in a single image using YOLOv8.
    
    Args:
        image_path: Path to input image file
        
    Returns:
        List of detections with keys: class_id, confidence, bbox
        
    Raises:
        FileNotFoundError: If image file doesn't exist
        ValueError: If image format not supported
    """
    pass
```

---

## 🔧 Contributing by Component

### Vision (YOLOv8 Detection)

**File:** `vision/detector.py`

**To add a new detection feature:**

1. Implement in `YOLOv8Detector` class
2. Add unit tests in `tests/test_detector.py`
3. Run model evaluation: `python mlops/evaluate.py`
4. Document in `README.md` under "Vision Pipeline"

```python
class YOLOv8Detector:
    def __init__(self, model_path: str):
        """Initialize detector with model file."""
        self.model = YOLO(model_path)
    
    def detect(self, image_path: str) -> List[Dict]:
        """Detect objects in image."""
        results = self.model.predict(image_path)
        return self._parse_results(results)
```

### Agents (LangGraph)

**File:** `agents/orchestrator.py`

**To add a new agent:**

1. Create agent class in `agents/custom_agent.py`
2. Register in `InventoryOrchestrator.register_agent()`
3. Add tests in `tests/test_agents.py`
4. Document in `CONTRIBUTING.md` with example

```python
class CustomAgent(BaseAgent):
    async def execute(self, state: State) -> State:
        """Process state and return updated state."""
        # Implementation
        return state
```

### IoT Firmware (ESP32-CAM)

**File:** `Initial_Version.ino`

**Arduino development:**

1. Edit firmware in Arduino IDE
2. Configure WiFi/MQTT in lines 26-42
3. Upload with BOOT button held
4. Monitor serial output at 115200 baud
5. Update `iot/esp32_client/ESP32_FIRMWARE_SETUP.md`

**To add new MQTT topics:**

- Update topic constants (lines 40-43)
- Document in setup guide with examples
- Test with `mosquitto_sub` and `mosquitto_pub`

### Database (PostgreSQL)

**File:** `database/models.py`

**To add a new model:**

```python
class NewModel(Base):
    __tablename__ = "new_models"
    
    id = Column(Integer, primary_key=True)
    name = Column(String, nullable=False, unique=True)
    created_at = Column(DateTime, default=datetime.utcnow)
```

Update migration and documentation.

### API (FastAPI)

**File:** `api/main.py`

**To add a new endpoint:**

```python
@app.post("/api/custom/endpoint")
async def custom_endpoint(request: RequestSchema) -> ResponseSchema:
    """
    New endpoint description.
    
    Args:
        request: Request payload
        
    Returns:
        Response with results
    """
    # Implementation
    return ResponseSchema(...)
```

Document in API docs (auto-generated at `/docs`).

---

## 🧪 Running Tests

### Full Test Suite

```bash
pytest tests/ -v --cov=. --cov-report=html
```

### Specific Tests

```bash
# Vision tests
pytest tests/test_detector.py -v

# Agent tests
pytest tests/test_agents.py -v

# Integration tests
pytest tests/integration/ -v

# With markers
pytest -m "not slow" -v
```

### Model Evaluation

```bash
python mlops/evaluate.py --model models/yolov8s.pt --data data/warehouse.yaml
```

---

## 🚀 Submitting a Pull Request

1. **Push to your fork:**

   ```bash
   git push origin feature/your-feature-name
   ```

2. **Create a Pull Request on GitHub:**
   - Use the provided PR template
   - Fill in all sections (description, tests, metrics)
   - Link related issues

3. **PR Checklist:**
   - [ ] Code follows style guide (Black, isort)
   - [ ] Tests added/updated
   - [ ] Documentation updated
   - [ ] Commit messages follow convention
   - [ ] CI/CD pipeline passes
   - [ ] No breaking changes (or documented)

4. **Code Review:**
   - Address feedback promptly
   - Don't force-push after review starts
   - Be open to suggestions

5. **Merge:**
   - Squash commits if needed
   - Delete branch after merge

---

## 🐛 Reporting Bugs

**Use the bug report template:** `.github/ISSUE_TEMPLATE/bug_report.yml`

**Include:**

- Clear description of the bug
- Steps to reproduce
- Expected vs actual behavior
- Screenshots/logs
- Hardware specs (for IoT issues)
- YOLOv8 model version

---

## ✨ Suggesting Features

**Use the feature request template:** `.github/ISSUE_TEMPLATE/feature_request.yml`

**Include:**

- Use case and problem statement
- Proposed solution
- Estimated effort
- Business impact

---

## 📚 Documentation

- **README.md** — Project overview
- **iot/esp32_client/ESP32_FIRMWARE_SETUP.md** — Hardware setup
- **API docs** — Auto-generated at `/docs`
- **Docstrings** — In Python code

---

## 🔐 Security

- Never commit secrets or credentials
- Use `.env` for sensitive data
- Follow OWASP guidelines
- Report security issues to the maintainer privately

---

## 📦 Dependencies

**Adding new dependencies:**

1. Install locally: `pip install package-name==version`
2. Add to `requirements.txt` with pinned version
3. Document why in commit message
4. Update Docker files if necessary

---

## 🎓 Learning Resources

- **YOLOv8:** <https://github.com/ultralytics/ultralytics>
- **LangGraph:** <https://langchain-ai.github.io/langgraph/>
- **FastAPI:** <https://fastapi.tiangolo.com/>
- **PostgreSQL:** <https://www.postgresql.org/docs/>
- **MQTT:** <https://mqtt.org/mqtt-specification>
- **ESP32-CAM:** <https://github.com/espressif/arduino-esp32>

---

## ❓ Questions?

- **Discussions:** GitHub Discussions tab
- **Issues:** For bugs and features
- **Email:** <samuel@example.com>

---

## 🙏 Thank You

Your contributions make SmartInventoryAI better for everyone. We appreciate your effort!

**Made with ❤️ by the SmartInventoryAI community**
