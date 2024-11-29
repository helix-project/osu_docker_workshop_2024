# Docker build

We've seen how to use `docker run` to run pre-existing images. But what if we want to build our own? 

We can use `docker build` to build an image from a `Dockerfile`. A `Dockerfile` is essentailly a list of commands that need to be ran to install everything we want into our image and set up how things will work within the container.

Here is a glossary of Dockerfile commands:

* `FROM image_name:version`: What’s the base image we want to build on top of
* `RUN cmd`: How to run "cmd" during the build process
* `ADD <src> <dest>`: copy the src file on host to the dest location inside image
    * (`<src>` can be a url, compressed files will be decompressed)
* `COPY <src> <dest>`: copy the src file on host to the dest location inside image
* `CMD ["cmd", "arg1", ... "argn"]`: specifies the default run command (`cmd arg1 ... argn`)
* `ENTRYPOINT ["cmd", "arg1"]`: specifies the command ran before at start up before user commands
* `ENV MY_VAR=VALUE`: defines environmental variable
* `SHELL bash`: specifies the default shell
* `USER username`: change user
* `EXPOSE port`: expose a port to the outside world
* `ARG MY_ARG=VALUE`: Build time argument.


## Breaking down a Dockerfile example

Let's take a look at a Dockerfile that will compile the code [calculate_pi.cpp](./calculate_pi.cpp):
```Dockerfile
FROM ubuntu:latest

# Install dependencies gcc
RUN apt-get update 
RUN apt-get install -y  gcc g++
RUN rm -rf /var/lib/apt/lists/*

ENV NUM_STEPS=100000

# add in the files
COPY ./calculate_pi.cpp /build/calculate_pi.cpp

# compile the code
RUN g++ /build/calculate_pi.cpp -o /build/calculate_pi.o -fopenmp

CMD ["bash", "-c", "/build/calculate_pi.o ${NUM_STEPS}"]
```

### `FROM`

The `RUN` command specifies the base image used in this build. In the above case, we're using the `ubuntu:latest` image as our base image. This means that we'll be building upon the previously built image. 

### `RUN`

The `RUN` command specifies a command to run during the build stage. In this example we've ran:
```dockerfile
RUN apt-get update 
RUN apt-get install -y  g++
RUN rm -rf /var/lib/apt/lists/*
...
RUN g++ /build/calculate_pi.cpp -o /build/calculate_pi.o -fopenmp
```
The first 3 commands run `apt-get` to install packages in ubuntu and `rm` to remove some bloat files. 
The last command runs `g++` to compile our code.

### `ENV`

We can use `ENV` to specify the an environmental variable that will be used within the container. 
In this case we're setting `NUM_STEPS` to have the default value of `100000`. We can overwrite this when running using `-e NUM_STEPS=X`.

### `COPY`

We can use `COPY` to copy files from the host system into the container. In this case we copy our source code `./calculate_pi.cpp` into the container and to the location `/build/calculate_pi.cpp`

### `CMD`

We use `CMD` to set or overwrite the default run command. In this case we're running:
```dockerfile
CMD ["bash", "-c", "/build/calculate_pi.o ${NUM_STEPS}"]
```

This will be result in:
```bash
bash-c "/build/calculate_pi.o ${NUM_STEPS}"
```

## Building our image

Now that we have our `Dockerfile` setup, let's build it using `docker build`:
```bash
docker build -t obriens/calculate_pi:latest .
```

Let's break this down:
* `-t`: Here we're assigning the "tag" ` obriens/calculate_pi:latest` to the iamge. This will be the image name. Note if we omit the version (in this case `latest`) then the version will default to `latest`.
* `.`: Here we're giving the path to the build location (where the files are). We can give a full or relative path. Since we're in the same directory we can use `.` which is "here".

Notice that we've omitted a file name. By default `docker build` looks for a file called `Dockerfile`. As we will see, this can be overwritten.

## Layers and Caching

As we build, we notice that the build is performed on layers:
```
[+] Building 17.2s (11/11) FINISHED                              docker:default
 => [internal] load build definition from Dockerfile                       0.0s
 => => transferring dockerfile: 403B                                       0.0s
 => [internal] load metadata for docker.io/library/ubuntu:latest           0.0s
 => [internal] load .dockerignore                                          0.0s
 => => transferring context: 2B                                            0.0s
 => CACHED [1/6] FROM docker.io/library/ubuntu:latest                      0.0s
 => [internal] load build context                                          0.0s
 => => transferring context: 38B                                           0.0s
 => [2/6] RUN apt-get update                                               4.3s
 => [3/6] RUN apt-get install -y  gcc g++                                 11.7s
 => [4/6] RUN rm -rf /var/lib/apt/lists/*                                  0.1s 
 => [5/6] COPY ./calculate_pi.cpp /build/calculate_pi.cpp                  0.0s 
 => [6/6] RUN g++ /build/calculate_pi.cpp -o /build/calculate_pi.o -fopen  0.5s 
 => exporting to image                                                     0.5s 
 => => exporting layers                                                    0.5s 
 => => writing image sha256:160a4e827e2104de2ab42a781110a05175ec4f5bcc98e  0.0s 
 => => naming to docker.io/obriens/calculate_pi:latest                     0.0s
```

