#include "benchmark/benchmark.h"

#include <iostream>

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/pointer.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <nlohmann/json.hpp>

#include "json_examples.hpp"

// Parsing benchmarks

static void rapidjson_parse_minimal_bench(benchmark::State& state)
{
    rapidjson::Reader reader;
    rapidjson::Document doc;

    for (auto _ : state)
    {
        rapidjson::StringStream ss {minimalJSON};
        doc.ParseStream<rapidjson::kParseStopWhenDoneFlag>(ss);
    }
}

BENCHMARK(rapidjson_parse_minimal_bench);

static void nlohmann_parse_minimal_bench(benchmark::State& state)
{
    for (auto _ : state)
    {
        nlohmann::json retval = nlohmann::json::parse(minimalJSON);
    }
}

BENCHMARK(nlohmann_parse_minimal_bench);

static void rapidjson_parse_large_bench(benchmark::State& state)
{
    rapidjson::Reader reader;
    rapidjson::Document doc;

    for (auto _ : state)
    {
        rapidjson::StringStream ss {largeJSON};
        doc.ParseStream<rapidjson::kParseStopWhenDoneFlag>(ss);
    }
}

BENCHMARK(rapidjson_parse_large_bench);

static void nlohmann_parse_large_bench(benchmark::State& state)
{
    for (auto _ : state)
    {
        auto retval = nlohmann::json::parse(largeJSON);
    }
}

BENCHMARK(nlohmann_parse_large_bench);

// Stringifying benchmarks

static void rapidjson_stringify_minimal_bench(benchmark::State& state)
{
    rapidjson::Reader reader;
    rapidjson::Document doc;
    rapidjson::StringStream ss {minimalJSON};
    doc.ParseStream<rapidjson::kParseStopWhenDoneFlag>(ss);
    rapidjson::StringBuffer buffer;

    for (auto _ : state)
    {
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        doc.Accept(writer);
        auto retval = buffer.GetString();
    }
}

BENCHMARK(rapidjson_stringify_minimal_bench);

static void nlohmann_stringify_minimal_bench(benchmark::State& state)
{
    nlohmann::json doc;
    doc = nlohmann::json::parse(minimalJSON);

    for (auto _ : state)
    {
        auto retval = doc.dump();
    }
}

BENCHMARK(nlohmann_stringify_minimal_bench);

static void rapidjson_stringify_large_bench(benchmark::State& state)
{
    rapidjson::Reader reader;
    rapidjson::Document doc;
    rapidjson::StringStream ss {largeJSON};
    doc.ParseStream<rapidjson::kParseStopWhenDoneFlag>(ss);
    rapidjson::StringBuffer buffer;

    for (auto _ : state)
    {
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        doc.Accept(writer);
        auto retval = buffer.GetString();
    }
}

BENCHMARK(rapidjson_stringify_large_bench);

static void nlohmann_stringify_large_bench(benchmark::State& state)
{
    nlohmann::json doc;
    doc = nlohmann::json::parse(largeJSON);

    for (auto _ : state)
    {
        auto retval = doc.dump();
    }
}

BENCHMARK(nlohmann_stringify_large_bench);

// Preetyfying benchmarks

static void rapidjson_prettify_minimal_bench(benchmark::State& state)
{
    rapidjson::Reader reader;
    rapidjson::Document doc;
    rapidjson::StringStream ss {minimalJSON};
    doc.ParseStream<rapidjson::kParseStopWhenDoneFlag>(ss);
    rapidjson::StringBuffer buffer;

    for (auto _ : state)
    {
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
        doc.Accept(writer);
        auto retval = buffer.GetString();
    }
}

BENCHMARK(rapidjson_prettify_minimal_bench);

static void nlohmann_prettify_minimal_bench(benchmark::State& state)
{
    nlohmann::json doc;
    doc = nlohmann::json::parse(minimalJSON);

    for (auto _ : state)
    {
        auto retval = doc.dump(4);
    }
}

BENCHMARK(nlohmann_prettify_minimal_bench);

static void rapidjson_prettify_large_bench(benchmark::State& state)
{
    rapidjson::Reader reader;
    rapidjson::Document doc;
    rapidjson::StringStream ss {largeJSON};
    doc.ParseStream<rapidjson::kParseStopWhenDoneFlag>(ss);
    rapidjson::StringBuffer buffer;

    for (auto _ : state)
    {
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
        doc.Accept(writer);
        auto retval = buffer.GetString();
    }
}

BENCHMARK(rapidjson_prettify_large_bench);

static void nlohmann_prettify_large_bench(benchmark::State& state)
{
    nlohmann::json doc;
    doc = nlohmann::json::parse(largeJSON);

    for (auto _ : state)
    {
        auto retval = doc.dump(4);
    }
}

BENCHMARK(nlohmann_prettify_large_bench);

// Field modifying benchmarks

static void rapidjson_field_modifying_minimal_bench(benchmark::State& state)
{
    rapidjson::Reader reader;
    rapidjson::Document doc;
    rapidjson::StringStream ss {minimalJSON};
    doc.ParseStream<rapidjson::kParseStopWhenDoneFlag>(ss);

    for (auto _ : state)
    {
        doc["lastField"] = "Test value";
    }
}

BENCHMARK(rapidjson_field_modifying_minimal_bench);

static void nlohmann_field_modifying_minimal_bench(benchmark::State& state)
{
    auto doc = nlohmann::json::parse(minimalJSON);

    for (auto _ : state)
    {
        doc["lastField"] = "Test value";
    }
}

