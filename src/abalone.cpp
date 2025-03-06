#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstring>
#include <iostream>
#include <vector>
#include <unistd.h>

void handle_error(const char* msg);
const char* map_file(const char* fname, size_t& length);

int main()
{
    size_t length;
    auto f = map_file("Test1.board", length);
    auto l = f + length;

    std::vector<std::string> values;
    const char* start = f;

    while (f < l)
    {
        if (*f == ',' || f == l - 1)  // If we find a comma or reach end of file
        {
            size_t token_length = f - start + (f == l - 1);  // Include last char if EOF
            if (token_length > 0)
                values.emplace_back(start, token_length);  // Store substring in vector
            start = f + 1;  // Move start to next character after comma
        }
        ++f;
    }

    // Print extracted values
    std::cout << "Extracted Values:\n";
    for (const auto& value : values)
        std::cout << value << "\n";

    // Free memory-mapped file
    munmap((void*)f, length);

    return 0;
}

void handle_error(const char* msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

const char* map_file(const char* fname, size_t& length)
{
    int fd = open(fname, O_RDONLY);
    if (fd == -1)
        handle_error("open");

    struct stat sb;
    if (fstat(fd, &sb) == -1)
        handle_error("fstat");

    length = sb.st_size;

    const char* addr = static_cast<const char*>(mmap(NULL, length, PROT_READ, MAP_PRIVATE, fd, 0u));
    if (addr == MAP_FAILED)
        handle_error("mmap");

    close(fd);  // Close file descriptor after mapping

    return addr;
}
