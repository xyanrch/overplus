
DOCKER_DEPS_IMAGE_BUILD_FLAGS?=--no-cache=true
build-docker-deps-image:
	docker build ${DOCKER_DEPS_IMAGE_BUILD_FLAGS} -t overplus/build_base:latest \
	  	-f ./buildFileEnv .

gen_cmake:
	docker run -it --rm --name=overplus \
	 	--mount type=bind,source=${PWD},target=/src \
		overplus/build_base:latest \
		bash -c \
		"mkdir -p build && \
		cd build && \
		cmake .."
build: gen_cmake
	ocker run -it --rm --name=overplus \
	 	--mount type=bind,source=${PWD},target=/src \
		overplus/build_base:latest \
		bash -c \
		"cd build && \
		 make " 