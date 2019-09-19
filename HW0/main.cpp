#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

void word_reverse(std::string &target) {
  std::reverse(target.begin(), target.end());
  std::cout << target;
}

void word_split(const std::string &target, const char *del) {
  bool first_substr = true;
  std::vector<char> tar(target.begin(), target.end());
  tar.push_back('\0');
  char *token = std::strtok(&*tar.begin(), del);
  while (token != nullptr) {
    if (first_substr) { // prevent print more space
      first_substr = false;
      std::cout << token;
    } else
      std::cout << ' ' << token;
    token = std::strtok(nullptr, del);
  }
}

void process_line(const bool is_file, const std::string &line,
                  const char *del) {
  std::istringstream iss(line);
  std::string option, target;
  iss >> option >> target;
  if (is_file) // only file need to print context
    std::cout << option << " " << target << std::endl;
  if (option.compare("reverse") == 0)
    word_reverse(target);
  else if (option.compare("split") == 0)
    word_split(target, del);
  std::cout << std::endl;
}

void gen_many_icon(int n, const char c) {
  while (n--)
    std::cout << c;
}

void read_file_fix_line(const bool pos, const char *file_name) {
  gen_many_icon(15, '-');
  if (pos) // first line or end line
    std::cout << "Input file " << file_name;
  else
    std::cout << "End of input file " << file_name;
  gen_many_icon(15, '-');
  std::cout << std::endl;
}

bool read_file(const char *file_name, const char *del) {
  std::ifstream infile(file_name);
  if (infile.good()) { // if file exist
    read_file_fix_line(true, file_name);
    std::string line;
    while (std::getline(infile, line)) {
      if (line.compare("exit") == 0)
        return false;
      process_line(true, line, del);
    }
    read_file_fix_line(false, file_name);
  } else
    std::cout << "File: " << file_name << " does not exist!" << std::endl;
  return true;
}

void user_interface(const char *del) {
  gen_many_icon(15, '*');
  std::cout << "User input";
  gen_many_icon(15, '*');
  std::cout << std::endl;
  std::string line;
  while (std::getline(std::cin, line)) {
    if (line.compare("exit") == 0)
      break; // end this block
    process_line(false, line, del);
  }
}

int main(int argc, char *argv[]) {
  if (argc != 3)
    std::cout << "Usage: " << argv[0] << " <file> <delimiter>" << std::endl;
  else {
    if (read_file(argv[1], argv[2])) // when file not contain "exit"
      user_interface(argv[2]);
  }
  return 0;
}
