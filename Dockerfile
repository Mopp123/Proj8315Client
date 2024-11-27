FROM ubuntu:22.04
RUN apt-get update
RUN apt-get install -y nginx

WORKDIR /usr/src/app

COPY ./nginx.conf /etc/nginx/sites-available/proj8315
RUN rm /etc/nginx/sites-enabled/default
RUN ln -s -t /etc/nginx/sites-enabled/ /etc/nginx/sites-available/proj8315
# nginx doesn't have wasm mime type by default
RUN sed -i 's/types {/types {\n    application\/wasm    wasm;/g' /etc/nginx/mime.types

EXPOSE 80
