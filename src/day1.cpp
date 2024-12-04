#include "app.h"

void parse_string_to_vectors(
    const std::string& input,
    std::vector<int>& vector1,
    std::vector<int>& vector2
) {
    std::istringstream stream(input);
    std::string line;

    while (std::getline(stream, line)) {
        std::istringstream line_stream(line);
        int num1, num2;
        if (line_stream >> num1 >> num2) {
            vector1.push_back(num1);
            vector2.push_back(num2);
        }
    }
}

template<typename T>
T freq(const std::vector<T>& v, T i) {
    return std::count(v.begin(), v.end(), i);
}

int day1(const std::optional<std::string>& input) {
    std::vector<int> v1 {3, 4, 2, 1, 3, 3};
    std::vector<int> v2 {4, 3, 5, 3, 9, 3};

    if (input) {
        parse_string_to_vectors(*input, v1, v2);
    }

    std::sort(v1.begin(), v1.end());
    std::sort(v2.begin(), v2.end());

    int total_distance = 0;
    int sim_score = 0;

    for (size_t i = 0; i < v1.size(); ++i) {
        int a = v1[i];
        int b = v2[i];
        int distance = std::abs(a - b);
        int sim = freq(v2, a);
        total_distance += distance;
        sim_score += a * sim;
    }

    spdlog::info("Distance between both lists: {}", total_distance);
    spdlog::info("Similarity score: {}", sim_score);

    return 0;
}

int main(int argc, char** argv) {
    return App(day1).run(argc, argv);
}
