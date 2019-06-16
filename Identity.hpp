#pragma once

// #include "Color.hpp"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

class Color {
  public:
	Color(int r, int g, int b) : m_r{r}, m_g{g}, m_b{b} {}

	bool operator<=(const Color &right) const {
		return m_r <= right.m_r && m_g <= right.m_g && m_b <= right.m_b;
	}

	double operator-(const Color &right) const {
		return abs(m_r - right.m_r) + abs(m_g - right.m_g) +
			   abs(m_b - right.m_b);
	}

	friend std::ostream &operator<<(std::ostream &out, const Color &c) {
		out << c.m_r << " " << c.m_g << " " << c.m_b;
		return out;
		// 	// return out << "[" << c.h << ", " << c.s << ", " << c.v << "]";
		// 	return out << c.h << " " << c.s << " " << c.v;
		// 	// return out << c.h << " " << c.s << " " << c.v;
	}

    // int r() {
    //     return m_r;
    // }

    // int g() {
    //     return m_g;
    // }

    // int b() {
    //     return m_b;
    // }

//   private:
	int m_r;
	int m_g;
	int m_b;
};

class NColors {
  public:
	NColors() {}
	NColors(std::vector<Color> colors) : m_colors{colors} {}

	bool operator<=(const NColors &right) const {
		bool ret = true;
		for (int i = 0; i < 3; ++i) {
			ret = ret && m_colors[i] <= right.m_colors[i];
		}
		return ret;
	}

	double operator-(const NColors &right) const {
		double sum = 0.0;
		for (int i = 0; i < 3; ++i) {
			sum += m_colors[i] - right.m_colors[i];
		}
		return sum;
	}

	friend std::ofstream &operator<<(std::ofstream &out, const NColors &c) {
		for (int i = 0; i < 3; ++i) {
			out << c(i) << std::endl;
		}
		return out;
	}

	// const std::vector<Color> &colors()  const { return m_colors; }

	const Color &operator()(int i) const {
		// return const_cast<Color&>(m_colors[i]);
		return m_colors[i];
	}

  private:
	std::vector<Color> m_colors;
};

// typedef struct s_identity Identity;
class Identity {
  public:
    Identity() {};
	Identity(std::string path);
	Identity(int x, int y, int width, int height, int density, NColors colors);

	// void set_colors(std::vector<Color> &colors);
	void write(const std::string &path) const;

	bool operator<=(const Identity &right) const {
		return m_x <= right.m_x && m_y <= right.m_y &&
			   m_width <= right.m_width && m_height <= right.m_height &&
			   m_density <= right.m_density && m_colors <= right.m_colors;
	}

	double operator-(const Identity &right) const {
		double sum = 0.0;
		sum += abs(m_x - right.m_x);
		sum += abs(m_y - right.m_y);
		sum += abs(m_width - right.m_width);
		sum += abs(m_height - right.m_height);
		sum += abs(m_density - right.m_density);
		// for (int i =0; i <3; ++i) {
		sum += abs(m_colors - right.m_colors);
		// }

		return sum;
	}

    Color operator()(int i) const {
        return m_colors(i);
    }

    std::vector<float> pos() {
        return std::vector<float>({static_cast<float>(m_x), static_cast<float>(m_y), 0.0f});
    }

    std::vector<float> density() {
        return std::vector<float>({static_cast<float>(m_width), static_cast<float>(m_height), static_cast<float>(m_density)});
    }

    std::vector<float> operator[](int i) {
        const Color & c = m_colors(i);
        return std::vector<float>({static_cast<float>(c.m_r), static_cast<float>(c.m_g), static_cast<float>(c.m_b)});
    }


	// const NColors &colors() { return m_colors; }

  private:
	int m_x;
	int m_y;
	int m_width;
	int m_height;
	int m_density;
	NColors m_colors;
};

class Box {
  public:
	Box(Identity _min, Identity mean, Identity _max)
		: m_min{_min}, m_mean{mean}, m_max{_max} {}
	Box(std::string path)
		: m_min(path + "min.txt"), m_mean(path + "mean.txt"),
		  m_max(path + "max.txt") {}

	const Identity &center() const { return m_mean; }

    bool in(const Identity & id) const {
        return m_min <= id && id <= m_max;
    }

    // bool operator<(const Box & right) const {
    //     return true;
    // }

    // bool operator()(const Box & right) const {
    //     return true;
    // }

  private:
	Identity m_min;
	Identity m_mean;
	Identity m_max;
};