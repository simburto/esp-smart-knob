from fastapi import FastAPI, Depends, HTTPException, status, Security, Response, Query
from fastapi.security import APIKeyHeader
from fastapi.responses import FileResponse
from pydantic import BaseModel
import spotipy
from spotipy.oauth2 import SpotifyOAuth
import os.path
import datetime
import json
import time
import requests
import struct
import asyncio
import httpx
from io import BytesIO
from PIL import Image
from datetime import datetime, timedelta
from google.oauth2 import service_account
from googleapiclient.discovery import build

app = FastAPI(title="Smart Display API")

# CONFIG

api_key_header = APIKeyHeader(name="API-Key", auto_error=False)
VALID_API_KEY = "API_KEY"

# Spotify Stats API
YOUR_SPOTIFY_API = "YOUR_SPOTIFY_API_BASE"
YOUR_TOKEN = "YOUR_SPOTIFY_TOKEN"

# Spotify Developer Creds
SPOTIPY_CLIENT_ID = "SPOTIFY_ID"
SPOTIPY_CLIENT_SECRET = "SPOTIFY_SECRET"
SPOTIPY_REDIRECT_URI = "http://127.0.0.1:8080"

# Google Calendar Config
SERVICE_ACCOUNT_FILE = 'SERVICE_ACCOUNT_FILE'
CALENDAR_IDS = [
    'CALENDAR_ID@group.calendar.google.com', # Labs/Tutorials
]
GOOGLE_SCOPES = ['https://www.googleapis.com/auth/calendar.readonly']


async def get_api_key(api_key_header: str = Security(api_key_header)):
    if api_key_header == VALID_API_KEY:
        return api_key_header
    else:
        raise HTTPException(
            status_code=status.HTTP_403_FORBIDDEN,
            detail="Could not validate credentials"
        )

# SERVICES

CACHE_DIR = "playlist_cache"
os.makedirs(CACHE_DIR, exist_ok=True)


# Spotify Setup 
sp = spotipy.Spotify(auth_manager=SpotifyOAuth(
    scope="user-modify-playback-state user-read-playback-state user-read-currently-playing app-remote-control streaming playlist-read-private playlist-read-collaborative user-read-private",
    client_id=SPOTIPY_CLIENT_ID,
    client_secret=SPOTIPY_CLIENT_SECRET,
    redirect_uri=SPOTIPY_REDIRECT_URI
))

# Google Calendar Setup 
def get_calendar_service():
    if not os.path.exists(SERVICE_ACCOUNT_FILE):
        raise HTTPException(status_code=500, detail="Missing 'service_account.json'.")
    
    try:
        creds = service_account.Credentials.from_service_account_file(
            SERVICE_ACCOUNT_FILE, scopes=GOOGLE_SCOPES
        )
        return build('calendar', 'v3', credentials=creds)
    except Exception as e:
        print(f"Calendar Auth Error: {e}")
        raise HTTPException(status_code=500, detail=f"Calendar Auth Error: {e}")


# OpenSky Network Setup
class OpenSkyAuth:
    def __init__(self, client_id, client_secret):
        self.client_id = client_id
        self.client_secret = client_secret
        self.token = None
        self.token_expiry = 0
        self.token_url = "https://auth.opensky-network.org/auth/realms/opensky-network/protocol/openid-connect/token"

    def get_headers(self):
        if not self.client_id or not self.client_secret:
            return {}
        if not self.token or time.time() >= self.token_expiry:
            self._refresh_token()
        return {"Authorization": f"Bearer {self.token}"}

    def _refresh_token(self):
        try:
            payload = {
                "grant_type": "client_credentials",
                "client_id": self.client_id,
                "client_secret": self.client_secret
            }
            response = requests.post(self.token_url, data=payload)
            if response.status_code == 200:
                data = response.json()
                self.token = data["access_token"]
                self.token_expiry = time.time() + data["expires_in"] - 60
            else:
                print(f"OpenSky Auth Failed: {response.text}")
        except Exception as e:
            print(f"OpenSky Error: {e}")

opensky_creds = {}
if os.path.exists("opensky_creds.json"):
    with open("opensky_creds.json") as f:
        opensky_creds = json.load(f)

opensky_auth = OpenSkyAuth(
    client_id=opensky_creds.get("client_id"),
    client_secret=opensky_creds.get("client_secret")
)

# Geocoding Helper
def get_location_coordinates(query: str):
    url = "https://nominatim.openstreetmap.org/search"
    params = {"q": query, "format": "json", "limit": 1}
    headers = {"User-Agent": "MySmartDisplay/1.0"}
    try:
        response = requests.get(url, params=params, headers=headers)
        data = response.json()
        if not data:
            raise HTTPException(status_code=404, detail=f"Location '{query}' not found.")
        location = data[0]
        return {
            "name": location["display_name"].split(",")[0],
            "full_name": location["display_name"],
            "latitude": float(location["lat"]),
            "longitude": float(location["lon"])
        }
    except Exception as e:
        if isinstance(e, HTTPException): raise e
        raise HTTPException(status_code=500, detail="Geocoding Service Failed")

