FROM httpd:2.4
WORKDIR /usr/local/apache2
COPY site-conf/.htaccess /usr/local/apache2/.htaccess
COPY site-conf/mime.types /usr/local/apache2/conf/mime.types
COPY ./public-html/ /usr/local/apache2/htdocs/
EXPOSE 80
