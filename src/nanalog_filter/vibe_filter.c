//spencer
//made with AI vibes!

#include <math.h>   // For M_PI, expf, cosf, sqrtf, fabsf
#include <stdbool.h> // For bool

// Define M_PI if it's not available in math.h (e.g., in some older C standards)
#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

/**
 * @brief Calculates biquad filter coefficients using zero-pole mapping (matched Z-transform).
 *
 * This function computes the 'a' (denominator) and 'b' (numerator) coefficients
 * for a second-order IIR filter based on analog filter characteristics.
 * The coefficients are for a Direct Form I or II Transposed biquad structure:
 * H(z) = (b0 + b1*z^-1 + b2*z^-2) / (a0 + a1*z^-1 + a2*z^-2)
 * where a0 is normalized to 1.0.
 *
 * @param f_cutoff  The cutoff frequency of the filter in Hertz (Hz).
 * Must be > 0 and < f_sample / 2.0.
 * @param f_sample  The sampling rate in Hertz (Hz). Must be > 0.
 * @param q_factor  The Q factor (resonance) of the filter.
 * For low-pass, Q=0.707 for Butterworth. Must be > 0.
 * @param zero      If false, calculates coefficients for a 40dB/decade Low-Pass filter
 * (two zeros at Nyquist frequency, z=-1).
 * If true, calculates coefficients for a 20dB/decade Low-Pass filter
 * (one zero at Nyquist, z=-1, and one zero at DC, z=0).
 * @param a         Pointer to a float array of size 3 for denominator coefficients [a0, a1, a2].
 * a[0] will be normalized to 1.0.
 * @param b         Pointer to a float array of size 3 for numerator coefficients [b0, b1, b2].
 */
void vibe_filter_coeffs(float f_cutoff, float f_sample, float q_factor, bool zero, float* a, float* b)
{
    // --- Input Validation and Defaulting (for robustness) ---
    if (f_sample <= 0.0f)
    {
        // Error: Sample rate must be positive. Set to pass-through (no filter)
        a[0] = 1.0f; a[1] = 0.0f; a[2] = 0.0f;
        b[0] = 1.0f; b[1] = 0.0f; b[2] = 0.0f;
        return;
    }
    if (f_cutoff <= 0.0f || f_cutoff >= f_sample / 2.0f)
    {
        // Error: Cutoff frequency must be positive and below Nyquist.
        // Set to pass-through
        a[0] = 1.0f; a[1] = 0.0f; a[2] = 0.0f;
        b[0] = 1.0f; b[1] = 0.0f; b[2] = 0.0f;
        return;
    }
    if (q_factor <= 0.0f)
    {
        // Error: Q factor must be positive. Default to Butterworth Q (0.707).
        q_factor = 0.707f;
    }

    // --- Common Calculations for Poles ---
    // Analog cutoff frequency in radians/second
    float omega_c = 2.0f * M_PI * f_cutoff;
    // Sampling period in seconds
    float T = 1.0f / f_sample;

    // Calculate the parameters for mapping analog poles to digital poles.
    // Analog poles for a second-order resonant system are s = -sigma +/- j*omega_d
    // sigma = omega_c / (2 * Q)
    // omega_d = omega_c * sqrt(1 - 1 / (4 * Q^2)) -- damped frequency (for Q >= 0.5)
    // Mapped digital poles: z = exp(s*T) = exp(-sigma*T) * exp(j*omega_d*T)

    float sigma_T = (omega_c / (2.0f * q_factor)) * T;
    float omega_d_T;

    // Calculate the term inside the square root for omega_d_T.
    // If Q < 0.5, the poles are real and distinct (overdamped).
    // For this biquad structure, which implicitly assumes complex conjugate poles
    // or critically damped behavior for its `cos(phi)` term, we clamp the sqrt argument.
    // This makes filters with Q < 0.5 behave as critically damped (Q=0.5) to maintain stability.
    float sqrt_arg = 1.0f - 1.0f / (4.0f * q_factor * q_factor);
    if (sqrt_arg < 0.0f)
    {
        omega_d_T = 0.0f; // Clamps to critically damped behavior for Q < 0.5
    }
    else
    {
        omega_d_T = omega_c * sqrtf(sqrt_arg) * T;
    }

    float R_poles = expf(-sigma_T); // Radial distance of digital poles from origin
    float phi_poles = omega_d_T;    // Angle of digital poles

    // Denominator coefficients (a0 is normalized to 1.0)
    // The denominator polynomial is (z - p1)(z - p2) = z^2 - (p1+p2)z + p1*p2
    // In z^-1 form: 1 - (p1+p2)z^-1 + p1*p2*z^-2
    // p1+p2 = 2 * R_poles * cos(phi_poles)
    // p1*p2 = R_poles^2
    a[0] = 1.0f;
    a[1] = -2.0f * R_poles * cosf(phi_poles);
    a[2] = R_poles * R_poles;

    // --- Calculate Numerator (Zero) Coefficients and Gain ---
    float K_gain;
    float b_num0, b_num1, b_num2;

    if (zero)
    {
        // 20dB/decade Low-Pass: One zero at Nyquist (z=-1), one zero at DC (z=0)
        // Numerator polynomial: (z - (-1)) * (z - 0) = z * (z + 1) = z^2 + z
        // In z^-1 form (divide by z^2): 1 + 1*z^-1 + 0*z^-2
        b_num0 = 1.0f;
        b_num1 = 1.0f;
        b_num2 = 0.0f;

        // Gain normalization: Set gain at DC (z=1) to 1.0
        // H(1) = K_gain * (b_num0 + b_num1 + b_num2) / (a[0] + a[1] + a[2]) = 1.0
        // K_gain = (a[0] + a[1] + a[2]) / (b_num0 + b_num1 + b_num2)
        float b_sum_at_dc = b_num0 + b_num1 + b_num2; // For this case, 1.0 + 1.0 + 0.0 = 2.0
        K_gain = (a[0] + a[1] + a[2]) / b_sum_at_dc;

    }
    else
    { 
        // 40dB/decade Low-Pass: Two zeros at Nyquist frequency (z=-1)
        // Numerator polynomial: (z - (-1))^2 = (z + 1)^2 = z^2 + 2z + 1
        // In z^-1 form (divide by z^2): 1 + 2*z^-1 + 1*z^-2
        b_num0 = 1.0f;
        b_num1 = 2.0f;
        b_num2 = 1.0f;

        // Gain normalization: Set gain at DC (z=1) to 1.0
        // H(1) = K_gain * (b_num0 + b_num1 + b_num2) / (a[0] + a[1] + a[2]) = 1.0
        // K_gain = (a[0] + a[1] + a[2]) / (b_num0 + b_num1 + b_num2)
        float b_sum_at_dc = b_num0 + b_num1 + b_num2; // For this case, 1.0 + 2.0 + 1.0 = 4.0
        K_gain = (a[0] + a[1] + a[2]) / b_sum_at_dc;
    }

    // Apply the gain to the numerator coefficients
    b[0] = K_gain * b_num0;
    b[1] = K_gain * b_num1;
    b[2] = K_gain * b_num2;
}