# ENDPOINTS

@app.get("/update-data")
def get_secure_data(api_key: str = Depends(get_api_key)):
    try:
        playback = sp.current_playback()
        if not playback or not playback.get("item"):
            return {"is_active": False, "message": "Spotify inactive."}

        track = playback["item"]
        artist_names = ", ".join([artist["name"] for artist in track["artists"]])
        cover_art = track["album"]["images"][0]["url"] if track["album"]["images"] else None
        artist_uri = track["artists"][0]["id"]
 
        device_vol = playback.get("device", {}).get("volume_percent", 50)

        return {
            "is_active": True,
            "is_playing": playback["is_playing"],
            "progress_ms": playback["progress_ms"],
            "duration_ms": track["duration_ms"],
            "shuffle_state": playback["shuffle_state"],
            "repeat_state": playback["repeat_state"],
            "song_name": track["name"],
            "artist": artist_names,
            "cover_art": cover_art,
            "volume": device_vol,
            "artist_id": artist_uri
        }
    except Exception as e:
        print(f"Data Error: {e}")
        return {"is_active": False}

@app.get("/spotify/play")
def play_music(api_key: str = Depends(get_api_key)):
    try:
        sp.start_playback()
        return {"status": "success"}
    except:
        return {"status": "failed"}

@app.get("/spotify/pause")
def pause_music(api_key: str = Depends(get_api_key)):
    try:
        sp.pause_playback()
        return {"status": "success"}
    except:
        return {"status": "failed"}

@app.get("/spotify/next")
def next_track(api_key: str = Depends(get_api_key)):
    try:
        sp.next_track()
        return {"status": "success"}
    except:
        return {"status": "failed"}

@app.get("/spotify/prev")
def prev_track(api_key: str = Depends(get_api_key)):
    try:
        sp.previous_track()
        return {"status": "success"}
    except:
        return {"status": "failed"}

@app.get("/spotify/shuffle")
def set_shuffle(state: str, api_key: str = Depends(get_api_key)):
    try:
        should_shuffle = (state.lower() == 'true')
        sp.shuffle(should_shuffle)
        return {"status": "success", "shuffle": should_shuffle}
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@app.get("/spotify/repeat")
def set_repeat(state: str, api_key: str = Depends(get_api_key)):
    try:
        sp.repeat(state)
        return {"status": "success", "repeat": state}
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@app.get("/spotify/image")
def get_spotify_image(url: str, width: int = 100, height: int = 100, api_key: str = Depends(get_api_key)):
    try:
        res = requests.get(url)
        img = Image.open(BytesIO(res.content))
        img = img.resize((width, height))
        img = img.convert("RGB")
        
        raw_data = bytearray()
        for y in range(height):
            for x in range(width):
                r, g, b = img.getpixel((x, y))
                rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
                raw_data.extend(struct.pack(">H", rgb565))
                
        return Response(content=bytes(raw_data), media_type="application/octet-stream")
    except Exception as e:
        print(f"Image Error: {e}")
        return Response(status_code=500)

@app.get("/calendar/events")
def get_calendar_events(count: int = 5, api_key: str = Depends(get_api_key)):
    try:
        service = get_calendar_service()
        # Use UTC string with 'Z' for timeMin
        now = datetime.utcnow().isoformat() + 'Z'

        all_events = []

        # Loop through the hardcoded IDs
        for cal_id in CALENDAR_IDS:
            try:
                events_result = service.events().list(
                    calendarId=cal_id, 
                    timeMin=now, 
                    maxResults=count, # Fetch 'count' from EACH to ensure we have enough total
                    singleEvents=True, 
                    orderBy='startTime'
                ).execute()
                
                cal_summary = events_result.get('summary', cal_id) # Fallback to ID if no summary

                for item in events_result.get('items', []):
                    item['source_calendar'] = cal_summary
                    all_events.append(item)
                    
            except Exception as e:
                print(f"Error fetching calendar {cal_id}: {e}")
                continue

        # Sort all combined events by start time
        def get_start(e):
            return e['start'].get('dateTime', e['start'].get('date'))

        all_events.sort(key=get_start)

        # Format and Slice
        formatted = []
        for event in all_events[:count]:
            start = event['start'].get('dateTime', event['start'].get('date'))
            clean_start = start.replace('T', ' ')[:16]
            
            # Use the Calendar Title (e.g., "Labs") as the source
            source = event.get('source_calendar', 'Unknown')
            # If the source is your email, just say "Personal" or empty
            if '@' in source: source = "Personal"

            formatted.append({
                "calendar": source,
                "summary": event.get('summary', 'No Title'),
                "start": start, 
                "display_time": clean_start 
            })

        return {"count": len(formatted), "events": formatted}

    except Exception as e:
        print(f"Global Calendar Error: {e}")
        return {"count": 0, "events": []}

