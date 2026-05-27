FROM python:3.11-slim

WORKDIR /app

COPY analysis_service/requirements.txt ./requirements.txt
RUN pip install --no-cache-dir -r requirements.txt

COPY analysis_service/app ./app

ENV DATA_DIR=/data
EXPOSE 8000

CMD ["uvicorn", "app.main:app", "--host", "0.0.0.0", "--port", "8000"]