BENCHMARK(nlohmann_field_modifying_minimal_bench);

static void rapidjson_field_modifying_large_bench(benchmark::State& state)
{
    rapidjson::Reader reader;
    rapidjson::Document doc;
    rapidjson::StringStream ss {largeJSON};
    doc.ParseStream<rapidjson::kParseStopWhenDoneFlag>(ss);

    for (auto _ : state)
    {
        doc["lastField"] = "Test value";
    }
}

BENCHMARK(rapidjson_field_modifying_large_bench);

static void nlohmann_field_modifying_large_bench(benchmark::State& state)
{
    auto doc = nlohmann::json::parse(largeJSON);

    for (auto _ : state)
    {
        doc["lastField"] = "Test value";
    }
}

BENCHMARK(nlohmann_field_modifying_large_bench);

// Retrieving string field benchmarks

static void rapidjson_get_string_field_minimal_bench(benchmark::State& state)
{
    rapidjson::Reader reader;
    rapidjson::Document doc;
    rapidjson::StringStream ss {minimalJSON};
    doc.ParseStream<rapidjson::kParseStopWhenDoneFlag>(ss);

    for (auto _ : state)
    {
        auto retval = doc["lastField"].GetString();
    }
}

BENCHMARK(rapidjson_get_string_field_minimal_bench);

static void nlohmann_get_string_field_minimal_bench(benchmark::State& state)
{
    auto doc = nlohmann::json::parse(minimalJSON);

    for (auto _ : state)
    {
        auto vretval = doc["lastField"];
    }
}

BENCHMARK(nlohmann_get_string_field_minimal_bench);

static void rapidjson_get_string_field_large_bench(benchmark::State& state)
{
    rapidjson::Reader reader;
    rapidjson::Document doc;
    rapidjson::StringStream ss {largeJSON};
    doc.ParseStream<rapidjson::kParseStopWhenDoneFlag>(ss);

    for (auto _ : state)
    {
        auto retval = doc["lastField"].GetString();
    }
}

BENCHMARK(rapidjson_get_string_field_large_bench);

static void nlohmann_get_string_field_large_bench(benchmark::State& state)
{
    auto doc = nlohmann::json::parse(largeJSON);

    for (auto _ : state)
    {
        auto retval = doc["lastField"];
    }
}

BENCHMARK(nlohmann_get_string_field_large_bench);

// Erasing a field benchmarks

static void rapidjson_erase_field_minimal_bench(benchmark::State& state)
{
    rapidjson::Reader reader;
    rapidjson::Document doc;
    rapidjson::StringStream ss {minimalJSON};
    doc.ParseStream<rapidjson::kParseStopWhenDoneFlag>(ss);

    for (auto _ : state)
    {
        doc.RemoveMember("lastField");
    }
}

BENCHMARK(rapidjson_erase_field_minimal_bench);

static void nlohmann_erase_field_minimal_bench(benchmark::State& state)
{
    auto doc = nlohmann::json::parse(minimalJSON);

    for (auto _ : state)
    {
        doc.erase("lastField");
    }
}

BENCHMARK(nlohmann_erase_field_minimal_bench);

static void rapidjson_erase_field_large_bench(benchmark::State& state)
{
    rapidjson::Reader reader;
    rapidjson::Document doc;
    rapidjson::StringStream ss {largeJSON};
    doc.ParseStream<rapidjson::kParseStopWhenDoneFlag>(ss);

    for (auto _ : state)
    {
        doc.RemoveMember("lastField");
    }
}

BENCHMARK(rapidjson_erase_field_large_bench);

static void nlohmann_erase_field_large_bench(benchmark::State& state)
{
    auto doc = nlohmann::json::parse(largeJSON);

    for (auto _ : state)
    {
        doc.erase("lastField");
    }
}

BENCHMARK(nlohmann_erase_field_large_bench);

// Adding a field benchmarks

static void rapidjson_add_kv_pair_minimal_bench(benchmark::State& state)
{
    rapidjson::Reader reader;
    rapidjson::Document doc;
    rapidjson::StringStream ss {minimalJSON};
    doc.ParseStream<rapidjson::kParseStopWhenDoneFlag>(ss);

    for (auto _ : state)
    {
        doc.AddMember("newField", "new value", doc.GetAllocator());
    }
}

BENCHMARK(rapidjson_add_kv_pair_minimal_bench);

static void nlohmann_add_kv_pair_minimal_bench(benchmark::State& state)
{
    auto doc = nlohmann::json::parse(minimalJSON);

    for (auto _ : state)
    {
        doc.push_back({"newField", "new value"});
    }
}

BENCHMARK(nlohmann_add_kv_pair_minimal_bench);

static void rapidjson_add_kv_pair_large_bench(benchmark::State& state)
{
    rapidjson::Reader reader;
    rapidjson::Document doc;
    rapidjson::StringStream ss {largeJSON};
    doc.ParseStream<rapidjson::kParseStopWhenDoneFlag>(ss);

    for (auto _ : state)
    {
        doc.AddMember("newField", "new value", doc.GetAllocator());
    }
}

BENCHMARK(rapidjson_add_kv_pair_large_bench);

static void nlohmann_add_kv_pair_large_bench(benchmark::State& state)
{
    auto doc = nlohmann::json::parse(largeJSON);

    for (auto _ : state)
    {
        doc.push_back({"newField", "new value"});
    }
}

BENCHMARK(nlohmann_add_kv_pair_large_bench);
