import pygame
import serial
import time
from datetime import datetime
import os

# Initialize Pygame
pygame.init()

# Set up display
width, height = 480, 300
win = pygame.display.set_mode((width, height))
pygame.display.set_caption("Map Drawer")
win.fill((255, 255, 255))
pygame.display.update()

MAP_FOLDER = "shared"

# Initialize font
font = pygame.font.SysFont(None, 24)

# Initialize Serial Communication
if __name__ == '__main__':
    ser = serial.Serial('/dev/ttyUSB0', 9600, timeout=1)
    ser.reset_input_buffer()
    time.sleep(2)  # Give some time for the connection to establish
    ser.flushInput()

# Initial position and direction
x, y = width // 2, height // 2
direction = 'left'

# Direction mapping
direction_map = {
    'up': {'L90': 'left', 'R90': 'right'},
    'down': {'L90': 'right', 'R90': 'left'},
    'left': {'L90': 'down', 'R90': 'up'},
    'right': {'L90': 'up', 'R90': 'down'}
}

# Length conversion factor
pixel_per_cm = 0.1  # Modify this if needed

# Distance counter
total_distance = 0

# Function to sanitize and validate command
def sanitize_command(command):
    command = command.strip()
    if command.startswith('F'):
        try:
            # Extract numeric part only, discard any trailing non-digit characters
            distance_str = ''.join(filter(str.isdigit, command[1:]))
            distance = int(distance_str)
            return f'F{distance}'
        except ValueError:
            return None
    elif command.startswith('L90'):
        return 'L90'
    elif command.startswith('R90'):
        return 'R90'
    elif command == 'START':
        return 'START'
    elif command == 'END':
        return 'END'
    return None

# Function to draw the red bar and text
def draw_distance_bar(total_distance):
    # Red bar
    bar_height = 30
    bar_color = (255, 0, 0)
    pygame.draw.rect(win, bar_color, (0, height - bar_height, width, bar_height))

    # Text
    text_color = (255, 255, 255)
    text = font.render(f'Droga przebyta [cm] = {total_distance}', True, text_color)
    win.blit(text, (10, height - bar_height + 5))

# Function to save the current map as an image
def save_map():
    # Ensure the directory exists
    if not os.path.exists(MAP_FOLDER):
        os.makedirs(MAP_FOLDER)
    
    timestamp = datetime.now().strftime('%Y-%m-%d_%H-%M-%S')
    filename = os.path.join(MAP_FOLDER, f'{timestamp}.png')
    pygame.image.save(win, filename)
    print(f"Map saved as {filename}")

# Function to reset the map and distance counter
def reset_map():
    global x, y, total_distance
    win.fill((255, 255, 255))
    total_distance = 0
    x, y = width // 2, height // 2
    draw_distance_bar(total_distance)
    pygame.display.update()

# Main loop
running = True
while running:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False

    if ser.in_waiting > 0:
        command = ser.readline().decode().strip()
        sanitized_command = sanitize_command(command)
        print(f"Received command: {command}")  # Debugging line to check command
        print(f"Sanitized command: {sanitized_command}")  # Debugging line to check sanitized command

        if sanitized_command == 'START':
            continue

        if sanitized_command == 'END':
            save_map()
            reset_map()
            continue

        if sanitized_command and sanitized_command.startswith('F'):
            distance = int(sanitized_command[1:]) * pixel_per_cm
            total_distance += int(sanitized_command[1:])  # Add to total distance counter
            print(f"Moving {distance} pixels {direction}")  # Debugging line to check distance and direction
            print(f"Total distance traveled: {total_distance} cm")  # Print total distance traveled

            if direction == 'up':
                new_y = y - distance
                pygame.draw.line(win, (0, 0, 0), (x, y), (x, new_y), 2)
                y = new_y
            elif direction == 'down':
                new_y = y + distance
                pygame.draw.line(win, (0, 0, 0), (x, y), (x, new_y), 2)
                y = new_y
            elif direction == 'left':
                new_x = x - distance
                pygame.draw.line(win, (0, 0, 0), (x, y), (new_x, y), 2)
                x = new_x
            elif direction == 'right':
                new_x = x + distance
                pygame.draw.line(win, (0, 0, 0), (x, y), (new_x, y), 2)
                x = new_x

        elif sanitized_command in ['L90', 'R90']:
            print(f"Turning {sanitized_command}")  # Debugging line to check turning command
            direction = direction_map[direction][sanitized_command]

        pygame.display.update()

        # Clear the input buffer
        ser.flushInput()

    # Draw the red bar and text
    draw_distance_bar(total_distance)
    pygame.display.update()

pygame.quit()
ser.close()
