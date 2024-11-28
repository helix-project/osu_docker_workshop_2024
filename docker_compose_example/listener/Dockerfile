FROM python:3.12-slim

ENV TARGET_URL="http://localhost:5000"

RUN pip install requests
COPY ./app /app

CMD ["python", "/app/app.py"]