import time
import sys
import glob
import serial
from digi.xbee.devices import XBeeDevice

def find_ports():
    """Find available serial ports on macOS"""
    ports = glob.glob('/dev/tty.*') + glob.glob('/dev/cu.*')
    return [p for p in ports if 'usbserial' in p or 'Bluetooth' in p or 'UART' in p]

def configure_xbee_api_mode(port, baud_rate=9600):
    """
    Configure an XBee device to use API mode (AP=1)
    
    Args:
        port (str): Serial port where XBee is connected
        baud_rate (int): Baud rate of the XBee device
    """
    print(f"Attempting to configure XBee on {port} at {baud_rate} baud...")
    
    # Try to directly use a serial connection first
    try:
        # Open serial connection
        ser = serial.Serial(port, baud_rate, timeout=2)
        print(f"Connected to {port}")
        
        # Enter AT command mode with +++ (wait 1s before and after)
        time.sleep(1)
        ser.write(b'+++')
        time.sleep(1)
        
        # Read response to check if we entered command mode
        response = ser.read(3)
        if b'OK' in response:
            print("Entered command mode successfully")
            
            # Set API mode
            ser.write(b'ATAP1\r')
            time.sleep(0.5)
            response = ser.read(5)
            print(f"ATAP1 response: {response}")
            
            # Write changes
            ser.write(b'ATWR\r')
            time.sleep(0.5)
            response = ser.read(5)
            print(f"ATWR response: {response}")
            
            # Exit command mode
            ser.write(b'ATCN\r')
            time.sleep(0.5)
            
            print("XBee configured to API mode. Please restart your application.")
            ser.close()
            return True
        else:
            print(f"Failed to enter command mode. Response: {response}")
            ser.close()
    except Exception as e:
        print(f"Serial approach failed: {str(e)}")
    
    # If direct serial approach fails, try with the XBee library
    try:
        # Initialize the XBee device
        xbee = XBeeDevice(port, baud_rate)
        
        # Try to open the device (this might fail if it's in AT mode)
        try:
            xbee.open(force_settings=True)
            print("Opened XBee device")
        except Exception as e:
            print(f"Warning when opening XBee: {str(e)}")
        
        # Try to enter AT command mode
        try:
            if xbee._enter_at_command_mode():
                print("Entered AT command mode with XBee library")
                
                # Send the API mode command (AP=1)
                xbee._serial_port.write(b"ATAP1\r")
                time.sleep(0.5)
                response = xbee._serial_port.read_existing()
                print(f"ATAP1 response: {response}")
                
                # Save changes permanently
                xbee._serial_port.write(b"ATWR\r")
                time.sleep(0.5)
                response = xbee._serial_port.read_existing()
                print(f"ATWR response: {response}")
                
                # Exit command mode
                xbee._serial_port.write(b"ATCN\r")
                time.sleep(0.5)
                
                print("XBee configured to API mode. Please restart your application.")
                return True
            else:
                print("Failed to enter AT command mode with XBee library")
        except Exception as e:
            print(f"Error in AT command mode: {str(e)}")
    except Exception as e:
        print(f"Error with XBee library approach: {str(e)}")
    finally:
        if 'xbee' in locals() and xbee.is_open():
            xbee.close()
    
    return False

if __name__ == "__main__":
    ports = find_ports()
    print(f"Found the following ports: {ports}")
    
    success = False
    for port in ports:
        if configure_xbee_api_mode(port):
            success = True
            break
    
    if not success:
        print("\nFailed to configure XBee on any port. Please try:")
        print("1. Ensure your XBee is properly connected")
        print("2. Try configuring manually with XCTU software")
        print("3. If you know the exact port, run: python configure_xbee.py /dev/your_port_name")
        sys.exit(1)
    else:
        print("\nXBee configuration successful! You can now run your application.")
        sys.exit(0)

