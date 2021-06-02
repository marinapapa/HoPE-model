#ifndef JSON_HPP_IMPORT_INCLUDED
#define JSON_HPP_IMPORT_INCLUDED

#include <string>
#include <fstream>
#include <filesystem>
#include <vector>
#include <nlohmann/json.hpp>  // nlohmann.github.io/json/
using json = nlohmann::json;


// removes R-style comments (# ...) from a 'non-conforming json' file 
// before passing its content to an nlohmann::json object.
// Don't use this function for conforming json as it depends on
// line-endings to work.
json uncomment_json(const std::filesystem::path& path);


// removes R-style comments (# ...) from a 'non-conforming json' 
// before passing its content to an nlohmann::json object.
// Don't use this function for conforming json as it depends on
// line-endings to work.
json uncomment_json(const std::string& jstr);



json compose_json(std::vector<std::filesystem::path> paths);


void save_json(const json& J, const std::filesystem::path& json_file);


#endif
