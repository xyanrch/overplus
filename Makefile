
DOCKER_DEPS_IMAGE_BUILD_FLAGS?=--no-cache=true


gen_cmake:
	docker run -it --init --rm --memory-swap=-1 --ulimit core=-1 --name="overplus" \
	   --workdir=/srv \
       --mount type=bind,source=${PWD},target=/src \
        yanrongdocker/overplus_build_base:latest \
        bash -c \
       "mkdir -p /src/build && \
       cd /src/build && \
       cmake .."

build: gen_cmake
	docker run -it --init --rm --memory-swap=-1 --ulimit core=-1  --name="overplus" \
         --workdir=/src \
         --mount type=bind,source=${PWD},target=/src \
         yanrongdocker/overplus_build_base:latest \
         bash -c \
        "cd build && \
         make " 

build-docker-deps-image:
	docker build ${DOCKER_DEPS_IMAGE_BUILD_FLAGS} -t yanrongdocker/overplus_build_base:latest \
       -f ./buildFileEnv .