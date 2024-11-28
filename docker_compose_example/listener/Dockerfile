FROM python:3.12-slim

ENV TARGET_URL="http://localhost:5000"

COPY ./requirements.txt /app/requirements.txt 
RUN pip install -r /app/requirements.txt 
COPY ./app /app

CMD ["python", "/app/app.py"]