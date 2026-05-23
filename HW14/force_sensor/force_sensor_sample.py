import serial
import time
import numpy as np
import matplotlib.pyplot as plt
# from scipy import signal
from pathlib import Path

def find_pico_port():
    """Attempt to find the Pico's COM port"""
    import sys
    if sys.platform == 'win32':
        # Try common COM ports on Windows
        for port in range(1, 20):
            try:
                ser = serial.Serial(f'COM{port}', 115200, timeout=1)
                ser.close()
                return f'COM{port}'
            except:
                continue
    return None

def collect_data(port='COM3', num_samples=200):
    """
    Connect to Pico and collect sensor data
    
    Args:
        port: Serial port (e.g., 'COM3')
        num_samples: Number of samples to collect
    
    Returns:
        times, raw_values, filtered_values
    """
    print(f"Opening serial port {port}...")
    ser = serial.Serial(port, 115200, timeout=5)
    time.sleep(2)  # Wait for connection to establish
    
    # Clear any existing data in buffer
    ser.reset_input_buffer()
    
    # Send number of samples
    print(f"Sending {num_samples} samples request...")
    ser.write(f"{num_samples}\n".encode())
    ser.flush()
    
    # Read until we see the data header
    print("Waiting for data...")
    while True:
        line = ser.readline().decode('utf-8', errors='ignore').strip()
        if "Collected Data" in line:
            break
        time.sleep(0.01)
    
    # Skip the header lines
    ser.readline()  # "Sample  Time(ms)  Raw  Filtered"
    ser.readline()  # "------  --------  --------"
    
    # Read data
    times = []
    raw_values = []
    filtered_values = []
    
    print("Reading samples...")
    for i in range(num_samples):
        line = ser.readline().decode('utf-8', errors='ignore').strip()
        if not line:
            break
        
        try:
            parts = line.split()
            if len(parts) >= 4:
                sample_num = int(parts[0])
                time_ms = int(parts[1])
                raw = int(parts[2])
                filtered = int(parts[3])
                
                times.append(time_ms / 1000.0)  # Convert to seconds
                raw_values.append(raw)
                filtered_values.append(filtered)
        except ValueError:
            continue
    
    ser.close()
    print(f"Collected {len(times)} samples")
    
    return np.array(times), np.array(raw_values), np.array(filtered_values)

