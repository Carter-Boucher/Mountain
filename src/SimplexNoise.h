/*
Code adapted based on Jonas Wagner implentation
https://cdnjs.cloudflare.com/ajax/libs/simplex-noise/2.4.0/simplex-noise.js
(copyright in original code)
*/

#include <vector>
#include <array>
#include <random>
#include <cstdint>
#include <cmath>
#include <algorithm>
#include <functional>

class SimplexNoise {
public:
	// Constructors:
	//   1) Default constructor uses a random seed from std::random_device
	//   2) Constructor that takes an unsigned seed for reproducible results
	explicit SimplexNoise(unsigned seed = std::random_device{}())
	{
		// Build the permutation table using the provided seed
		std::mt19937 rng(seed);
		std::uniform_int_distribution<> dist(0, 255);

		// 1) Fill p with 0..255
		p.resize(256);
		for (int i = 0; i < 256; ++i) {
			p[i] = static_cast<uint8_t>(i);
		}

		// 2) Shuffle p
		for (int i = 0; i < 256; i++) {
			int r = i + dist(rng) % (256 - i);
			std::swap(p[i], p[r]);
		}

		// 3) Populate perm and permMod12
		perm.resize(512);
		permMod12.resize(512);
		for (int i = 0; i < 512; i++) {
			perm[i] = p[i & 255];
			permMod12[i] = perm[i] % 12;
		}
	}

	// 2D noise
	double noise2D(double xin, double yin) const
	{
		// Skewing/Unskewing factors for 2D
		static const double F2 = 0.5 * (std::sqrt(3.0) - 1.0);
		static const double G2 = (3.0 - std::sqrt(3.0)) / 6.0;

		// Noise contributions from the three corners
		double n0 = 0.0, n1 = 0.0, n2 = 0.0;

		// Skew the input space to determine which simplex cell we're in
		double s = (xin + yin) * F2;
		int i = fastFloor(xin + s);
		int j = fastFloor(yin + s);

		double t = (i + j) * G2;
		double X0 = i - t;
		double Y0 = j - t;

		double x0 = xin - X0;
		double y0 = yin - Y0;

		// Determine which simplex we are in
		int i1, j1;
		if (x0 > y0) {
			i1 = 1; j1 = 0;
		}
		else {
			i1 = 0; j1 = 1;
		}

		// Offsets for corners
		double x1 = x0 - i1 + G2;
		double y1 = y0 - j1 + G2;
		double x2 = x0 - 1.0 + 2.0 * G2;
		double y2 = y0 - 1.0 + 2.0 * G2;

		// Work out the hashed gradient indices
		int ii = i & 255;
		int jj = j & 255;

		// Calculate the contribution from the three corners
		double t0 = 0.5 - x0 * x0 - y0 * y0;
		if (t0 >= 0) {
			int gi0 = permMod12[ii + perm[jj]] * 3;
			t0 *= t0;
			n0 = t0 * t0 * (grad3[gi0 + 0] * x0 + grad3[gi0 + 1] * y0);
		}

		double t1 = 0.5 - x1 * x1 - y1 * y1;
		if (t1 >= 0) {
			int gi1 = permMod12[ii + i1 + perm[jj + j1]] * 3;
			t1 *= t1;
			n1 = t1 * t1 * (grad3[gi1 + 0] * x1 + grad3[gi1 + 1] * y1);
		}

		double t2 = 0.5 - x2 * x2 - y2 * y2;
		if (t2 >= 0) {
			int gi2 = permMod12[ii + 1 + perm[jj + 1]] * 3;
			t2 *= t2;
			n2 = t2 * t2 * (grad3[gi2 + 0] * x2 + grad3[gi2 + 1] * y2);
		}

		// Sum up and scale the result
		return 70.0 * (n0 + n1 + n2);
	}

