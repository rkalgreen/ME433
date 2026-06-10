import pygame
import serial
import math

# -----------------------------
# CONFIG
# -----------------------------
SERIAL_PORT = "COM6"     # Change to your Pico's port
BAUD = 115200

WIDTH, HEIGHT = 800, 600
WRENCH_LENGTH = 250
HANDLE_WIDTH = 20
HEAD_RADIUS = 40

# -----------------------------
# SERIAL SETUP
# -----------------------------
ser = serial.Serial(SERIAL_PORT, BAUD, timeout=0.01)

# -----------------------------
# PYGAME SETUP
# -----------------------------
pygame.init()
screen = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("Ratchet Wrench Rotation")
clock = pygame.time.Clock()

center = (WIDTH // 2, HEIGHT // 2)

# -----------------------------
# DRAW WRENCH FUNCTION
# -----------------------------
def draw_wrench(surface, angle_deg):
    angle_rad = math.radians(angle_deg)

    # Compute wrench endpoint
    x2 = center[0] + WRENCH_LENGTH * math.cos(angle_rad)
    y2 = center[1] - WRENCH_LENGTH * math.sin(angle_rad)

    # Draw handle
    pygame.draw.line(surface, (180, 180, 180), center, (x2, y2), HANDLE_WIDTH)

    # Draw ratchet head
    pygame.draw.circle(surface, (200, 200, 200), (int(x2), int(y2)), HEAD_RADIUS)

    # Draw inner circle (ratchet gear)
    pygame.draw.circle(surface, (120, 120, 120), (int(x2), int(y2)), HEAD_RADIUS // 2)

# -----------------------------
# MAIN LOOP
# -----------------------------
running = True
current_angle = 0

while running:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False

    # -------------------------
    # READ SERIAL VALUE
    # -------------------------
    try:
        line = ser.readline().decode().strip()
        if line.isdigit():
            val = int(line)
            val = max(0, min(4000, val))  # clamp
            current_angle = (val / 4000) * 200
    except:
        pass

    # -------------------------
    # DRAW
    # -------------------------
    screen.fill((30, 30, 30))
    draw_wrench(screen, current_angle)

    # Display angle text
    font = pygame.font.SysFont(None, 36)
    txt = font.render(f"{current_angle:.1f}°", True, (255, 255, 255))
    screen.blit(txt, (20, 20))

    pygame.display.flip()
    clock.tick(60)

pygame.quit()
