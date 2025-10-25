import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from scipy import stats
from scipy.optimize import curve_fit
import os

EXP_NUMBER = 2

# Moving average window size (set this to control smoothing)
WINDOW_SIZE = 25 if EXP_NUMBER == 1 else 75

DATA_FILE = f'data/exp{EXP_NUMBER}.csv'
OUT_FILE = f'plots/preliminary_results_exp{EXP_NUMBER}.pdf'

# Create plots directory if it doesn't exist
os.makedirs('plots', exist_ok=True)

# Read the CSV file
df = pd.read_csv(DATA_FILE, comment='#')

# Extract data
iterations = df['iteration'].values
n_ticks = df['n_ticks'].values

# Quantization parameters
quantization_step = 16
quantization_variance = (quantization_step**2) / 12  # Variance of uniform quantization noise

print("\nCalculating moving average...")
print("-" * 70)

try:
    # Calculate moving average using pandas (centered window)
    df_temp = pd.DataFrame({'n_ticks': n_ticks})
    moving_avg = df_temp['n_ticks'].rolling(window=WINDOW_SIZE, center=True, min_periods=1).mean().values
    
    # Detrend: remove moving average, keep mean
    mean_n_ticks = np.mean(n_ticks)
    n_ticks_detrended = n_ticks - (moving_avg - mean_n_ticks)
    
    # Calculate variance (with quantization correction)
    var_measured = np.var(n_ticks_detrended)
    var_real = max(0, var_measured - quantization_variance)
    std_real = np.sqrt(var_real)
    
    results = {
        'name': 'Moving Average',
        'fitted': moving_avg,
        'detrended': n_ticks_detrended,
        'variance': var_real,
        'std': std_real,
        'window_size': WINDOW_SIZE
    }
    
    print(f"Moving Average (window={WINDOW_SIZE}): Variance = {var_real:8.4f}, Std = {std_real:6.4f}")
        
except Exception as e:
    print(f"Moving average calculation failed - {e}")

print("-" * 70)
print()


fitted_line = results['fitted']
n_ticks_detrended = results['detrended']
fit_type = results['name']

# Calculate final statistics
mean_n_ticks = np.mean(n_ticks)
mean_detrended = np.mean(n_ticks_detrended)

# Measured variances
var_original_measured = np.var(n_ticks)
var_detrended_measured = np.var(n_ticks_detrended)

# Real variances (subtracting quantization noise)
var_original = max(0, var_original_measured - quantization_variance)
var_detrended = max(0, var_detrended_measured - quantization_variance)

# Standard deviations
std_original = np.sqrt(var_original)
std_detrended = np.sqrt(var_detrended)

# R ratios
R_original = mean_n_ticks / var_original if var_original > 0 else np.inf
R_detrended = mean_detrended / var_detrended if var_detrended > 0 else np.inf

# Create 2x2 figure
fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2, figsize=(14, 10))

# Original data with fit
ax1.plot(iterations, n_ticks, 'o', alpha=0.6, markersize=4, label='Data points')
ax1.plot(iterations, fitted_line, 'r-', linewidth=2, label=f'{fit_type} (window={WINDOW_SIZE})')
ax1.fill_between(iterations, fitted_line - std_original, 
                  fitted_line + std_original, alpha=0.3, color='red', 
                  label=f'±1σ ({std_original:.2f})')
ax1.set_xlabel('Iteration')
ax1.set_ylabel('Number of ticks')
ax1.set_title(f'Original Data with {fit_type} (σ = {std_original:.2f})')
ax1.legend()
ax1.grid(True, alpha=0.3)

# Detrended data
ax2.plot(iterations, n_ticks_detrended, 'o', alpha=0.6, markersize=4, color='green')
ax2.axhline(mean_detrended, color='r', linestyle='--', linewidth=2, label='Mean')
ax2.fill_between(iterations, mean_detrended - std_detrended, 
                  mean_detrended + std_detrended, alpha=0.3, color='green',
                  label=f'±1σ ({std_detrended:.2f})')
ax2.set_xlabel('Iteration')
ax2.set_ylabel('Number of ticks (detrended)')
ax2.set_title(f'Temperature-Corrected Data (σ = {std_detrended:.2f})')
ax2.legend()
ax2.grid(True, alpha=0.3)

# Histogram: Original
ax3.hist(n_ticks, bins=30, edgecolor='black', alpha=0.7)
ax3.axvline(mean_n_ticks, color='r', linestyle='--', linewidth=2)
ax3.set_xlabel('Number of ticks')
ax3.set_ylabel('Frequency')
ax3.set_title(f'Original Distribution (σ = {std_original:.2f})')
ax3.grid(True, alpha=0.3)

# Histogram: Detrended
ax4.hist(n_ticks_detrended, bins=30, edgecolor='black', alpha=0.7, color='green')
ax4.axvline(mean_detrended, color='r', linestyle='--', linewidth=2)
ax4.set_xlabel('Number of ticks (detrended)')
ax4.set_ylabel('Frequency')
ax4.set_title(f'Detrended Distribution (σ = {std_detrended:.2f})')
ax4.grid(True, alpha=0.3)

plt.tight_layout()

# Save the plot

plt.savefig(OUT_FILE, dpi=300, bbox_inches='tight')
print(f"Plot saved to {OUT_FILE}")

plt.show()

# Print statistics
print(f"\n{fit_type} (window={WINDOW_SIZE}) selected")
print(f"\n=== Original Data ===")
print(f"Mean: {mean_n_ticks:.4f}")
print(f"Measured Variance: {var_original_measured:.4f}")
print(f"Quantization Variance: {quantization_variance:.4f}")
print(f"Real Variance: {var_original:.4f}")
print(f"Real Std: {std_original:.4f}")
print(f"R (mean/variance): {R_original:.6f}")

print(f"\n=== Temperature corrected Data ===")
print(f"Mean: {mean_detrended:.4f}")
print(f"Measured Variance: {var_detrended_measured:.4f}")
print(f"Quantization Variance: {quantization_variance:.4f}")
print(f"Real Variance: {var_detrended:.4f}")
print(f"Real Std: {std_detrended:.4f}")
print(f"R (mean/variance): {R_detrended:.6f}")