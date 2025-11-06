import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from scipy import stats
from scipy.optimize import curve_fit
import os

EXP_NUMBER = 1

# Temperature corection (at 30s per experiment, we consider temperature to be approxiamtely constant over 15min intervals)
WINDOW_SIZE = 40

DATA_FILE = f'data/exp{EXP_NUMBER}.csv'
OUT_FILE = f'plots/paper_results_exp{EXP_NUMBER}.pdf'

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

# Create figure
fig, ax4 = plt.subplots(1, 1, figsize=(14, 10))

# Plot histogram and get bin information
n, bins, patches = ax4.hist(n_ticks_detrended, bins=30, edgecolor='black', alpha=0.7, color='green', label='Data')

# Fit normal distribution
mu = mean_detrended
sigma = std_detrended

# Create x values for the fitted curve
x_min, x_max = ax4.get_xlim()
x = np.linspace(x_min, x_max, 500)

# Calculate the normal distribution PDF
pdf = stats.norm.pdf(x, mu, sigma)

# Scale PDF to match histogram (multiply by total count and bin width)
bin_width = bins[1] - bins[0]
pdf_scaled = pdf * len(n_ticks_detrended) * bin_width

# Plot the fitted Gaussian
ax4.plot(x, pdf_scaled, 'b--', linewidth=2, label=f'Normal fit')# (μ={mu:.1f}, σ={sigma:.2f})')

ax4.axvline(mean_detrended, color='r', linestyle='--', linewidth=2, label=f'Mean')# = {mean_detrended:.1f}')
ax4.set_xlabel('Number of ticks', fontsize=22)
ax4.set_ylabel('Frequency', fontsize=22)
if EXP_NUMBER == 1:
    ax4.set_xlim(167980,168060)
ax4.tick_params(axis='both', which='major', labelsize=22, length=16, width=2)
ax4.grid(True, alpha=0.3)
ax4.legend(fontsize=20, loc='best')

plt.tight_layout()

# Save the plot
plt.savefig(OUT_FILE, dpi=300, bbox_inches='tight')
print(f"Plot saved to {OUT_FILE}")

plt.show()