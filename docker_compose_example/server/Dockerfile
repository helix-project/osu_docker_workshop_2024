FROM python:3.12-slim

# Default value for the flask port
ENV FLASK_PORT=5000

RUN pip install flask

COPY ./app /app

CMD ["python", "/app/app.py"]