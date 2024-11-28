#!/bin/bash

echo "Building docker image..."
docker build . -t proj8315-client

echo "creating container..."
docker compose -f docker-compose.yml create
