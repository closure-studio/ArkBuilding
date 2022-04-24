//
// Created by Nonary on 2022/4/24.
//

#include "api_json_io.h"
namespace albc::api
{
Json::Value JsonReader::Read(const char *json) const
{
    Json::CharReaderBuilder builder;
    builder.settings_["collectComments"] = false;
    std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    Json::Value root;
    std::string errs;
    bool ok = reader->parse(json, json + strlen(json), &root, &errs);
    if (!ok)
    {
        throw std::runtime_error(errs);
    }
    return root;
}
std::string JsonWriter::Write(const Json::Value &json) const
{
    std::ostringstream oss;
    Json::StreamWriterBuilder builder;
    builder.settings_["emitUTF8"] = true;

    std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    writer->write(json, &oss);
    return oss.str();
}
} // namespace albc::api