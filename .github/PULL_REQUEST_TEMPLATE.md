## 🎯 PR Description

**Related Issue:** Closes #[issue number]

### Summary

Brief description of changes made in this PR.

### Type of Change

- [ ] 🐛 Bug Fix (fixes issue #___)
- [ ] ✨ New Feature (implements feature #___)
- [ ] 📚 Documentation Update
- [ ] 🔄 Refactoring (no functional changes)
- [ ] ⚡ Performance Improvement
- [ ] 🔐 Security Fix
- [ ] 🎨 Code Style/Formatting

---

## 📝 Changes Made

### Modified Files

- [ ] `agents/` — LangGraph orchestration changes
- [ ] `vision/` — YOLOv8 detection pipeline changes
- [ ] `iot/` — ESP32-CAM firmware changes
- [ ] `cloud/` — GCP integration changes
- [ ] `database/` — PostgreSQL schema/queries changes
- [ ] `api/` — FastAPI endpoint changes
- [ ] `mlops/` — Training/evaluation pipeline changes
- [ ] `tests/` — Test additions/modifications
- [ ] `docs/` — Documentation updates

### Implementation Details

Describe what was implemented and why:

```python
# Example: Explain key code changes here
```

---

## ✅ Testing & Validation

### Tests Added

- [ ] Unit tests added (pytest)
- [ ] Integration tests added
- [ ] Manual testing completed

### Test Results

```bash
# Paste test output here
pytest --cov=. --cov-report=html
# Coverage: XX%
```

### Vision Model Evaluation (if applicable)

- [ ] YOLOv8 model evaluation completed
- [ ] mAP@0.5 score: ____ (acceptable threshold: > 0.75)
- [ ] mAP@0.5:0.95 score: ____ (acceptable threshold: > 0.60)

```
# Paste model evaluation metrics here
Model: yolov8s
Dataset: warehouse_products_v2
Results:
  - mAP@0.5: 0.82
  - mAP@0.5:0.95: 0.68
  - Precision: 0.85
  - Recall: 0.79
```

### Hardware Testing (if applicable to IoT/firmware)

- [ ] Tested on ESP32-CAM
- [ ] Tested on Raspberry Pi 4B
- [ ] Verified MQTT communication
- [ ] Verified servo control
- [ ] Power consumption within limits

**Hardware Configuration:**

- ESP32-CAM: AI Thinker ESP32-CAM
- Servo Motors: SG90 (if applicable)
- Power Supply: 5V/2A

---

## 📊 Performance & Metrics

### Performance Impact

- [ ] No performance regression
- [ ] Improved performance by ___% (specify metric)
- [ ] API response time: ___ ms (acceptable: < 500ms)
- [ ] Memory usage: ___ MB (acceptable: < 512MB)

### Metrics

```
# Include relevant metrics:
- Inference latency: XXX ms/image
- Throughput: XX images/sec
- Accuracy change: ±X%
- Build time: XX sec
- Docker image size: XX MB
```

---

## 📸 Screenshots/Demo (if applicable)

### Vision Detection Results

```
[Paste image or metrics here]
```

### API Response Example

```json
{
  "status": "success",
  "detection_count": 42,
  "avg_confidence": 0.87,
  "processing_time_ms": 245
}
```

---

## 🔍 Code Review Checklist

- [ ] Code follows project style guide (Black, isort, mypy)
- [ ] Comments added for complex logic
- [ ] No hardcoded credentials or secrets
- [ ] All dependencies in `requirements.txt` with pinned versions
- [ ] Dockerfile builds successfully
- [ ] CI/CD pipeline passes
- [ ] Documentation updated (README, docstrings, comments)
- [ ] Backwards compatible or breaking change documented

---

## 🚀 Deployment Notes

### Breaking Changes

- [ ] This PR contains breaking changes: ___________

### Migration Guide (if applicable)

```bash
# Commands to run on deployment
python scripts/migrate_db.py
docker build -t smartinventoryai:latest .
```

### Deployment Checklist

- [ ] Database migrations applied
- [ ] Environment variables documented in `.env.example`
- [ ] Secrets rotated (if applicable)
- [ ] Version bumped in relevant files
- [ ] CHANGELOG.md updated

---

## 📖 Documentation

### Documentation Updates

- [ ] README.md updated
- [ ] Docstrings added/updated
- [ ] CONTRIBUTING.md updated (if applicable)
- [ ] Setup guide updated
- [ ] API documentation updated

### Related Documentation

- Link to relevant docs: ___
- Link to related issues: ___

---

## 🤝 Reviewers

@Samuel-AI-Electronic-Engineer — Please review this PR

---

## Additional Notes

Any additional context or discussion points for reviewers:

---

## Checklist Before Merging

- [ ] All CI checks passing (GitHub Actions)
- [ ] At least 1 approval from code owner
- [ ] All conversations resolved
- [ ] Branch is up-to-date with main
- [ ] Tests added for new functionality
- [ ] Documentation updated
