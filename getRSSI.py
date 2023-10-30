#import pyserial
import serial
import json

#label = location
label = input("Enter location: ")
file_idx = 0

# Open serial port /dev/ttyUSB0 baudrate=115200
ser = serial.Serial('/dev/ttyUSB0', 115200)

#Keep reading from serial port
global message
flag = 1
while(flag):
    line = ser.readline()
    line = line.decode().strip()
    if line ==  'start':
        print("Start of data")
        while True:
            line = ser.readline()
            line = line.decode().strip()
            if line == 'end':
                #the file name will be checked if file_name+idx.json exists and add 1 if it does
                while True:
                    try:
                        with open(f"{label}_{file_idx}.json") as f:
                            file_idx += 1
                    except FileNotFoundError:
                        break
                with open(f"{label}_{file_idx}.json",'w') as f:
                    data = message[:-1].split(';')
                    json.dump(data, f)
                print("End of data")
                flag -= 1
                break
            else:
                if(line != ''):
                    message = line
    else:
        print(line)
print("Done!")