Let's modify `calculate_pi.cpp` and rerun the code:
```
[+] Building 0.6s (11/11) FINISHED                               docker:default
 => [internal] load build definition from Dockerfile                       0.0s
 => => transferring dockerfile: 403B                                       0.0s
 => [internal] load metadata for docker.io/library/ubuntu:latest           0.0s
 => [internal] load .dockerignore                                          0.0s
 => => transferring context: 2B                                            0.0s
 => [1/6] FROM docker.io/library/ubuntu:latest                             0.0s
 => [internal] load build context                                          0.0s
 => => transferring context: 1.31kB                                        0.0s
 => CACHED [2/6] RUN apt-get update                                        0.0s
 => CACHED [3/6] RUN apt-get install -y  gcc g++                           0.0s
 => CACHED [4/6] RUN rm -rf /var/lib/apt/lists/*                           0.0s
 => [5/6] COPY ./calculate_pi.cpp /build/calculate_pi.cpp                  0.0s
 => [6/6] RUN g++ /build/calculate_pi.cpp -o /build/calculate_pi.o -fopen  0.5s
 => exporting to image                                                     0.0s
 => => exporting layers                                                    0.0s
 => => writing image sha256:015addfc091276262c4d712f2927c71d89d20e311872e  0.0s
 => => naming to docker.io/obriens/calculate_pi:latest          
```
We can see that layers that haven't changed aren't reran. Instead we used the "cached" version of these layers. However we still need to rerun any layer after. This is something to keep in mind. If you have files that you'd like to change regularly, it's best to put them as late in the build process as possible.

Since we're building upon previous layers, the number of layers we use can have an impact on the final size of the image. Consider `Dockerfile-layers` example:
```dockerfile
FROM ubuntu:latest

# Install dependencies gcc
RUN apt-get update && apt-get install -y  \
    g++ && \
    rm -rf /var/lib/apt/lists/*

ENV NUM_STEPS=100000

# add in the files
COPY ./calculate_pi.cpp /build/calculate_pi.cpp

# compile the code
RUN g++ /build/calculate_pi.cpp -o /build/calculate_pi.o -fopenmp

CMD ["bash", "-c", "/build/calculate_pi.o ${NUM_STEPS}"]
```

In the above we've combined the dependecy installs into a single line, and as a result, a single stage. We can build this example and assign it to a new tag:
```bash
docker build -t obriens/calculate_pi:layers -f Dockerfile-layers .
```
Notice that we're using `-f filename` to specify the name of the dockerfile we're building from.

Let's look at the size of this image using `docker image ls`:
```
> docker image ls obriens/calculate_pi 
REPOSITORY             TAG       IMAGE ID       CREATED          SIZE
obriens/calculate_pi   layers    6c5f6486554b   13 seconds ago   360MB
obriens/calculate_pi   latest    015addfc0912   3 minutes ago    402MB
```

By combining these three steps together we've saved 40MB of space. This might not sound like much but small consistent efforts can save a lot of money in storage costs.

## Multistage builds

Let's say we have some dependency that is only needed for one part of the build, but not needed for the final product. This could be a compiler, a git repo or source code. Keeping these in the final image could add unnessicary bloat to the image. To avoid this we can use multi-stage builds. Let's take a look at the `Dockerfile-multi` file:
```dockerfile
FROM ubuntu:latest AS base

# Install dependencies gcc
RUN apt-get update && apt-get install -y \
    g++\
    && rm -rf /var/lib/apt/lists/*

# add in the files
COPY ./calculate_pi.cpp /build/calculate_pi.cpp

# compile the code
RUN g++ /build/calculate_pi.cpp -o /build/calculate_pi.o -fopenmp


FROM ubuntu:latest AS final
COPY --from=base /build/calculate_pi.o /app/calculate_pi.o
COPY --from=base /lib/x86_64-linux-gnu/libgomp.so.1 /lib/x86_64-linux-gnu/libgomp.so.1
ENV NUM_STEPS=100000

CMD ["bash", "-c", "/app/calculate_pi.o ${NUM_STEPS}"]
```

Here we're using two stages in our build. On the first line we specify that we're using the `ubuntu:latest` image, but we're calling this stage `base` using:
```dockerfile
FROM ubuntu:latest AS base
```

This is our first layer. We could of called this anything, `base` isn't a keyword.
In this stage we install `g++`, copy the source code `calculate_pi.cpp` into the container and then compile it using `g++`. As soon we've compiled `calculate_pi.o`, we no longer need `g++`.

It makes sense to make a new stage that just has the compiled code and what is needed to run it. Since we're using OpenMP (`-fopenmp`), we'll also need that library. So we create a new stage to our build called `final`:
```dockerfile
FROM ubuntu:latest AS final
```

We're again using the ubuntu image. Next we copy over what we need from the previous image:
```dockerfile
COPY --from=base /build/calculate_pi.o /app/calculate_pi.o
COPY --from=base /lib/x86_64-linux-gnu/libgomp.so.1 /lib/x86_64-linux-gnu/libgomp.so.1
```
Here we're using `--from=STAGENAME` to specify the stage that we're copying from. We copy across the compiled `calculate_pi.o`, but also the library `/lib/x86_64-linux-gnu/libgomp.so.1` (OpenMP is installed while installing `g++`).

We can build this with:
```bash
docker build -t obriens/calculate_pi:multi -f Dockerfile-multi .
```

Let's look at the sizes of all our `calculate_pi` images:
```bash
docker image ls obriens/calculate_pi
```

```
docker image ls obriens/calculate_pi

REPOSITORY             TAG           IMAGE ID       CREATED          SIZE
obriens/calculate_pi   multi         3f2b4f88f155   2 minutes ago    78.5MB
obriens/calculate_pi   latest        98a4b6284ff2   8 minutes ago    402MB
obriens/calculate_pi   layers        9451ff72301e   8 minutes ago    360MB
```

Using multi-stage builds we can dramatically reduce the size of our final images.



# Resource Management with Docker:

## CPU management

## Memory management