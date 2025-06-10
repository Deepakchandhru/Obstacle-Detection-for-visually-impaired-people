from flask import Flask, request, jsonify, render_template_string

app = Flask(__name__)
last_message = ""

@app.route('/data', methods=['POST'])
def data():
    global last_message
    content = request.get_json()
    print("Received from ESP32:", content)
    last_message = content['message']
    return jsonify({"status": "received"}), 200

@app.route('/')
def home():
    return render_template_string("""
        <!DOCTYPE html>
        <html>
        <head>
            <title>ESP32 Sensor Data</title>
        </head>
        <body>
            <h1>ESP32 Sensor Data</h1>
            <p id="sensor">{{ message }}</p>

            <script>
                function speak(text) {
                    const utterance = new SpeechSynthesisUtterance(text);
                    window.speechSynthesis.speak(utterance);
                }

                function fetchData() {
                    fetch('/latest')
                        .then(response => response.json())
                        .then(data => {
                            document.getElementById('sensor').innerText = data.message;
                            speak(data.message);
                        });
                }

                setInterval(fetchData, 1000); 
            </script>
        </body>
        </html>
    """, message=last_message)

@app.route('/latest', methods=['GET'])
def latest():
    return jsonify({"message": last_message})

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
