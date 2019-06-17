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

    void operator +=(const Color & right) {
        m_r += right.m_r;
        m_g += right.m_g;
        m_b += right.m_b;
    }

    void expandMin(const Color & right) {
        m_r = std::min(m_r, right.m_r);
        m_g = std::min(m_g, right.m_g);
        m_b = std::min(m_b, right.m_b);
    }

    void expandMax(const Color & right) {
        m_r = std::max(m_r, right.m_r);
        m_g = std::max(m_g, right.m_g);
        m_b = std::max(m_b, right.m_b);
    }
	// int r() {
	//     return m_r;
	// }

    Color operator /(int scalar) const {
        return Color(m_r /scalar, m_g / scalar, m_b / scalar);
    }
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
	NColors() : m_colors{std::vector<Color>({Color(0, 0, 0), Color(0, 0, 0), Color(0, 0, 0)})} {}
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
    Color & operator()(int i) {
        return m_colors[i];
    }

	Color operator()(int i) const {
		// return const_cast<Color&>(m_colors[i]);
		return m_colors[i];
	}

    // Color operator[](int i) const {
    //     return m_colors[i];
    // }

    void operator += (const NColors & right) {
        for (int i =0; i <3; ++i) {
            m_colors[i] += right.m_colors[i];
        }
    }

    NColors operator /(int scalar) const {
        NColors ret;
        for (int i =0; i <3; ++i) {
            ret(i) = m_colors[i] / scalar;
        }
        return ret;
    }

    void expandMin(const NColors & right) {
        for (int i =0; i <3; ++i) {
            Color & c = m_colors[i];
            const Color & c2 = right(i);
            c.expandMin(c2);
            // m_colors(i).expandMin(right(i));
        }
    }
    void expandMax(const NColors & right) {
        for (int i =0; i <3; ++i) {
            m_colors[i].expandMax(right(i));
        }
    }

  private:
	std::vector<Color> m_colors;
};

// typedef struct s_identity Identity;
class Identity {
  public:
	Identity(){};
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

	Color operator()(int i) { return m_colors(i); }

	std::vector<float> pos() const {
		return std::vector<float>(
			{static_cast<float>(m_x), static_cast<float>(m_y), 0.0f});
	}

	std::vector<float> density() const {
		return std::vector<float>({static_cast<float>(m_width),
								   static_cast<float>(m_height),
								   static_cast<float>(m_density)});
	}

	std::vector<float> rgbs(int i) const {
		const Color &c = m_colors(i);
		return std::vector<float>({static_cast<float>(c.m_r),
								   static_cast<float>(c.m_g),
								   static_cast<float>(c.m_b)});
	}

    void operator += (const Identity & right) {
        m_x += right.m_x;
        m_y += right.m_y;
        m_width += right.m_width;
        m_height += right.m_height;
        m_density += right.m_density;
        m_colors += right.m_colors;
    }

    Identity operator/(int scalar) {
        Identity ret;
        ret.m_x = m_x / scalar;
        ret.m_y = m_y / scalar;
        ret.m_width = m_width / scalar;
        ret.m_height = m_height / scalar;
        ret.m_density = m_density /scalar;
        ret.m_colors = m_colors / scalar;

        return ret;
    }

    void expandMin(const Identity & right) {
        m_x = std::min(m_x, right.m_x);
        m_y = std::min(m_y, right.m_y);
        m_width = std::min(m_width, right.m_width);
        m_height = std::min(m_height, right.m_height);
        m_density = std::min(m_density, right.m_density);
        m_colors.expandMin(right.m_colors);
    }

    void expandMax(const Identity & right) {
        m_x = std::max(m_x, right.m_x);
        m_y = std::max(m_y, right.m_y);
        m_width = std::max(m_width, right.m_width);
        m_height = std::max(m_height, right.m_height);
        m_density = std::max(m_density, right.m_density);
        m_colors.expandMax(right.m_colors);
    }

    friend std::ostream & operator<<(std::ostream & out, const Identity & id) {
        out << id.m_x << " " << id.m_y << " " << id.m_width << " " << id.m_height << " " << id.m_density << std::endl;
        for (int i =0; i <3; ++i) {
            out << id.m_colors(i) << std::endl;
        }
        return out;
        // out << m_colors;
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

    void write(const std::string & path) const;

	const Identity &center() const { return m_mean; }

	bool in(const Identity &id) const { return m_min <= id && id <= m_max; }

    void set_mean(Identity && mean) {
        m_mean = std::move(mean);
    }
	// bool operator<(const Box & right) const {
	//     return true;
	// }

	// bool operator()(const Box & right) const {
	//     return true;
	// }
    void operator += (const Identity & right) {
        m_min.expandMin(right);
        m_max.expandMax(right);
    }

	std::vector<float> pos(const std::vector<float> &colors) const {
		return vbo(colors, m_min.pos(), m_max.pos());
	}

	std::vector<float> density(const std::vector<float> &colors) const {
		return vbo(colors, m_min.density(), m_max.density());
	}

	std::vector<float> rgb(const std::vector<float> &colors, int i) const {
		return vbo(colors, m_min.rgbs(i), m_max.rgbs(i));
	}

	std::vector<float> vbo(const std::vector<float> &colors,
						   const std::vector<float> &min,
						   const std::vector<float> &max) const {

		std::vector<float> boxes;

		std::vector<float> a({min[0], min[1], min[2]});
		std::vector<float> b({max[0], min[1], min[2]});
		std::vector<float> c({max[0], max[1], min[2]});
		std::vector<float> d({min[0], max[1], min[2]});

		std::vector<float> e({min[0], min[1], max[2]});
		std::vector<float> f({max[0], min[1], max[2]});
		std::vector<float> g({max[0], max[1], max[2]});
		std::vector<float> h({min[0], max[1], max[2]});

		std::vector<std::pair<std::vector<float> &, std::vector<float> &>>
			indices = {{a, b}, {b, c}, {c, d}, {d, a}, {e, f}, {f, g},
					   {g, h}, {h, e}, {a, e}, {b, f}, {c, g}, {d, h}};

		for (size_t i = 0; i < indices.size(); ++i) {
			std::vector<float> &first = indices[i].first;
			std::vector<float> &second = indices[i].second;

			boxes.insert(boxes.end(), first.begin(), first.end());
			boxes.insert(boxes.end(), colors.begin(), colors.end());
			boxes.insert(boxes.end(), second.begin(), second.end());
			boxes.insert(boxes.end(), colors.begin(), colors.end());
		}
        return boxes;
	}

  private:
	Identity m_min;
	Identity m_mean;
	Identity m_max;
};