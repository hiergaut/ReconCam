#pragma once

#include <unistd.h>
#include <iostream>
#include <fstream>


std::string getCurTime() {
	time_t t = time(0);
	tm *now = localtime(&t);

	int hour = now->tm_hour;
	std::string hour_str = std::to_string(hour);
	if (hour < 10) {
		hour_str = "0" + hour_str;
	}

	int min = now->tm_min;
	std::string min_str = std::to_string(min);
	if (min < 10)
		min_str = "0" + min_str;

	int sec = now->tm_sec;
	std::string sec_str = std::to_string(sec);
	if (sec < 10)
		sec_str = "0" + sec_str;

	return hour_str + ':' + min_str + ':' + sec_str;
}

std::string getDay() {
	time_t t = time(0);
	tm *now = localtime(&t);

	int year = now->tm_year + 1900;
	int month = now->tm_mon + 1;
	int day = now->tm_mday;

	return std::to_string(year) + ":" + std::to_string(month) + ":" +
		   std::to_string(day);
}

std::string getHostname() {
	std::string file = "/etc/hostname";
	std::ifstream getFile(file.c_str());
	if (!getFile.is_open()) {
		std::cout << "unable to open /etc/hostname" << std::endl;
		perror(file.c_str());
		exit(4);
	}

	std::string ret;
	getFile >> ret;
	getFile.close();

	return ret;
}


// Point3f readPointFromFile(std::string path) {

// 	// std::ifstream file("learningFile/known/" + filename + "/min.txt");
// 	std::ifstream file(path);
// 	if (!file.is_open()) {
// 		std::cout << "cannot open file known" << std::endl;
// 		exit(1);
// 	}

// 	// while (getline(file, line)) {
// 	std::string line;
// 	getline(file, line);
// 	file.close();
// 	float h, h2, h3;
// 	sscanf(line.c_str(), "%f %f %f", &h, &h2, &h3);

// 	return std::move(Point3f(h, h2, h3));
// }

std::vector<std::string> forEachFileInDir(std::string dirPath) {
	// std::string dirPath = "learningFile/known/";
	std::vector<std::string> files;
	DIR *dp;
	struct dirent *dirp;
	if ((dp = opendir(dirPath.c_str())) == NULL) {
		std::cout << "Error cannot opening " << dirPath << std::endl;
		exit(2);
	}
	while ((dirp = readdir(dp)) != NULL) {
		std::string filename(dirp->d_name);
		if (filename.compare(".") && filename.compare("..")) {
			files.push_back(std::move(filename));
		}
	}
	closedir(dp);

	return files;
	// return std::move(files);
}

bool emptyDir(std::string dirPath) {
	int cpt = 0;
	for (auto &file : forEachFileInDir(dirPath)) {
		(void)file;
		++cpt;
	}

	return cpt == 0;
}

void thread_alert(std::string filename) {
	std::cout << "new thread : " << filename << std::endl;
	std::fstream fs;
	fs.open(filename, std::ios::out);
	if (!fs.is_open()) {
		std::cout << "unable to open file : " << filename << std::endl;
	}
	fs.close();

	usleep(1000000 * 18);

	std::remove(filename.c_str());

	if (emptyDir("alert/")) {
		std::remove("alert.jpg");
	}
}
