FROM python:3.12-slim

WORKDIR /app

COPY main.py ./
COPY api/ ./api/
COPY models/ ./models/
COPY motion/ ./motion/

EXPOSE 8000

CMD ["python", "-u", "main.py"]
