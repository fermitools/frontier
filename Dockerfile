FROM tomcat:9.0

COPY build/dist/Frontier.war /usr/local/tomcat/webapps/
COPY context.xml /usr/local/tomcat/conf/context.xml
EXPOSE 8080
CMD ["catalina.sh", "run"]
