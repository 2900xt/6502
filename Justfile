console:
    cd console && pio run -t upload

monitor:
  cd console && pio run -t monitor

upload:
  uv run --with pyserial --with tqdm console/scripts/eeprom_upload.py --port /dev/ttyUSB0 --byte ea

format:
    find src -type f \( -name '*.cpp' -o -name '*.h' -o -name '*.hpp' -o -name '*.cc' -o -name '*.cxx' \) -exec clang-format -i {} +
