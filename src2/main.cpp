#include <fstream>
#include <gamma/lexer/lex.hpp>
#include <iostream>
#include <print>

int main(int argc, const char **argv)
{
    if (argc != 2)
    {
        std::println(stderr, "Usage: {} <source_file.gm>", argv[0]);
        return -1;
    }

    std::ifstream src_file { argv[1], std::ios::binary | std::ios::ate };
    if (!src_file)
    {
        std::println(stderr, "ERROR: failed to open {}; did you provide the proper path?", argv[1]);
        return -1;
    }

    const auto end_pos = src_file.tellg();
    if (end_pos < 0)
    {
        std::println(stderr, "ERROR: failed to get size of {}", argv[1]);
        return -1;
    }

    const std::streamsize stream_size { end_pos };
    src_file.seekg(0, std::ios::beg);

    std::string file_content(stream_size, 0);
    if (!src_file.read(file_content.data(), stream_size))
    {
        std::println(stderr, "ERROR: failed to read {}", argv[1]);
        return -1;
    }

    try
    {
        const auto tokens { gm::lexer::lex(file_content, argv[1]) };
        for (const auto &token : tokens)
        {
            std::println("{}", token);
        }
    }
    catch (const gm::lexer::lexer_error &e)
    {
        return -1;
    }

    return 0;
}