	// 3D noise
	double noise3D(double xin, double yin, double zin) const
	{
		// Skewing/Unskewing factors for 3D
		static const double F3 = 1.0 / 3.0;
		static const double G3 = 1.0 / 6.0;

		double n0, n1, n2, n3; // Noise contributions
		n0 = n1 = n2 = n3 = 0.0;

		// Skew the input space
		double s = (xin + yin + zin) * F3;
		int i = fastFloor(xin + s);
		int j = fastFloor(yin + s);
		int k = fastFloor(zin + s);

		double t = (i + j + k) * G3;
		double X0 = i - t;
		double Y0 = j - t;
		double Z0 = k - t;

		double x0 = xin - X0;
		double y0 = yin - Y0;
		double z0 = zin - Z0;

		// Determine which simplex we are in
		int i1, j1, k1;
		int i2, j2, k2;

		if (x0 >= y0) {
			if (y0 >= z0) {
				i1 = 1; j1 = 0; k1 = 0;
				i2 = 1; j2 = 1; k2 = 0;
			}
			else if (x0 >= z0) {
				i1 = 1; j1 = 0; k1 = 0;
				i2 = 1; j2 = 0; k2 = 1;
			}
			else {
				i1 = 0; j1 = 0; k1 = 1;
				i2 = 1; j2 = 0; k2 = 1;
			}
		}
		else {
			if (y0 < z0) {
				i1 = 0; j1 = 0; k1 = 1;
				i2 = 0; j2 = 1; k2 = 1;
			}
			else if (x0 < z0) {
				i1 = 0; j1 = 1; k1 = 0;
				i2 = 0; j2 = 1; k2 = 1;
			}
			else {
				i1 = 0; j1 = 1; k1 = 0;
				i2 = 1; j2 = 1; k2 = 0;
			}
		}

		double x1 = x0 - i1 + G3;
		double y1 = y0 - j1 + G3;
		double z1 = z0 - k1 + G3;

		double x2 = x0 - i2 + 2.0 * G3;
		double y2 = y0 - j2 + 2.0 * G3;
		double z2 = z0 - k2 + 2.0 * G3;

		double x3 = x0 - 1.0 + 3.0 * G3;
		double y3 = y0 - 1.0 + 3.0 * G3;
		double z3 = z0 - 1.0 + 3.0 * G3;

		// Hashed gradient indices of the four simplex corners
		int ii = i & 255;
		int jj = j & 255;
		int kk = k & 255;

		// Calculate the contribution from the four corners
		double t0 = 0.6 - x0 * x0 - y0 * y0 - z0 * z0;
		if (t0 < 0) n0 = 0.0;
		else {
			int gi0 = permMod12[ii + perm[jj + perm[kk]]] * 3;
			t0 *= t0;
			n0 = t0 * t0 * (grad3[gi0 + 0] * x0 + grad3[gi0 + 1] * y0 + grad3[gi0 + 2] * z0);
		}

		double t1 = 0.6 - x1 * x1 - y1 * y1 - z1 * z1;
		if (t1 < 0) n1 = 0.0;
		else {
			int gi1 = permMod12[ii + i1 + perm[jj + j1 + perm[kk + k1]]] * 3;
			t1 *= t1;
			n1 = t1 * t1 * (grad3[gi1 + 0] * x1 + grad3[gi1 + 1] * y1 + grad3[gi1 + 2] * z1);
		}

		double t2 = 0.6 - x2 * x2 - y2 * y2 - z2 * z2;
		if (t2 < 0) n2 = 0.0;
		else {
			int gi2 = permMod12[ii + i2 + perm[jj + j2 + perm[kk + k2]]] * 3;
			t2 *= t2;
			n2 = t2 * t2 * (grad3[gi2 + 0] * x2 + grad3[gi2 + 1] * y2 + grad3[gi2 + 2] * z2);
		}

		double t3 = 0.6 - x3 * x3 - y3 * y3 - z3 * z3;
		if (t3 < 0) n3 = 0.0;
		else {
			int gi3 = permMod12[ii + 1 + perm[jj + 1 + perm[kk + 1]]] * 3;
			t3 *= t3;
			n3 = t3 * t3 * (grad3[gi3 + 0] * x3 + grad3[gi3 + 1] * y3 + grad3[gi3 + 2] * z3);
		}

		// Sum up and scale the result
		return 32.0 * (n0 + n1 + n2 + n3);
	}

private:
	// The permutation vector
	std::vector<uint8_t> p;
	// Expanded permutation arrays
	std::vector<uint8_t> perm;
	std::vector<uint8_t> permMod12;

	// Fast floor
	static int fastFloor(double x) {
		return (x >= 0) ? (int)x : (int)x - 1;
	}

	// Gradients for 3D (length = 12 * 3)
	// Each triplet is a gradient direction
	static constexpr std::array<double, 36> grad3 = {
		 1,  1,  0,  -1,  1,  0,   1, -1,  0,  -1, -1,  0,
		 1,  0,  1,  -1,  0,  1,   1,  0, -1,  -1,  0, -1,
		 0,  1,  1,   0, -1,  1,   0,  1, -1,   0, -1, -1
	};

	// Gradients for 4D (length = 32 * 4)
	static constexpr std::array<double, 128> grad4 = {
		0, 1, 1, 1,    0, 1, 1,-1,    0, 1,-1, 1,    0, 1,-1,-1,
		0,-1, 1, 1,    0,-1, 1,-1,    0,-1,-1, 1,    0,-1,-1,-1,
		1, 0, 1, 1,    1, 0, 1,-1,    1, 0,-1, 1,    1, 0,-1,-1,
	   -1, 0, 1, 1,   -1, 0, 1,-1,   -1, 0,-1, 1,   -1, 0,-1,-1,
		1, 1, 0, 1,    1, 1, 0,-1,    1,-1, 0, 1,    1,-1, 0,-1,
	   -1, 1, 0, 1,   -1, 1, 0,-1,   -1,-1, 0, 1,   -1,-1, 0,-1,
		1, 1, 1, 0,    1, 1,-1, 0,    1,-1, 1, 0,    1,-1,-1, 0,
	   -1, 1, 1, 0,   -1, 1,-1, 0,   -1,-1, 1, 0,   -1,-1,-1, 0
	};
};

// #include <iostream>
// int main() {
//     SimplexNoise noise(12345); // seeded
//     std::cout << noise.noise2D(0.1, 0.2) << std::endl;
//     std::cout << noise.noise3D(0.1, 0.2, 0.3) << std::endl;
//     std::cout << noise.noise4D(0.1, 0.2, 0.3, 0.4) << std::endl;
//     return 0;
// }
