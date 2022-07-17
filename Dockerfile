FROM ubuntu:18.04
LABEL Description="overplus docker image"
ENV HOME /root

SHELL ["/bin/bash", "-c"]

COPY build/overplus /usr/local/bin/overplus

RUN apt-get update && apt-get -y --no-install-recommends install \
    build-essential \
    libssl-dev 

# Let us add some heavy dependency
RUN cd ${HOME} && \
    wget --no-check-certificate --quiet \
        https://boostorg.jfrog.io/artifactory/main/release/1.66.0/source/boost_1_66_0.tar.gz && \
        tar xzf ./boost_1_66_0.tar.gz && \
        cd ./boost_1_66_0 && \
        ./bootstrap.sh && \
        ./b2 install && \
        cd .. && \
        rm -rf ./boost_1_66

RUN mkdir -p /etc/overplus 

COPY ConfigTemplate/server.json /etc/overplus/server.json
COPY ConfigTemplate/cert.crt  /etc/overplus/cert.crt
COPY ConfigTemplate/cert.key  /etc/overplus/cert.key
    


#WORKDIR /config
CMD ["overplus","/etc/overplus/server.json"]