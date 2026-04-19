# Backend Dockerfile
FROM python:3.11-slim
WORKDIR /app
COPY requirements.txt /app/
COPY /gs/backend /app/
RUN apt-get update && apt-get install -y gcc build-essential
RUN pip install -r requirements.txt
EXPOSE 8000

#CMD ["fastapi", "run", "gs/backend/main.py"]
CMD ["uvicorn", "gs.backend.main:app", "--host", "0.0.0.0", "--port", "8000"]
