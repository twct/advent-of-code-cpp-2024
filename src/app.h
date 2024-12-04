#ifndef __APP_H__
#define __APP_H__

#include <spdlog/spdlog.h>

#include <fstream>
#include <functional>
#include <sstream>
#include <string>

class App {
  public:
    template<typename Func>
    App(Func&& func) : m_entrypoint(func) {}

    int run(int argc, char** argv) {
        std::optional<std::string> file_contents;

        if (argc > 1) {
            const std::string filename = argv[1];
            std::ifstream file(filename);

            if (file) {
                std::string content(
                    (std::istreambuf_iterator<char>(file)),
                    std::istreambuf_iterator<char>()
                );
                file_contents = std::move(content);
            } else {
                spdlog::error("{}: failed to open file: {}", argv[0], filename);

                return 1;
            }
        }

        return m_entrypoint(file_contents);
    }

  private:
    std::function<int(const std::optional<std::string>&)> m_entrypoint;
};

#endif
