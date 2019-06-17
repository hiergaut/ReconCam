#include "Identity.hpp"
#include <assert.h>

// #include <vector>

Identity::Identity(std::string path) {

	// std::ifstream file("learningFile/known/" + filename + "/min.txt");
	std::ifstream file(path);
	if (!file.is_open()) {
		std::cout << "cannot open file : " << path << std::endl;
		exit(1);
	}

	// while (getline(file, line)) {
	std::string line;

	getline(file, line);

	// float h, h2, h3;
	sscanf(line.c_str(), "%d %d %d %d %d", &m_x, &m_y, &m_width, &m_height,
		   &m_density);

	int r, g, b;
	std::vector<Color> colors;
	for (int i = 0; i < 3; ++i) {
		getline(file, line);
		sscanf(line.c_str(), "%d %d %d", &r, &g, &b);

		colors.push_back(Color(r, g, b));
	}
	file.close();

	m_colors = colors;

	// return std::move(Point3f(h, h2, h3));
    assert(0 <= m_x && m_x <= 640);
    assert(0 <= m_y && m_y <= 480);

}

Identity::Identity(int x, int y, int width, int height, int density,
				   NColors colors)
	: m_x{x}, m_y{y}, m_width{width}, m_height{height}, m_density{density},
	  m_colors{colors} {}

// void Identity::set_colors(std::vector<Color> &colors) {
// 	m_colors = std::move(colors);
// }

void Identity::write(const std::string &path) const {
	std::ofstream out;
	out.open(path);

	// std::string line;
	char line[256];
	sprintf(line, "%d %d %d %d %d\n", m_x, m_y, m_width, m_height, m_density);
	out << line;

	out << m_colors;
	// for (int i =0; i <3; ++i) {
	//     out
	// }

	// out << colors[0] << std::endl;
	// out << colors[1] << std::endl;
	// out << colors[2] << std::endl;
	out.close();
}

void Box::write(const std::string & path) const {
    m_min.write(path + "min.txt");
    m_mean.write(path + "mean.txt");
    m_max.write(path + "max.txt");
}
