#!/bin/bash

pid=$( cat server.pid )
kill -SIGTERM $pid
tail -20 server.log

