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