def compute_fft(signal_data, sample_rate):
    """
    Compute FFT and return frequencies and magnitudes
    
    Args:
        signal_data: Signal array
        sample_rate: Sampling rate in Hz
    
    Returns:
        frequencies, magnitudes
    """
    n = len(signal_data)
    
    # Apply Hann window to reduce spectral leakage
    windowed = signal_data * np.hanning(n)
    
    # Compute FFT
    fft_vals = np.fft.fft(windowed)
    magnitudes = np.abs(fft_vals[:n//2]) / (n/2)
    frequencies = np.fft.fftfreq(n, 1/sample_rate)[:n//2]
    
    return frequencies, magnitudes

def plot_results(times, raw_values, filtered_values, output_dir='HW14'):
    """Plot and save results"""
    
    # Create output directory if it doesn't exist
    Path(output_dir).mkdir(exist_ok=True)
    
    # Compute sampling rate
    dt = np.mean(np.diff(times))
    sample_rate = 1.0 / dt if dt > 0 else 80
    nyquist = sample_rate / 2
    
    print(f"\nSampling rate: {sample_rate:.1f} Hz")
    print(f"Nyquist frequency: {nyquist:.1f} Hz")
    
    # Compute FFTs
    freqs_raw, mag_raw = compute_fft(raw_values, sample_rate)
    freqs_filt, mag_filt = compute_fft(filtered_values, sample_rate)
    
    # Plot 1: Time-domain data
    plt.figure(figsize=(12, 6))
    plt.plot(times, raw_values, 'b-', label='Raw', alpha=0.7, linewidth=0.8)
    plt.plot(times, filtered_values, 'r-', label='Filtered (IIR)', linewidth=1.5)
    plt.xlabel('Time (s)')
    plt.ylabel('Sensor Value (counts)')
    plt.title('Force Sensor Data - Raw vs Filtered')
    plt.legend()
    plt.grid(True, alpha=0.3)
    plt.tight_layout()
    plt.savefig(f'{output_dir}/time_domain.png', dpi=150)
    print(f"Saved: {output_dir}/time_domain.png")
    plt.close()
    
    # Plot 2: FFT comparison (linear scale)
    plt.figure(figsize=(12, 6))
    plt.plot(freqs_raw[:len(freqs_raw)//2], mag_raw[:len(mag_raw)//2], 'b-', label='Raw FFT', linewidth=1)
    plt.plot(freqs_filt[:len(freqs_filt)//2], mag_filt[:len(mag_filt)//2], 'r-', label='Filtered FFT', linewidth=1)
    plt.axvline(x=25, color='g', linestyle='--', alpha=0.5, label='25-30 Hz noise band')
    plt.axvline(x=30, color='g', linestyle='--', alpha=0.5)
    plt.xlabel('Frequency (Hz)')
    plt.ylabel('Magnitude')
    plt.title('FFT - Linear Scale')
    plt.legend()
    plt.xlim([0, nyquist])
    plt.grid(True, alpha=0.3)
    plt.tight_layout()
    plt.savefig(f'{output_dir}/fft_linear.png', dpi=150)
    print(f"Saved: {output_dir}/fft_linear.png")
    plt.close()
    
    # Plot 3: FFT comparison (log scale for better dynamic range)
    plt.figure(figsize=(12, 6))
    plt.semilogy(freqs_raw[:len(freqs_raw)//2], mag_raw[:len(mag_raw)//2] + 1e-6, 'b-', label='Raw FFT', linewidth=1)
    plt.semilogy(freqs_filt[:len(freqs_filt)//2], mag_filt[:len(mag_filt)//2] + 1e-6, 'r-', label='Filtered FFT', linewidth=1)
    plt.axvline(x=25, color='g', linestyle='--', alpha=0.5, label='25-30 Hz noise band')
    plt.axvline(x=30, color='g', linestyle='--', alpha=0.5)
    plt.xlabel('Frequency (Hz)')
    plt.ylabel('Magnitude (log scale)')
    plt.title('FFT - Log Scale')
    plt.legend()
    plt.xlim([0, nyquist])
    plt.grid(True, alpha=0.3, which='both')
    plt.tight_layout()
    plt.savefig(f'{output_dir}/fft_log.png', dpi=150)
    print(f"Saved: {output_dir}/fft_log.png")
    plt.close()
    
    # Print some statistics
    print(f"\nSignal Statistics:")
    print(f"Raw - Mean: {np.mean(raw_values):.0f}, Std: {np.std(raw_values):.0f}")
    print(f"Filtered - Mean: {np.mean(filtered_values):.0f}, Std: {np.std(filtered_values):.0f}")
    
    # Find dominant frequencies
    idx_raw = np.argsort(mag_raw)[-5:][::-1]
    idx_filt = np.argsort(mag_filt)[-5:][::-1]
    print(f"\nTop 5 frequencies (Raw): {freqs_raw[idx_raw]}")
    print(f"Top 5 frequencies (Filtered): {freqs_filt[idx_filt]}")

def main():
    """Main function"""
    try:
        # Try to find port automatically
        port = find_pico_port()
        if not port:
            port = 'COM3'  # Default to COM3
        
        print(f"Using port: {port}")
        
        # Collect data
        num_samples = 300  # ~3.75 seconds at 80Hz
        times, raw_values, filtered_values = collect_data(port, num_samples)
        
        # Plot results
        plot_results(times, raw_values, filtered_values, output_dir='HW14')
        
        print("\n✓ Analysis complete! Check HW14/ folder for images.")
        
    except Exception as e:
        print(f"Error: {e}")
        print("Make sure the Pico is connected and try specifying the port manually:")
        print("  times, raw, filt = collect_data('COM3', 300)")

if __name__ == '__main__':
    main()
