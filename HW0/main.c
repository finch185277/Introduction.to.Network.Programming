#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

void process_line(bool is_file, std::string &line, const std::string &c) {
  std::istringstream iss(line);
  std::string option, target;
  iss >> option >> target;
  if (is_file) // only file need to print context
    std::cout << option << " " << target << std::endl;
  if (option.compare("reverse") == 0) {
    std::reverse(target.begin(), target.end());
    std::cout << target;
  } else if (option.compare("split") == 0) {
    std::istringstream tar(target);
    std::string s;
    bool first_substr = true;
    while (std::getline(tar, s, c[0])) {
      if (first_substr) { // prevent print more space
        first_substr = false;
        std::cout << s;
      } else
        std::cout << ' ' << s;
    }
  }
  std::cout << std::endl;
}

void gen_many_icon(int n, char c) {
  while (n--)
    std::cout << c;
}

void read_file_fix_line(bool pos, const std::string &str) {
  gen_many_icon(15, '-');
  if (pos) // first line or end line
    std::cout << "Input file " << str;
  else
    std::cout << "End of input file " << str;
  gen_many_icon(15, '-');
  std::cout << std::endl;
}

bool read_file(const std::string &file_name, const std::string &c) {
  read_file_fix_line(true, file_name);
  std::ifstream infile(file_name);
  std::string line;
  while (std::getline(infile, line)) {
    if (line.compare("exit") == 0)
      return false;
    process_line(true, line, c);
  }
  read_file_fix_line(false, file_name);
  return true;
}

void user_interface(const std::string &c) {
  gen_many_icon(15, '*');
  std::cout << "User input";
  gen_many_icon(15, '*');
  std::cout << std::endl;
  std::string line;
  while (std::getline(std::cin, line)) {
    if (line.compare("exit") == 0)
      break; // end this block
    process_line(false, line, c);
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