@app.get("/weather")
def get_weather(city: str = "London", api_key: str = Depends(get_api_key)):
    try:
        loc = get_location_coordinates(city)
        params = {
            "latitude": loc["latitude"],
            "longitude": loc["longitude"],
            "hourly": "temperature_2m,relative_humidity_2m,apparent_temperature,precipitation,wind_speed_10m",
            "forecast_days": 1,
            "timezone": "auto"
        }
        res = requests.get("https://api.open-meteo.com/v1/forecast", params=params).json()
        hourly = res.get("hourly", {})
        formatted_hours = []
        count = len(hourly.get("time", []))
        for i in range(count):
            formatted_hours.append({
                "time": hourly["time"][i],
                "temp_c": hourly["temperature_2m"][i],
                "feels_like_c": hourly["apparent_temperature"][i],
                "humidity": f"{hourly['relative_humidity_2m'][i]}%",
                "precip_mm": hourly["precipitation"][i],
                "wind_kph": hourly["wind_speed_10m"][i]
            })

        return {
            "city": loc["name"],
            "full_name": loc["full_name"],
            "forecast": formatted_hours
        }
    except Exception as e:
        print(e)
        raise HTTPException(status_code=500, detail=str(e))

@app.get("/flights/overhead")
def get_flights_overhead(city: str = "London", radius_km: int = 30, on_ground: bool = False,
                               api_key: str = Depends(get_api_key)):
    try:
        loc = get_location_coordinates(city)
        deg = radius_km / 111.0
        lamin, lamax = loc["latitude"] - deg, loc["latitude"] + deg
        lomin, lomax = loc["longitude"] - deg, loc["longitude"] + deg

        url = f"https://opensky-network.org/api/states/all?lamin={lamin}&lomin={lomin}&lamax={lamax}&lomax={lomax}"
        headers = opensky_auth.get_headers()

        response = requests.get(url, headers=headers)
        if response.status_code != 200:
            raise HTTPException(status_code=502, detail="OpenSky Network unreachable.")

        data = response.json()
        states = data.get("states", [])
        if not states:
            return {"city": loc["name"], "message": "No flights detected overhead."}

        formatted_flights = []
        hex_url = "https://api.adsbdb.com/v0/aircraft/"
        for s in states:
            is_on_ground = s[8]
            if not on_ground and is_on_ground: continue
            if s[13] is None and not is_on_ground: continue
            
            try:
                requrl = hex_url + s[0]
                resp = requests.get(requrl, timeout=2)
                type_code = ""
                if resp.status_code == 200:
                    type_code = resp.json().get("response", {}).get("aircraft", {}).get("type", "")
            except:
                type_code = ""

            formatted_flights.append({
                "callsign": s[1].strip() if s[1].strip() else "No Callsign",
                "country": s[2],
                "altitude_m": s[13],
                "velocity_ms": s[9],
                "on_ground": is_on_ground,
                "icao_code": s[0],
                "type": type_code
            })

        return {
            "city": loc["name"],
            "radius_km": radius_km,
            "flight_count": len(formatted_flights),
            "flights": formatted_flights
        }

    except requests.exceptions.RequestException as e:
        raise HTTPException(status_code=500, detail=f"Connection Error: {str(e)}")

def get_iso_date(dt):
    return dt.strftime('%Y-%m-%dT%H:%M:%S.000Z')

@app.get('/tracks/check')
def check_tracks(playlist_id: str):
    playlist_meta = sp.playlist(playlist_id, fields="snapshot_id")
    # Sanitize it immediately so the ESP32 gets a URL-safe string
    safe_snapshot = playlist_meta["snapshot_id"].replace("/", "_").replace("+", "-").replace("=", "")
    return {
        "snapshot_id": safe_snapshot
    }

@app.get('/tracks')
def get_tracks(playlist_id: str, snapshot_id: str = None):
    # If the ESP didn't provide a snapshot, look it up and sanitize it
    if not snapshot_id:
        playlist_meta = sp.playlist(playlist_id, fields="snapshot_id")
        snapshot_id = playlist_meta["snapshot_id"].replace("/", "_").replace("+", "-").replace("=", "")
        
    cache_file = os.path.join(CACHE_DIR, f"{playlist_id}_{snapshot_id}.json")
    
    # CACHE HIT: Send file directly
    if os.path.exists(cache_file):
        return FileResponse(cache_file, media_type='application/json')
            
    # CACHE MISS: Fetch new tracks
    tracks = []
    results = sp.playlist_items(playlist_id)
    tracks.extend(results['items'])
    while results['next']:
        results = sp.next(results)
        tracks.extend(results['items'])
        
    formatted_tracks = []
    for t in tracks:
        if t.get('track'):
            formatted_tracks.append({
                "id": t['track']['id'],
                "track_name": t['track']['name']
            })
            
    # Save to server's drive
    with open(cache_file, 'w') as f:
        json.dump(formatted_tracks, f)
        
    return formatted_tracks

