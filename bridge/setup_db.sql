-- TSI-Telemetry Database Setup for TimescaleDB
-- Run this once to create the database and table

-- Create database (run this as postgres superuser)
-- CREATE DATABASE telemetry;

-- Connect to telemetry database, then run:

-- Enable TimescaleDB extension
CREATE EXTENSION IF NOT EXISTS timescaledb;

-- Create the metrics table
CREATE TABLE IF NOT EXISTS car_metrics (
    time            TIMESTAMPTZ NOT NULL,
    -- Core metrics
    rpm             INTEGER,
    speed           INTEGER,
    coolant_temp    INTEGER,
    intake_temp     INTEGER,
    throttle        INTEGER,
    engine_load     INTEGER,
    manifold_pressure INTEGER,
    fuel_level      INTEGER,
    timing_advance  INTEGER,
    boost_pressure  INTEGER DEFAULT 0,
    -- New transmission diagnosis metrics
    actual_gear     INTEGER DEFAULT 0,      -- Actual gear position (0=N, 1-7)
    battery_voltage INTEGER DEFAULT 0,      -- Control module voltage (V * 10)
    demanded_torque INTEGER DEFAULT 0,      -- Driver demanded torque %
    actual_torque   INTEGER DEFAULT 0,      -- Actual engine torque %
    fuel_rate       INTEGER DEFAULT 0,      -- Fuel rate (L/h * 10)
    torque_slip     INTEGER DEFAULT 0       -- Demanded - Actual torque (slip indicator)
);

-- Convert to hypertable (TimescaleDB magic for time-series)
SELECT create_hypertable('car_metrics', 'time', if_not_exists => TRUE);

-- Create index for faster queries
CREATE INDEX IF NOT EXISTS idx_car_metrics_time ON car_metrics (time DESC);

-- Migration: Add new columns to existing table (run if table already exists)
ALTER TABLE car_metrics ADD COLUMN IF NOT EXISTS actual_gear INTEGER DEFAULT 0;
ALTER TABLE car_metrics ADD COLUMN IF NOT EXISTS battery_voltage INTEGER DEFAULT 0;
ALTER TABLE car_metrics ADD COLUMN IF NOT EXISTS demanded_torque INTEGER DEFAULT 0;
ALTER TABLE car_metrics ADD COLUMN IF NOT EXISTS actual_torque INTEGER DEFAULT 0;
ALTER TABLE car_metrics ADD COLUMN IF NOT EXISTS fuel_rate INTEGER DEFAULT 0;
ALTER TABLE car_metrics ADD COLUMN IF NOT EXISTS torque_slip INTEGER DEFAULT 0;

-- Optional: Set up compression policy (compress data older than 7 days)
-- ALTER TABLE car_metrics SET (
--     timescaledb.compress,
--     timescaledb.compress_segmentby = ''
-- );
-- SELECT add_compression_policy('car_metrics', INTERVAL '7 days');

-- Optional: Set up retention policy (delete data older than 90 days)
-- SELECT add_retention_policy('car_metrics', INTERVAL '90 days');

-- Verify setup
\dt
\dx
