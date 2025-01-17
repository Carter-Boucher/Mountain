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

private:
	std::vector<uint8_t> p;
	std::vector<uint8_t> perm;
	std::vector<uint8_t> permMod12;

	// Fast floor
	static int fastFloor(double x) {
		return (x >= 0) ? (int)x : (int)x - 1;
	}

	static constexpr std::array<double, 36> grad3 = {
		 1,  1,  0,  -1,  1,  0,   1, -1,  0,  -1, -1,  0,
		 1,  0,  1,  -1,  0,  1,   1,  0, -1,  -1,  0, -1,
		 0,  1,  1,   0, -1,  1,   0,  1, -1,   0, -1, -1
	};
};
