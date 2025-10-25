import serial
import datetime
import time

def connect_to_serial(port, baudrate, timeout=1):
    """Attempt to connect to serial port with error handling"""
    while True:
        try:
            ser = serial.Serial(port, baudrate, timeout=timeout)
            print(f"Connected to {ser.name}")
            return ser
        except serial.SerialException as e:
            time.sleep(0.05)

def main():
    port = 'COM5'
    baudrate = 9600
    
    # Open log file
    log_file = open('data/exp2.csv', 'a')
    
    # Initial connection
    ser = connect_to_serial(port, baudrate)
    
    try:
        while True:
            try:
                # Check if there's data waiting
                if ser.in_waiting > 0:
                    # Read incoming data
                    data = ser.readline().decode('utf-8').strip()
                    
                    # Print to screen
                    print(data)
                    
                    # Save to log file
                    log_file.write(data + '\n')
                    log_file.flush()
                
                else:
                    # Small delay to prevent excessive CPU usage
                    time.sleep(0.01)
                    
            except serial.SerialException as e:
                print(f"Serial connection lost: {e}")
                print("Attempting to reconnect...")
                
                # Close the current connection
                try:
                    ser.close()
                except:
                    pass
                
                # Log the disconnection
                timestamp = datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')
                disconnect_message = f"[{timestamp}] SERIAL DISCONNECTED - Attempting reconnection..."
                print(disconnect_message)
                
                # Attempt to reconnect
                ser = connect_to_serial(port, baudrate)
                
                # Log successful reconnection
                timestamp = datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')
                reconnect_message = f"[{timestamp}] SERIAL RECONNECTED"
                print(reconnect_message)
                
            except UnicodeDecodeError as e:
                print(f"Unicode decode error: {e} - Skipping malformed data")
                continue
                
            except Exception as e:
                print(f"Unexpected error: {e}")
                time.sleep(1)
                continue

    except KeyboardInterrupt:
        print("\nStopping...")
    finally:
        try:
            ser.close()
        except:
            pass
        log_file.close()
        print("Serial port closed and log file saved.")

if __name__ == "__main__":
    main()