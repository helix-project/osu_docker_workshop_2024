# Docker Compose

In this example we'll look at Docker Compose. Docker compose allows us to deploy a container or "stack" of containers from a easy to read yaml file. 


To show how we can use Docker Compose to setup a containerized network, we'll create a stack of two containers. One "server" which will recieve responses and return a random state. The other container will be a "listener", which will send a request to our server and log the random state.

## Server

For the server, we'll use [flask](https://flask.palletsprojects.com/en/stable/tutorial/) to create a [REST API server](https://www.ibm.com/topics/rest-apis).

Let's look at the server [server/app/app.py](server/app/app.py) file:
```python
from flask import Flask, jsonify
import os
import random

app = Flask(__name__)

@app.route("/")
def hello():
    return "Hello World!"


@app.route('/status', methods=['GET'])
def get_tasks():

    tasks = [
        {
            'id': 1,
            'random_status': random.choice(['success', 'failure'])
        },
        {
            'id': 2,
            'random_status': random.choice(['success', 'failure'])
        }
    ]
    return jsonify({'tasks': tasks})


if __name__ == '__main__':
    app.run(host="0.0.0.0", port=os.environ["FLASK_PORT"], debug=True)
```

We're using `flask` to create our REST API server. We're also going to import `jsonify` from `flask` to automatically format what we're sending to json; `os` to get the environmental variable `FLASK_PORT` and `random` to get a random status (`success` or `failure`).

We define two "routes", these can be thought of addressed serviced by our server. By going to `address:port/` we'll recieve the text "Hello World". By going to `address:port/status` we'll receive a json file with two entries of dictionaries, each with a random status. 

If we want to run this container, setting the `FLASK_PORT` varible to 8001, we could use:
```bash
docker run -e FLASK_PORT=8001 --rm -it obriens:server
```
Navigating to [localhost:8001](localhost:8001), we hope to see "Hello World" however we will receive an "Unable to connect" error. 

This is because we haven't mapped the port from inside the container to outside the container. We can map the port using `-p PORT_OUTSIDE:PORT_INSIDE`, for example:
```bash
docker run -e FLASK_PORT=8001 -p 8024:8001 --rm -it obriens:server
```
Maps port 8001 (i.e. the `FLASK_PORT` value) to 8024, on the host system. Navigating to [localhost:8024](localhost:8024), we'll now see "Hello World". Navigating to [localhost:8024/status](localhost:8024/status) should give us a json file with random statuses.


## Listener

The listener program can be found in [listener/app/app.py](listener/app/app.py):
```python
import logging
import requests
import os
import time

# Get the URL from the environment variable
url = os.getenv('TARGET_URL')

logging.basicConfig(level=logging.INFO)

if url is None:
    logging.error('TARGET_URL environment variable is not set.')
else:

    while True:
        try:
            logging.info(f'Fetching data from {url}')
            # Make a GET request to the URL
            response = requests.get(url+'/status')
            response.raise_for_status()  # Raise an exception for HTTP errors
            
            # Convert response to JSON
            data = response.json()
            
            # Log the data
            data = data['tasks']
            for d in data:
                print (d)
                logging.info(f'Status of {d["id"]}: {d["random_status"]}')
        
        except requests.exceptions.RequestException as e:
            logging.error(f'Error fetching data from {url}: {e}')
        
        time.sleep(5)
```

We'll be using `logging` to handle to logging of data, `requests` to handle the "GET" request when reaching out to the server, `os` to get the environmental variable `TARGET_URL` and `time` to add a sleep step to our program loop.

If we lauch this container, we'd need to pass the `TARGET_URL`. The problem with this that the networking here is also containerized. We can create a new containerized network using
```bash
docker network create api_net
```

This creates a new network. We can then start the server container on the `api_net` network:
```bash
docker run -e FLASK_PORT=8001 -p 8024:8001 --rm -it --network=api_net --name  my_server obriens:server
```

Here we've created a new container that is on the `api_net` network, we've set the name of the container to `my_server` and the `FLASK_PORT` to 8001. We've also mapped port 8001 inside the container to 8024 outside the container. We can now try to connect the listener container:
```bash
docker run --rm -it --network=api_net --name my_listener  -e TARGET_URL=http://my_server:8001 obriens/listener
```
Here we've created the container, named it `my_listener`, added it to the `api_net` network and set the `TARGET_URL` to `http://my_server:8001`. Running this we can see that the two containers are able to talk to each other.

## Docker Compose

The above example involved a lot of steps, we needed to  build each image, create a network, and attach each container to it. We can do all this using Docker Compose. Let's look at [./docker-compose.yml](./docker-compose.yml):
```dockerfile
services:

  server:
    image: server
    build:
      context: ./server
      dockerfile: Dockerfile
    ports:
      - "5000:5000"
    environment:
      - FLASK_PORT=5000
    networks:
      - app_net
    volumes:
      - app_data:/app/data
      

  listener:
    image: listener
    build:
      context: ./listener
      dockerfile: Dockerfile
    environment:
      - TARGET_URL=http://server:5000
    networks:
      - app_net
  

networks:
  app_net:
    # Specify driver options
    driver: bridge


volumes:
  app_data:
    driver: local
```

We start off by defining `services`. Here we list our two containers as seperate services. Let's break down the first entry:
```dockerfile
  server:
    image: server
    build:
      context: ./server
      dockerfile: Dockerfile
    ports:
      - "5000:5000"
    environment:
      - FLASK_PORT=5000
    restart: on-failure
    networks:
      - app_net
    volumes:
      - app_data:/app/data
```
Here we specify the `image` as `server`, this will be the name of the image. We set the `build` `context` to be `./server`. This sets the directory that the container will be built from. THe `dockerfile` entry specifes the Dockerfile to use for the build. We map port `5000` within the container to `5000` outside the container and set the environmental variable `FLASK_PORT` to be 5000. We attach the container to a network called `app_net` and mount a volume `app_data` to `/app/data` inside the container. We also added a condition to `restart` the container `on-failure` meaning that if the container fails for some reason, it will be automatically restarted (see [here](https://github.com/compose-spec/compose-spec/blob/main/spec.md#restart) for more details).

After the `listener` container, we create a network called `app_data`:
```dockerfile
networks:
  app_net:
    # Specify driver options
    driver: bridge
```

We then create a docker volume for persistent storage:
```dockerfile
volumes:
  app_data:
    driver: local
```

With the docker file in place, we can build everything we need by running:
```bash
docker compose build
```

To run the container we can simply run:
```bash
docker compose up
```
We can see the stream of the logs appear infront of us. If we don't want the output to be attached to the terminal, we can detach it using `-d`:
```bash
docker compose up -d
```
These containers are now running quietly in the background. We can check to make sure their status by running:
```bash
docker compose ps
```
From the same directory. We can also read the logs using:
```bash
docker compose logs
```
We can shut down these containers using:
```bash
docker compose down
```
