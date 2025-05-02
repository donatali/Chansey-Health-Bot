import time
import serial

def configure_xbee_api_mode():
    # Define your XBee port
    PORT = "/dev/tty.usbserial-AL02BZKQ"  # Use the port we identified in your system
    BAUD_RATE = 9600  # Standard XBee baud rate
    
    print(f"Attempting to configure XBee on {PORT}")
    
    try:
        # Open serial connection
        print("Opening serial connection...")
        ser = serial.Serial(PORT, BAUD_RATE, timeout=2)
        print("Serial connection opened")
        
        # Enter AT command mode
        # First clear the buffer
        ser.reset_input_buffer()
        ser.reset_output_buffer()
        
        print("Entering command mode...")
        # Guard time before +++ (1 second of silence)
        time.sleep(1)
        ser.write(b'+++')
        # Guard time after +++ (1 second of silence)
        time.sleep(1)
        
        response = ser.read(3)
        print(f"Command mode response: {response}")
        
        if b'OK' in response:
            print("Successfully entered command mode")
            
            # Configure API mode
            print("Setting API mode (AP=1)...")
            ser.write(b'ATAP1\r')
            time.sleep(0.5)
            response = ser.read_all()
            print(f"API mode response: {response}")
            
            # Write changes permanently
            print("Writing changes permanently (WR)...")
            ser.write(b'ATWR\r')
            time.sleep(0.5)
            response = ser.read_all()
            print(f"Write response: {response}")
            
            # Exit command mode
            print("Exiting command mode (CN)...")
            ser.write(b'ATCN\r')
            time.sleep(0.5)
            
            print("Configuration complete - XBee should now be in API mode")
            print("Please restart your application to use the XBee in API mode")
            ser.close()
            return True
        else:
            print("Failed to enter command mode - no 'OK' response")
            
            # Try alternative approach - some XBees need different guard times
            print("Trying alternative approach...")
            ser.reset_input_buffer()
            ser.reset_output_buffer()
            
            # Longer guard time (2 seconds)
            time.sleep(2) 
            ser.write(b'+++')
            time.sleep(2)
            
            response = ser.read(3)
            print(f"Alternative approach response: {response}")
            
            if b'OK' in response:
                print("Successfully entered command mode with alternative approach")
                
                # Configure API mode
                print("Setting API mode (AP=1)...")
                ser.write(b'ATAP1\r')
                time.sleep(1)
                response = ser.read_all()
                print(f"API mode response: {response}")
                
                # Write changes permanently
                print("Writing changes permanently (WR)...")
                ser.write(b'ATWR\r')
                time.sleep(1)
                response = ser.read_all()
                print(f"Write response: {response}")
                
                # Exit command mode
                print("Exiting command mode (CN)...")
                ser.write(b'ATCN\r')
                time.sleep(1)
                
                print("Configuration complete - XBee should now be in API mode")
                print("Please restart your application to use the XBee in API mode")
                ser.close()
                return True
            else:
                print("Failed to enter command mode after multiple attempts")
                ser.close()
                return False
            
    except Exception as e:
        print(f"Error during configuration: {str(e)}")
        return False

if __name__ == "__main__":
    configure_xbee_api_mode()

