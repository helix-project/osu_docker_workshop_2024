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
