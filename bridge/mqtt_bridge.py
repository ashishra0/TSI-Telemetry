#!/usr/bin/env python3
"""
MQTT to TimescaleDB Bridge for TSI-Telemetry

Subscribes to HiveMQ Cloud and writes car telemetry to TimescaleDB.
Configured via environment variables or .env file.
"""

import os
import sys
import json
import time
import ssl
import paho.mqtt.client as mqtt
import psycopg2
from psycopg2 import OperationalError

# Load .env file if it exists
from pathlib import Path
env_file = Path(__file__).parent / '.env'
if env_file.exists():
    with open(env_file) as f:
        for line in f:
            line = line.strip()
            if line and not line.startswith('#') and '=' in line:
                key, value = line.split('=', 1)
                os.environ.setdefault(key.strip(), value.strip())

# ============================================
# Configuration from environment variables
# ============================================
# HiveMQ Cloud settings
MQTT_HOST = os.environ.get("MQTT_HOST", "34637ad922cb4854b4a024400343ccc6.s1.eu.hivemq.cloud")
MQTT_PORT = int(os.environ.get("MQTT_PORT", 8883))
MQTT_USERNAME = os.environ.get("MQTT_USERNAME", "")
MQTT_PASSWORD = os.environ.get("MQTT_PASSWORD", "")
MQTT_TOPIC = os.environ.get("MQTT_TOPIC", "car/telemetry")

# TimescaleDB settings
DB_HOST = os.environ.get("DB_HOST", "localhost")
DB_PORT = int(os.environ.get("DB_PORT", 5433))
DB_NAME = os.environ.get("DB_NAME", "telemetry")
DB_USER = os.environ.get("DB_USER", "postgres")
DB_PASSWORD = os.environ.get("DB_PASSWORD", "")

# ============================================
# Validate configuration
# ============================================
if not MQTT_USERNAME or not MQTT_PASSWORD:
    print("ERROR: MQTT_USERNAME and MQTT_PASSWORD are required for HiveMQ Cloud")
    sys.exit(1)

if not DB_PASSWORD:
    print("ERROR: DB_PASSWORD is required")
    sys.exit(1)

# ============================================
# Database connection with retry
# ============================================
def get_db_connection(max_retries=10, retry_delay=5):
    """Connect to database with retry logic."""
    for attempt in range(max_retries):
        try:
            conn = psycopg2.connect(
                host=DB_HOST,
                port=DB_PORT,
                database=DB_NAME,
                user=DB_USER,
                password=DB_PASSWORD
            )
            conn.autocommit = True
            print(f"Connected to database at {DB_HOST}:{DB_PORT}/{DB_NAME}")
            return conn
        except OperationalError as e:
            print(f"Database connection attempt {attempt + 1}/{max_retries} failed: {e}")
            if attempt < max_retries - 1:
                time.sleep(retry_delay)

    print("ERROR: Could not connect to database after multiple attempts")
    sys.exit(1)

# Global database connection
conn = None

# ============================================
# MQTT callbacks
# ============================================
def on_connect(client, userdata, flags, rc, properties=None):
    if rc == 0:
        print(f"Connected to HiveMQ Cloud at {MQTT_HOST}")
        client.subscribe(MQTT_TOPIC)
        print(f"Subscribed to topic: {MQTT_TOPIC}")
    else:
        print(f"MQTT connection failed with code: {rc}")

def on_disconnect(client, userdata, rc, properties=None):
    print(f"Disconnected from MQTT (rc={rc})")
    if rc != 0:
        print("Unexpected disconnect, will auto-reconnect...")

def on_message(client, userdata, msg):
    global conn
    try:
        payload = msg.payload.decode()
        data = json.loads(payload)

        # Map ESP32 JSON fields to database columns
        # Core metrics + new transmission diagnosis metrics
        cur = conn.cursor()
        cur.execute("""
            INSERT INTO car_metrics (
                time, rpm, speed, coolant_temp, intake_temp,
                throttle, engine_load, manifold_pressure, fuel_level,
                timing_advance, boost_pressure,
                actual_gear, battery_voltage, demanded_torque, actual_torque,
                fuel_rate, torque_slip
            )
            VALUES (NOW(), %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s)
        """, (
            data.get('rpm'),
            data.get('speed'),
            data.get('coolant_temp'),
            data.get('intake_temp'),
            data.get('throttle'),
            data.get('engine_load'),
            data.get('manifold_pressure'),
            data.get('fuel_level'),
            data.get('timing_advance'),
            data.get('boost_pressure', 0),
            data.get('actual_gear', 0),
            data.get('battery_voltage', 0),
            data.get('demanded_torque', 0),
            data.get('actual_torque', 0),
            data.get('fuel_rate', 0),
            data.get('torque_slip', 0)
        ))

        # Enhanced logging for transmission diagnosis
        gear = data.get('actual_gear', 0)
        torque_slip = data.get('torque_slip', 0)
        gear_str = f"G{gear}" if gear > 0 else "N"
        slip_warning = " [SLIP!]" if torque_slip > 10 else ""

        print(f"Inserted: rpm={data.get('rpm')}, speed={data.get('speed')}, "
              f"gear={gear_str}, torque_slip={torque_slip}%{slip_warning}")

    except json.JSONDecodeError as e:
        print(f"JSON parse error: {e}")
    except Exception as e:
        print(f"Error processing message: {e}")
        # Try to reconnect to database
        try:
            conn = get_db_connection(max_retries=3)
        except:
            pass

# ============================================
# Main
# ============================================
def main():
    global conn

    print("=" * 50)
    print("TSI-Telemetry MQTT Bridge")
    print("=" * 50)
    print(f"HiveMQ Cloud: {MQTT_HOST}:{MQTT_PORT}")
    print(f"Database: {DB_HOST}:{DB_PORT}/{DB_NAME}")
    print(f"Topic: {MQTT_TOPIC}")
    print("=" * 50)

    # Connect to database
    conn = get_db_connection()

    # Setup MQTT client with TLS for HiveMQ Cloud
    client = mqtt.Client(
        client_id="tsi-bridge",
        protocol=mqtt.MQTTv5,
        callback_api_version=mqtt.CallbackAPIVersion.VERSION2
    )

    # Enable TLS
    client.tls_set(tls_version=ssl.PROTOCOL_TLS)

    # Set credentials
    client.username_pw_set(MQTT_USERNAME, MQTT_PASSWORD)

    client.on_connect = on_connect
    client.on_disconnect = on_disconnect
    client.on_message = on_message

    # Connect and loop
    print(f"Connecting to HiveMQ Cloud...")
    client.connect(MQTT_HOST, MQTT_PORT, 60)

    # This will automatically reconnect on disconnect
    client.loop_forever()

if __name__ == "__main__":
    main()
