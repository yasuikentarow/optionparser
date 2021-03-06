﻿#pragma once
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
/**
   オプション解析
*/
namespace optionparser {
class OptionParser {
 public:
  struct Error {
    std::string message;

    Error(const std::string& message)
      : message(message)
    {}
  };

 private:
  struct Table {
    char option;
    const char* longOption;
    const char* argument;
    const char* description;
    std::function<void(const char*)> func;

    Table(char option, 
          const char* longOption, 
          const char* argument, 
          const char* description, 
          std::function<void(const char*)> func)
      : option(option), 
        longOption(longOption), 
        argument(argument), 
        description(description), 
        func(func)
    {}
  };

 private:
  int argc_ = 0;
  const char** argv_ = nullptr;
  std::vector<Table> tables_;
  int cursor_ = 0;
  const char* optionArgument_ = nullptr;

 public:
  OptionParser() = default;
  virtual ~OptionParser() = default;

  /**
     オプションを追加する
     @param[in] option オプション
     @param[in] longOption 長いオプション
     @param[in] argument 引数名
     @param[in] description 説明
     @param[in] func 挙動
  */
  OptionParser& append(char option, 
                       const char* longOption, 
                       const char* argument, 
                       const char* description, 
                       std::function<void(const char*)> func) {
    tables_.emplace_back(option, longOption, argument, description, func);
    return *this;
  }

  /**
     オプションを解析する
  */
  void parse(int argc, const char** argv) {
    argc_ = argc;
    argv_ = argv;
    cursor_ = 1;
    optionArgument_ = nullptr;
    while(auto argument = getArgument()) {
      if(argument[0] == '-') {
        if(argument[1] == '\0') {
          next();
          break;
        }
        else if(!parseOption(argument)) {
          std::ostringstream stream;
          stream << "no such option, " << argument;
          error(stream.str());
        }
      }
      else {
        break;
      }
    }
  }

  /**
   */
  const char** begin() const {
    return &argv_[cursor_];
  }

  /**
   */
  const char** end() const {
    return &argv_[argc_];
  }

  /**
     ヘルプメッセージを取得する
     @return ヘルプメッセージ
   */
  std::string help() const {
    std::ostringstream stream;
    for(auto& table : tables_) {
      stream << "  -" << table.option;
      if(table.longOption) {
        stream << ", --" << table.longOption;
      }
      if(table.argument) {
        stream << (table.longOption ? '=' : ' ') << table.argument;
      }
      if(table.description) {
        stream << std::endl << "    " << table.description;
      }
      stream << std::endl;
    }
    return stream.str();
  }

  /**
     警告
     @param[in] message メッセージ
  */
  virtual void warning(const std::string& message) {
    std::cerr << "WARNING: " << message << std::endl;
  }

  /**
     エラー
     @param[in] message メッセージ
  */
  virtual void error(const std::string& message) {
    throw Error(message);
  }

 private:
  /**
   */
  bool parseOption(const char* option) {
    for(auto& table : tables_) {
      if(match(table, option)) {
        next();
        table.func(getOptionArgument(table, option));
        return true;
      }
    }
    return false;
  }

  /**
   */
  const char* getOptionArgument(const Table& table, const char* option) {
    if(table.argument) {
      if(optionArgument_) {
        auto argument = optionArgument_;
        optionArgument_ = nullptr;
        return argument;
      }
      if(auto argument = getArgument()) {
        if(argument[0] != '-') {
          next();
          return argument;
        }
      }
      if(table.argument[0] != '[') {
        std::ostringstream stream;
        stream << "nothing " << table.argument << " for " << option;
        error(stream.str());
      }
    }
    else if(optionArgument_) {
      std::ostringstream stream;
      stream << "surplus argument for " << option;
      warning(stream.str());
    }
    return nullptr;
  }

  /**
     引数を取得する
     @return 引数
  */
  const char* getArgument() const {
    return (cursor_ < argc_) ? argv_[cursor_] : nullptr;
  }

  /**
     次の引数
  */
  void next() {
    if(cursor_ < argc_) {
      cursor_++;
    }
  }

  /**
   */
  bool match(const Table& table, const char* argument) {
    if(argument[1] == table.option) {
      if(argument[2] != '\0') {
        optionArgument_ = &argument[2];
      }
      return true;
    }
    else if(argument[1] == '-' && table.longOption) {
      if(strcmp(&argument[2], table.longOption) == 0) {
        return true;
      }
      auto len = strlen(table.longOption);
      if(strncmp(&argument[2], table.longOption, len) == 0) {
        switch(argument[2 + len]) {
        case '\0':
          return true;
        case '=':
          optionArgument_ = &argument[2 + len];
          return true;
        default:
          break;
        }
      }
    }
    return false;
  }
};
}
