import pygame
import pgzrun
import os
import ctypes
import serial
from random import randint
from time import time

ser = serial.Serial('COM6',115200, timeout=0.5) 
print('Opening port: ')
print(ser.name)

# Get screen dimensions
try:
    user32 = ctypes.windll.user32
    screen_width = user32.GetSystemMetrics(0)
    screen_height = user32.GetSystemMetrics(1)
except:
    screen_width = 1920
    screen_height = 1080

WIDTH = 600
HEIGHT = 600

# Game constants
BIRD_X = 100
BIRD_SIZE = 15
GRAVITY = 0.5
FLAP_STRENGTH = -5
PIPE_WIDTH = 50
PIPE_GAP = 150
PIPE_SPEED = 3
SPAWN_DISTANCE = 200

# Game state
class Game:
    def __init__(self):
        self.reset()
    
    def reset(self):
        self.bird_y = HEIGHT // 2
        self.bird_vy = 0
        self.pipes = []
        self.score = 0
        self.game_over = False
        self.started = False
        self.start_time = time()
    
    def flap(self):
        if not self.game_over:
            self.bird_vy = FLAP_STRENGTH
    
    def spawn_pipe(self):
        gap_y = randint(80, HEIGHT - 80 - PIPE_GAP)
        self.pipes.append({
            'x': WIDTH,
            'top': gap_y,
            'bottom': gap_y + PIPE_GAP,
            'scored': False
        })
    
    def update(self):
        # Check if 1 second has passed to start the game
        if not self.started and time() - self.start_time >= 1:
            self.started = True
        
        if self.game_over or not self.started:
            return
        
        # Update bird physics
        self.bird_vy += GRAVITY
        self.bird_y += self.bird_vy
        
        # Check bird bounds (top/bottom collision)
        if self.bird_y - BIRD_SIZE < 0 or self.bird_y + BIRD_SIZE > HEIGHT:
            self.game_over = True
            return
        
        # Spawn new pipe when it's time
        if len(self.pipes) == 0 or WIDTH - self.pipes[-1]['x'] > SPAWN_DISTANCE:
            self.spawn_pipe()
        
        # Update pipes
        for pipe in self.pipes[:]:
            pipe['x'] -= PIPE_SPEED
            
            # Remove off-screen pipes
            if pipe['x'] + PIPE_WIDTH < 0:
                self.pipes.remove(pipe)
                continue
            
            # Check collision with pipe
            if (BIRD_X + BIRD_SIZE > pipe['x'] and 
                BIRD_X - BIRD_SIZE < pipe['x'] + PIPE_WIDTH):
                if (self.bird_y - BIRD_SIZE < pipe['top'] or 
                    self.bird_y + BIRD_SIZE > pipe['bottom']):
                    self.game_over = True
                    return
            
            # Award point for passing pipe
            if not pipe['scored'] and pipe['x'] + PIPE_WIDTH < BIRD_X:
                pipe['scored'] = True
                self.score += 1

game = Game()
window_centered = False

# INPUT HANDLING - Easy to modify for external button
last_flap_time = 0

def get_input():
    """
    Check if the player pressed the flap button.
    Reads from Pico serial port without blocking the game loop.
    """
    global last_flap_time
    
    # Check spacebar
    if keyboard.SPACE:
        return True
    
    # Check serial input from Pico (non-blocking)
    try:
        if ser.in_waiting > 0:
            flap_button = ser.readline().decode().strip()
            if flap_button == 'FLAP':
                # # Simple debounce: ignore if we got a flap in the last 100ms
                # current_time = time()
                # if current_time - last_flap_time > 0.1:
                #     last_flap_time = current_time
                return True
    except:
        pass
    
    return False

def update():
    if get_input():
        game.flap()
    game.update()

def draw():
    global window_centered
    
    # Center window on first draw
    if not window_centered:
        try:
            window_x = max(0, (screen_width - WIDTH) // 2)
            window_y = max(0, (screen_height - HEIGHT) // 2)
            # Get the pygame window handle and move it
            hwnd = ctypes.windll.user32.GetActiveWindow()
            ctypes.windll.user32.MoveWindow(hwnd, window_x, window_y, WIDTH, HEIGHT, True)
        except Exception as e:
            print(f"Could not center window: {e}")
        window_centered = True
    
    screen.fill((135, 206, 235))  # Sky blue
    
    # Draw pipes
    for pipe in game.pipes:
        # Top pipe
        pygame.draw.rect(screen.surface, (34, 139, 34), 
                        (pipe['x'], 0, PIPE_WIDTH, pipe['top']))
        # Bottom pipe
        pygame.draw.rect(screen.surface, (34, 139, 34),
                        (pipe['x'], pipe['bottom'], PIPE_WIDTH, HEIGHT - pipe['bottom']))
    
    # Draw bird
    screen.draw.filled_circle((BIRD_X, int(game.bird_y)), BIRD_SIZE, color=(255, 255, 0))
    
    # Draw score
    screen.draw.text(f"Score: {game.score}", (10, 10), fontsize=30, color=(0, 0, 0))
    
    # Draw startup delay message
    if not game.started:
        time_left = max(0, 1 - (time() - game.start_time))
        screen.draw.text(f"Get Ready! {time_left:.1f}", (WIDTH//2 - 80, HEIGHT//2), 
                        fontsize=30, color=(0, 0, 0))
    
    # Draw game over message
    if game.game_over:
        screen.draw.text("GAME OVER! Press R to restart", (WIDTH//2 - 150, HEIGHT//2), 
                        fontsize=20, color=(255, 0, 0))

def on_key_down(key):
    if key == pygame.K_r and game.game_over:
        game.reset()

pgzrun.go()