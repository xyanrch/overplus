FROM alpine:3.11

COPY . overplus
RUN apk add --no-cache --virtual .build-deps \
        build-base \
        cmake \
        boost-dev \
        openssl-dev \
        mariadb-connector-c-dev \
    && (cd overplus && cmake . && make -j $(nproc) && strip -s overplus \
    && mv overplus /usr/local/bin) \
    && rm -rf overplus \
    && apk del .build-deps \
    && apk add --no-cache --virtual .overplus-rundeps \
        libstdc++ \
        boost-system \
        boost-program_options \
        mariadb-connector-c

WORKDIR /config
CMD ["overplus"]