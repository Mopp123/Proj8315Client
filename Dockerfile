FROM httpd:2.4
WORKDIR /usr/local/apache2
COPY conf/.htaccess /usr/local/apache2/.htaccess
COPY ./public-html/ /usr/local/apache2/htdocs/
COPY conf/mime.types /usr/local/apache2/conf/mime.types
EXPOSE 80
