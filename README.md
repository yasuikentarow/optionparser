# optionparser

## Usage

```C++
#include "optionparser/OptionParser.hpp"

int main(int argc, const char** argv) {
  std::string output;
  optionparser::OptionParser option(argc, argv);
  try {
    option.append('o', "output", "FILE", "出力ファイルを指定する", 
                  [&](const char* arg) {
                    output.assign(arg);
                  });
    option.append('h', "help", nullptr, "このメッセージを表示する", 
                  [&](const char*) {
                    option.showHelp(std::cerr);
                    exit(-1);
                  });
    option.parse();
  }
  catch(const optionparser::OptionParser::Error& e) {
    std::cerr << e.message << std::endl;
    exit(-1);
  }
  ...
  return 0;
}
```
