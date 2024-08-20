#!/bin/bash

# CONNECT TO SERVER AND COMMANDS

{
  echo "PASS 123"
  sleep 1
  echo "USER neves 0 * rodrigo"
  sleep 1
  echo "NICK big7"
  sleep 1
  echo "JOIN #channel"
  sleep 1
} | nc localhost 4555
