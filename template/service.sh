#!/system/bin/sh
until [ "$(getprop sys.boot_completed)" = "1" ]; do
    sleep 1
done
settings put system min_refresh_rate 120.0
settings put system peak_refresh_rate 120.0
