console:
  cd console && pio run -t upload

monitor:
  cd console && pio run -t monitor

test-upload:
  uv run --with pyserial --with tqdm scripts/eeprom_upload.py --port /dev/ttyUSB0 --byte ea

OS:
  make -C 6502-os

upload: OS
  uv run --with pyserial --with tqdm scripts/eeprom_upload.py --port /dev/ttyUSB0 --filename 6502-os/build/OS.bin

format:
  find src -type f \( -name '*.cpp' -o -name '*.h' -o -name '*.hpp' -o -name '*.cc' -o -name '*.cxx' \) -exec clang-format -i {} +
