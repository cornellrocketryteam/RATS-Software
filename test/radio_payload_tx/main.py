import machine
import utime
from machine import Pin

led = Pin("LED", Pin.OUT)

# Set up UART on the Raspberry Pi Pico
# Default pins: TX is GP0 (pin 1) and RX is GP1 (pin 2)
uart = machine.UART(0, baudrate=115200, tx=machine.Pin(0), rx=machine.Pin(1))

def main():

    print("Starting UART sender. Sending 'START' every 1 second...")
    

    
    while True:
        # Send the START command followed by a newline
        uart.write("START\n")
        
        # Flash LED to indicate transmission (optional)
        led.value(1)
        utime.sleep(0.1)
        led.value(0)
        
        # Wait for the remainder of the 1 second interval
        utime.sleep(0.9)
        
        print("Sent 'START' command")

# Run the main function when the script starts
if __name__ == "__main__":
    main()