@app.get("/playlists")
def get_playlists(api_key: str = Depends(get_api_key)):
    playlists = sp.current_user_playlists()["items"]
    playlist_data  = []
    for i in range(len(playlists)):
        playlist_data.append({"name": playlists[i]["name"], "track_count": playlists[i]["tracks"]["total"], "id": playlists[i]["id"]})
    return{
        "playlists" : playlist_data
    }

@app.get("/start_playback")
def start_playback(selected_songs: str, api_key: str = Depends(get_api_key)):
    selected_songs  = selected_songs.split()
    track_id = []
    for i in range(len(selected_songs)):
        track_id.append("spotify:track:" + selected_songs[i])
    sp.repeat("context")
    sp.start_playback(uris = track_id)
    return(track_id)


async def fetch_spotify_data(client, endpoint, start, end, limit=5, extra_params=None):
    url = f"{YOUR_SPOTIFY_API}/spotify/{endpoint}"
    params = {
        "start": start,
        "end": end,
        "token": YOUR_TOKEN,
        "nb": limit,
        "offset": 0
    }
    if extra_params:
        params.update(extra_params)

    try:
        resp = await client.get(url, params=params)
        if resp.status_code != 200:
            print(f"Stats API Error [{resp.status_code}] {endpoint}")
            return []
        return resp.json()
    except Exception as e:
        print(f"Stats Connection Error: {e}")
        return []

@app.get("/stats")
async def get_stats():
    now = datetime.utcnow() + timedelta(hours=5)
    dt_month = now.replace(day=1, hour=5, minute=0, second=0, microsecond=0)
    str_now = get_iso_date(now)
    str_month = get_iso_date(dt_month)
    
    async with httpx.AsyncClient() as client:
        results = await asyncio.gather(
            fetch_spotify_data(client, "top/artists", str_month, str_now),
            fetch_spotify_data(client, "top/songs", str_month, str_now),
            fetch_spotify_data(client, "time_per", str_month, str_now, extra_params={"timeSplit": "all"}),
        )

    def clean_artists(data_raw):
        target_list = data_raw
        if isinstance(data_raw, dict):
            target_list = data_raw.get("content") or data_raw.get("artists") or []
        if not isinstance(target_list, list): return []
        cleaned = []
        for item in target_list[:5]:
            name = item.get("name")
            if not name and "artist" in item: name = item["artist"].get("name")
            cleaned.append({"name": str(name or "Unknown"), "count": int(item.get("count", 0))})
        return cleaned

    def clean_songs(data_raw):
        target_list = data_raw
        if isinstance(data_raw, dict):
            target_list = data_raw.get("content") or data_raw.get("songs") or []
        if not isinstance(target_list, list): return []
        cleaned = []
        for item in target_list[:5]:
            name = None
            if "track" in item and isinstance(item["track"], dict):
                name = item["track"].get("name")
            if not name: name = item.get("name")
            cleaned.append({"name": str(name or "Unknown"), "count": int(item.get("count", 0))})
        return cleaned

    total_minutes = 0
    if results[2] and isinstance(results[2], list) and len(results[2]) > 0:
         total_minutes = round(results[2][0].get("count", 0) / 60000)

    return {
        "month": {
            "minutes": total_minutes,
            "artists": clean_artists(results[0]),
            "tracks": clean_songs(results[1])
        }
    }

@app.get("/artist/stats")
async def get_artist_stats(artist_id: str, api_key: str = Depends(get_api_key)):
    url = f"{YOUR_SPOTIFY_API}/artist/{artist_id}/stats"
    params = {"token": YOUR_TOKEN}
    default_response = {"count": 0, "first": None}
    if not artist_id:
        return default_response

    try:
        async with httpx.AsyncClient() as client:
            
            resp = await client.get(url, params=params, timeout=10.0)
            if resp.status_code != 200:
                return default_response
    
            results = resp.json()
            total = results.get("total", {})
            first_last = results.get("firstLast", {})
            first = first_last.get("first", {})
    
            return {
                "count": total.get("count", 0),
                "first": first.get("played_at")
            }

    except Exception as e:
        print(f"Artist Stats Connection Error: {e}")
        return default_response