/**
 * @brief Applies a biquad (second-order IIR) filter to an array of samples.
 *
 * This function implements the Direct Form II Transposed biquad filter structure.
 * It processes samples in-place, meaning the input array `samples` will be
 * overwritten with the filtered output.
 *
 * The internal filter state `state` must be initialized to zeros before the
 * first call to this function for a given filter instance. This `state` array
 * must persist across multiple calls to maintain filter continuity.
 *
 * @param samples   Pointer to the array of float samples to be filtered.
 * The filtered output will overwrite these samples.
 * @param nsamples  The number of samples in the `samples` array.
 * @param a         Pointer to a float array of size 3 containing the denominator
 * coefficients [a0, a1, a2]. a[0] is assumed to be 1.0.
 * @param b         Pointer to a float array of size 3 containing the numerator
 * coefficients [b0, b1, b2].
 * @param state     Pointer to a float array of size 2 for the internal filter state.
 * state[0] holds w[n-1] and state[1] holds w[n-2].
 * Must be initialized to {0.0f, 0.0f} before first use.
 */
void vibe_filter(float* samples, uint32_t nsamples, float* a, float* b, float* state)
{
    // Coefficients for clarity (a0 is implicitly 1.0)
    float a1 = a[1];
    float a2 = a[2];
    float b0 = b[0];
    float b1 = b[1];
    float b2 = b[2];

    // Internal state variables
    // state[0] corresponds to w[n-1]
    // state[1] corresponds to w[n-2]
    float w_n_minus_1 = state[0];
    float w_n_minus_2 = state[1];

    // Loop through each sample in the input array
    for (uint32_t n = 0; n < nsamples; ++n)
    {
        float x_n = samples[n]; // Get current input sample

        // Calculate the current internal state variable w[n]
        // w[n] = x[n] - a1*w[n-1] - a2*w[n-2]
        float w_n = x_n - (a1 * w_n_minus_1) - (a2 * w_n_minus_2);

        // Calculate the current output sample y[n]
        // y[n] = b0*w[n] + b1*w[n-1] + b2*w[n-2]
        float y_n = (b0 * w_n) + (b1 * w_n_minus_1) + (b2 * w_n_minus_2);

        // Update the state variables for the next iteration
        w_n_minus_2 = w_n_minus_1; // w[n-2] becomes old w[n-1]
        w_n_minus_1 = w_n;         // w[n-1] becomes current w[n]

        samples[n] = y_n; // Store the filtered output back into the array
    }

    // Save the updated state back to the provided state array
    state[0] = w_n_minus_1;
    state[1] = w_n_minus_2;
}
