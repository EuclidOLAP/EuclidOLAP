# docker build -t euclidolap:<version> .
# docker run -d -p 8760:8760 -p 8761:8761 --name euclidolap euclidolap:<version>
FROM centos:7.9.2009

RUN mkdir -p /usr/local/olap/data/
RUN mkdir -p /usr/local/olap/meta/

COPY demo-*.txt /usr/local/olap/
COPY euclid-svr /usr/local/olap/
COPY euclid-cli /usr/local/olap/

COPY data/* /usr/local/olap/data/
COPY meta/* /usr/local/olap/meta/

COPY docker-depending/jdk-17_linux-x64_bin.tar.gz /usr/local/
COPY docker-depending/olapweb-0.0.1-SNAPSHOT.jar /usr/local/

WORKDIR /usr/local/olap/

CMD tar zxf /usr/local/jdk-17_linux-x64_bin.tar.gz -C /usr/local/ \
&& rm -f /usr/local/jdk-17_linux-x64_bin.tar.gz \
&& echo 'export JAVA_HOME=/usr/local/jdk-17.0.3.1' >> /root/.bashrc \
&& echo 'export JRE_HOME=${JAVA_HOME}/jre' >> /root/.bashrc \
&& echo 'export CLASSPATH=.:${JAVA_HOME}/lib:${JRE_HOME}/lib' >> /root/.bashrc \
&& echo 'export PATH=${JAVA_HOME}/bin:$PATH' >> /root/.bashrc \
&& export JAVA_HOME=/usr/local/jdk-17.0.3.1 \
&& export JRE_HOME=${JAVA_HOME}/jre \
&& export CLASSPATH=.:${JAVA_HOME}/lib:${JRE_HOME}/lib \
&& export PATH=${JAVA_HOME}/bin:$PATH \
&& echo '#!/bin/bash' > /usr/local/olapweb-startd.sh \
&& echo 'java -jar /usr/local/olapweb-0.0.1-SNAPSHOT.jar --server.port=8761 > /usr/local/olapweb.log 2> /usr/local/olapweb-err.log &' >> /usr/local/olapweb-startd.sh \
&& chmod +x /usr/local/olapweb-startd.sh \
&& /usr/local/olapweb-startd.sh \
&& /usr/local/olap/euclid-svr