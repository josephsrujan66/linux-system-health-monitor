#!/bin/bash

PROJECT_DIR="/home/kellton/Documents/linux-health-monitor"
LOG_DIR="$PROJECT_DIR/logs"

# Delete log files older than 7 days
find "$LOG_DIR" -type f -name "*.log" -mtime +7 -delete

# Create today's log directory if required
mkdir -p "$LOG_DIR"

echo "$(date): Maintenance completed" >> "$LOG_DIR/maintenance.log"
