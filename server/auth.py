import os
import json
import requests
from flask import Flask, request, redirect, url_for, render_template_string
from spotipy import SpotifyOAuth
from werkzeug.middleware.proxy_fix import ProxyFix

app = Flask(__name__)
app.wsgi_app = ProxyFix(app.wsgi_app, x_proto=1, x_host=1)
app.secret_key = "SECRET-KEY"

# ==========================================
# CONFIGURATION
# ==========================================

SERVER_IP = "127.0.0.1"
PORT = 5001
BASE_URL = f"BASE_URL"

# File Paths
SPOTIFY_CACHE = ".cache"
OPENSKY_FILE = "opensky_creds.json"

# Spotify Config
SPOTIPY_CLIENT_ID = "SPOTIFY ID"
SPOTIPY_CLIENT_SECRET = "SPOTIFY SECRET"
SPOTIFY_SCOPE = "user-modify-playback-state user-read-playback-state user-read-currently-playing app-remote-control streaming playlist-read-private playlist-read-collaborative user-read-private"

HOME_TEMPLATE = """
<!DOCTYPE html>
<html>
<head>
    <title>Smart Display Auth Server</title>
    <style>
        body { font-family: sans-serif; max_width: 800px; margin: 40px auto; padding: 20px; text-align: center; }
        .card { border: 1px solid #ccc; padding: 20px; margin: 20px; border-radius: 8px; box-shadow: 2px 2px 5px rgba(0,0,0,0.1); }
        .status { font-weight: bold; }
        .success { color: green; }
        .missing { color: red; }
        button { padding: 10px 20px; font-size: 16px; cursor: pointer; background-color: #007bff; color: white; border: none; border-radius: 4px; }
        button:disabled { background-color: #ccc; }
        input { padding: 8px; margin: 5px; width: 200px; }
    </style>
</head>
<body>
    <h1>Smart Display Authentication Manager</h1>
    <p>Run this on your server, access it from your laptop.</p>

    <div class="card">
        <h2>1. Spotify</h2>
        <p>Status: <span class="status {{ 'success' if spotify_ok else 'missing' }}">{{ '✅ Connected' if spotify_ok else '❌ Not Connected' }}</span></p>
        {% if not spotify_ok %}
            <a href="/auth/spotify"><button>Connect Spotify</button></a>
        {% else %}
            <button disabled>Connected</button>
            <form action="/reset/spotify" method="post" style="display:inline;"><button style="background:red; font-size:12px;">Reset</button></form>
        {% endif %}
    </div>

    <div class="card">
        <h2>2. OpenSky Network</h2>
        <p>Status: <span class="status {{ 'success' if opensky_ok else 'missing' }}">{{ '✅ Connected' if opensky_ok else '❌ Not Connected' }}</span></p>
        {% if not opensky_ok %}
            <form action="/auth/opensky" method="post">
                <input type="text" name="client_id" placeholder="Client ID" required><br>
                <input type="password" name="client_secret" placeholder="Client Secret" required><br>
                <button type="submit">Save Credentials</button>
            </form>
        {% else %}
            <button disabled>Connected</button>
            <form action="/reset/opensky" method="post" style="display:inline;"><button style="background:red; font-size:12px;">Reset</button></form>
        {% endif %}
    </div>
</body>
</html>
"""

# Routes

@app.route("/")
def index():
    # Check which files exist
    spotify_ok = os.path.exists(SPOTIFY_CACHE)
    opensky_ok = os.path.exists(OPENSKY_FILE)

    return render_template_string(HOME_TEMPLATE,
                                  spotify_ok=spotify_ok,
                                  opensky_ok=opensky_ok)


@app.route("/auth/spotify")
def auth_spotify():
    sp_oauth = SpotifyOAuth(
        client_id=SPOTIPY_CLIENT_ID,
        client_secret=SPOTIPY_CLIENT_SECRET,
        redirect_uri=f"{BASE_URL}/callback/spotify",
        scope=SPOTIFY_SCOPE,
        open_browser=False
    )
    auth_url = sp_oauth.get_authorize_url()
    return redirect(auth_url)


@app.route("/callback/spotify")
def callback_spotify():
    sp_oauth = SpotifyOAuth(
        client_id=SPOTIPY_CLIENT_ID,
        client_secret=SPOTIPY_CLIENT_SECRET,
        redirect_uri=f"{BASE_URL}/callback/spotify",
        scope=SPOTIFY_SCOPE,
        open_browser=False,
        cache_path=SPOTIFY_CACHE  # Ensure it saves to the specific file
    )
    code = request.args.get("code")
    token_info = sp_oauth.get_access_token(code)
    # The library automatically saves to .cache when get_access_token is called
    return redirect("/")


@app.route("/reset/spotify", methods=["POST"])
def reset_spotify():
    if os.path.exists(SPOTIFY_CACHE): os.remove(SPOTIFY_CACHE)
    return redirect("/")


@app.route("/auth/opensky", methods=["POST"])
def auth_opensky():
    client_id = request.form.get("client_id")
    client_secret = request.form.get("client_secret")

    # Verify Credentials
    token_url = "https://auth.opensky-network.org/auth/realms/opensky-network/protocol/openid-connect/token"
    try:
        payload = {"grant_type": "client_credentials", "client_id": client_id, "client_secret": client_secret}
        response = requests.post(token_url, data=payload)

        if response.status_code == 200:
            with open(OPENSKY_FILE, "w") as f:
                json.dump({"client_id": client_id, "client_secret": client_secret}, f)
            return redirect("/")
        else:
            return f"<h1>Authentication Failed</h1><p>OpenSky rejected those credentials. <a href='/'>Try Again</a></p>"
    except Exception as e:
        return f"Error: {e}"


@app.route("/reset/opensky", methods=["POST"])
def reset_opensky():
    if os.path.exists(OPENSKY_FILE): os.remove(OPENSKY_FILE)
    return redirect("/")


if __name__ == "__main__":
    # 0.0.0.0 allows access from other computers on the network
    print(f"--- Starting Auth Server ---")
    print(f"1. Add '{BASE_URL}/callback/spotify' to Spotify Developer Dashboard Redirect URIs.")
    print(f"----------------------------")
    print(f"OPEN BROWSER TO: {BASE_URL}")

    app.run(host="0.0.0.0", port=PORT, debug=True)
