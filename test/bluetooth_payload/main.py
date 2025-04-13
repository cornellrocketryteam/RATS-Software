import machine
import utime
from pico_a_bluetooth import main_bluetooth
from machine import Pin

led = Pin("LED", Pin.OUT)

# Set up UART on the Raspberry Pi Pico
# Default pins: TX is GP0 (pin 1) and RX is GP1 (pin 2)
uart = machine.UART(0, baudrate=115200, tx=machine.Pin(0), rx=machine.Pin(1))


def main():
    led.toggle()
    print("UART monitoring started. Waiting for 'START' command...")
    
    # Buffer to store incoming data
    buffer = ""
    
    while True:
        if uart.any():
            # Read available bytes
            data = uart.read(1)
            
            # Decode the byte to ASCII and add to buffer
            char = data.decode('utf-8')
            buffer += char
            
            # Check if the buffer contains a complete line
            if char == '\n' or char == '\r':
                # Strip whitespace and check for the START command
                command = buffer.strip()
                print(f"Received: {command}")
                
                if command == "START":
                    print("Radio message received. Starting Payload")
                    main_bluetooth()
                
                # Reset the buffer
                buffer = ""
        
        # Small delay to prevent hogging the CPU
        utime.sleep(0.01)

# Run the main function
if __name__ == "__main__":
    main()
