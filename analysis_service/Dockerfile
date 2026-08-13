FROM python:3.11-slim

WORKDIR /app

COPY analysis_service/requirements.txt ./requirements.txt
RUN pip install --no-cache-dir -r requirements.txt \
    && useradd --system --uid 1001 --no-create-home app \
    && chown -R app:app /app

COPY --chown=app:app analysis_service/app ./app

ENV DATA_DIR=/data
EXPOSE 8000

USER app

HEALTHCHECK --interval=10s --timeout=5s --start-period=10s --retries=5 \
  CMD python -c "import urllib.request,sys; sys.exit(0) if urllib.request.urlopen('http://127.0.0.1:8000/health',timeout=2).status==200 else sys.exit(1)"

CMD ["uvicorn", "app.main:app", "--host", "0.0.0.0", "--port", "8000"]
