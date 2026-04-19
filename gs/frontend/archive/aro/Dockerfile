# ARO docker file (vite + react)
# for development only

FROM node:24-alpine

WORKDIR /app

COPY package*.json .

RUN npm ci

EXPOSE 5173

CMD ["npm", "run", "dev", "--", "--host", "0.0.0.0"]
