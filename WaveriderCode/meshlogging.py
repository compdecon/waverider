import meshtastic
import meshtastic.serial_interface
from pubsub import pub

# Callback function to handle incoming messages
def onReceive(packet, interface):
    # Check if it's a data packet
    if 'decoded' in packet and 'text' in packet['decoded']:
        sender = packet['fromId']
        text = packet['decoded']['text']
        print(f"Message from {sender}: {text}")
    # Print full packet info for debugging
    # print(f"Raw Packet: {packet}")

# Subscribe to the receive topic
pub.subscribe(onReceive, "meshtastic.receive.data")

# Start the interface (automatically connects to radio)
# Uses default serial port, or specify: meshtastic.serial_interface.SerialInterface(devPath="/dev/ttyUSB0")
interface = meshtastic.serial_interface.SerialInterface()

while True:
    pass # Keep the script running