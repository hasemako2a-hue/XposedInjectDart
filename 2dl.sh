#!/bin/bash
(cd app/build/outputs/apk/debug/ && python3 -m http.server 8080)
