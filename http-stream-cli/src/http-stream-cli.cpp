// http-stream-cli.cpp : Defines the entry point for the application.
//

#include <iostream>
#include <fstream>

#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

#include "parser/validator.h"

using namespace std;

int main()
{
    spdlog::set_level(spdlog::level::debug);

    std::string path = "D:\\Desktop\\C++\\Visual Studio\\http-stream-cli\\example.json";
    std::ifstream f(path);
    nlohmann::json json = nlohmann::json::parse(f);

    if (httpstream::parser::check_object(json["request"]["body"]))
    {
        std::cout << "successful\n";
    }

    f.close();
    return 0;
}
