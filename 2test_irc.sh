#!/bin/bash

# CONNECT TO SERVER AND COMMANDS

{
  echo "PASS 123"
  sleep 1
  echo "USER juvenaldo 0 * lilbro"
  sleep 1
  echo "NICK juvenas"
  sleep 1
  echo "JOIN #channel"
  sleep 1
} | nc localhost 4555
