FROM python:3.12-slim

# Default value for the flask port
ENV FLASK_PORT=5000

COPY ./requirements.txt /app/requirements.txt
RUN pip install -r /app/requirements.txt

COPY ./app /app

CMD ["python", "/app/app.py"]