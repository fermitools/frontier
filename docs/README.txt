
------------------------------------------------------------
$Id$
------------------------------------------------------------

In order to get the servlet to function properly, create a file called 
build.properties in the top-level directory of the webapp. 
In this file, put the following:

app.name=MyProject
app.version=1.1
catalina.home=/poldata1/blumenfe/jakarta-tomcat-4.1.27
manager.url=http://pollux.pha.jhu.edu:8080/manager

You will need to pick a project name, which will also be the context path in 
the URL:
http://pollux.pha.jhu.edu:8080/MyProject


You will also need to put these in the build.properties file:

manager.username=your_username
manager.password=your_password

although filling in the appropriate values for your_username and your_password.


The entire directory structure and files contained in CVS should be copied
to the top level directory of the webapp:

$CATALINA_HOME/webapps/MyProject

------------------------------------------------------------
CURRENT PROBLEMS
------------------------------------------------------------

This section should list known problems with the system. Please update
it when problems are fixed!


None known!?

------------------------------------------------------------
SOLVED PROBLEMS
------------------------------------------------------------
