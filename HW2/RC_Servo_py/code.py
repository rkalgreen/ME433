import time
import pwmio

# Servo constants (microseconds)
MIN_PULSE = 500
MAX_PULSE = 2500
PERIOD_US = 20000  # 20ms = 20000us

def angle_to_duty(angle):
    """
    Convert angle (0-180) to PWM duty cycle.
    """
    pulse = MIN_PULSE + (angle / 180) * (MAX_PULSE - MIN_PULSE)
    duty = int(pulse / PERIOD_US * 65535)
    return duty

def sweep_servo(pin, start_angle=0, end_angle=180, step=1, delay=0.01):
    """
    Sweep a servo motor through angles using PWM.

    :param pin: The pin connected to the servo signal wire
    :param start_angle: Starting angle (default 0)
    :param end_angle: Ending angle (default 180)
    :param step: Angle increment per step (default 1)
    :param delay: Delay between steps in seconds (default 0.01)
    """
    # Initialize PWM
    pwm = pwmio.PWMOut(pin, frequency=50)
    while True:
        # Sweep from start to end
        for angle in range(start_angle, end_angle + step, step):
            pwm.duty_cycle = angle_to_duty(angle)
            time.sleep(delay)

        # Sweep back from end to start
        for angle in range(end_angle, start_angle - step, -step):
            pwm.duty_cycle = angle_to_duty(angle)
            time.sleep(delay) 

# Example usage (uncomment to test)
import board
sweep_servo(board.GP15)  #servo signal on GP15
 
