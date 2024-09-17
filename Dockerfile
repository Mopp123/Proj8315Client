FROM ubuntu:22.04
RUN apt-get update
#RUN apt-get install -y python3
#RUN apt-get install -y python3-dev
#RUN apt-get install -y python3-pip
RUN apt-get install -y nginx

#RUN apt-get install -y libpq-dev

WORKDIR /usr/src/app

#COPY ./myshowcase/requirements.txt /usr/src/app/requirements.txt
COPY ./nginx.conf /etc/nginx/sites-available/proj8315
RUN rm /etc/nginx/sites-enabled/default
RUN ln -s -t /etc/nginx/sites-enabled/ /etc/nginx/sites-available/proj8315
#RUN pip install -r requirements.txt
#RUN pip install gunicorn

EXPOSE 80
