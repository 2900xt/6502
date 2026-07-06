#!/usr/bin/env python

import serial
import argparse
import time
from tqdm import tqdm

def main():
    parser = argparse.ArgumentParser(description="at28c256 eeprom programmer using ATMEGA 6502 console")
    parser.add_argument("filename", type=str, help="path to .BIN file to upload")
    parser.add_argument("--port", type=str, default='/dev/ttyUSB0', help="serial port running ATMEGA 6502 console")
    parser.add_argument("--baud", type=int, default=115200, help="baud rate to use")

    args = parser.parse_args()

    print("--- UPLOAD to AT28C256 ---")

    try:
        ser = serial.Serial(port=args.port, baudrate=args.baud, timeout=1)
    except:
        print(f"ERROR: cannot connect to '{args.port}'")
        exit(0)

    print(f"Connected to '{args.port}' @ {args.baud}")

    # initialize the reader
    try:
        resp = ser.read_until(b'>')
        assert resp.endswith(b'>') 
        
        ser.write(b'1')

        time.sleep(1)
        resp = ser.read_until(b'>')
        assert resp.endswith(b'>') and (b"OK" in resp)
    except:
        print(f"ERROR: chip init failed")
        exit(0)
    
    # read raw binary file
    try:
        with open(args.filename, 'rb') as file:
            bin = file.read()
    except:
        print(f"ERROR: file '{args.filename}' read error")
        exit(0)

    # assert file length = 32k
    if len(bin) != 0x8000:
        print(f"ERROR: file '{args.filename}' is not 32K bytes")
        exit(0)
    
    with tqdm(total=0x8000, unit="B", unit_scale=True, desc="Uploading") as pbar:
        for addr in range(len(bin)):
            byte = bin[addr]
            payload = f'w {addr:04x} {byte:02x}\r\n'
            ser.write(payload.encode("utf-8"))

            resp = ser.read_until(b'>')
            assert resp.endswith(b'>') and (b"OK" in resp)

            pbar.update(1)

    

if __name__ == "__main__":
    main()