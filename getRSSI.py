#import pyserial
import serial
import json

#label = location
label = input("Enter location: ")
num_attempts = 10

# Open serial port /dev/ttyUSB0 baudrate=115200
ser = serial.Serial('/dev/ttyUSB0', 115200)

#Keep reading from serial port
global message
i=0
while(i<num_attempts):
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
                        with open(f"data/{label}_{i}.json") as f:
                            i += 1
                    except FileNotFoundError:
                        break
                with open(f"data/{label}_{i}.json",'w') as f:
                    data = message[:-1].split(';')
                    json.dump(data, f)
                print("End of data")
                break
            else:
                if(line != ''):
                    message = line
    else:
        print(line)
print("Done!")