#include <stdint.h>

extern "C" {

    int floor(double x) {
        return (int)x;
    }

    int ceil(double x) {
        if (x == (double)(int)x) { return (int)x; }
        return (int)x;
    }

    double pow(double x, int i) {
        double ret = x;
        for (int j = 1; j < i; j++) {
            ret *= x;
        }
        return ret;
    }

    // Implementing natural logarithm (ln) using an approximation method
    double ln(double x) {
        if (x <= 0) return 0; // Error: ln is undefined for non-positive values
        double result = 0;
        double term = (x - 1) / (x + 1);
        double term_squared = term * term;
        
        for (int i = 1; i < 100; i += 2) {
            result += (1.0 / i) * pow(term, i);
        }
        
        return 2 * result;
    }

    // Function to compute logarithm of x to the base b
    double log_b(double x, double b) {
        return ln(x) / ln(b);
    }
}