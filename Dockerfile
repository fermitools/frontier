FROM registry.cern.ch/docker.io/tomcat:9.0

COPY build/dist/Frontier.war /usr/local/tomcat/webapps/
COPY context.xml /usr/local/tomcat/conf/Catalina/localhost/atlr.xml
EXPOSE 8080
CMD ["catalina.sh", "run"]
