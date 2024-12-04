#include "app.h"

struct Report {
    std::vector<int> levels;
};

void parse_reports(std::vector<Report>& reports, const std::string& input);

bool is_report_safe(const std::vector<int>& levels) {
    if (levels.size() <= 1) {
        return true;
    }

    bool is_increasing = true;
    bool is_decreasing = true;

    for (size_t i = 1; i < levels.size(); ++i) {
        int diff = levels[i] - levels[i - 1];

        if (diff == 0 || std::abs(diff) > 3) {
            return false;
        }

        is_increasing &= (diff > 0);
        is_decreasing &= (diff < 0);
    }

    return is_increasing || is_decreasing;
}

bool can_report_be_made_safe(const std::vector<int>& levels) {
    if (is_report_safe(levels)) {
        return true;
    }

    for (size_t remove_idx = 0; remove_idx < levels.size(); ++remove_idx) {
        std::vector<int> modified_levels;

        for (size_t i = 0; i < levels.size(); ++i) {
            if (i != remove_idx) {
                modified_levels.push_back(levels[i]);
            }
        }

        if (is_report_safe(modified_levels)) {
            return true;
        }
    }

    return false;
}

int day2(const std::optional<std::string>& input) {
    std::vector<Report> reports {
        Report {{7, 6, 4, 2, 1}},
        Report {{1, 2, 7, 8, 9}},
        Report {{9, 7, 6, 2, 1}},
        Report {{1, 3, 2, 4, 5}},
        Report {{8, 6, 4, 4, 1}},
        Report {{1, 3, 6, 7, 9}},
    };

    if (input) {
        parse_reports(reports, *input);
    }

    int safe_reports = 0;
    int safe_reports_with_problem_dampening = 0;

    for (const auto& report : reports) {
        if (is_report_safe(report.levels)) {
            safe_reports += 1;
        }
        if (can_report_be_made_safe(report.levels)) {
            safe_reports_with_problem_dampening += 1;
        }
    }

    spdlog::info("Number of safe reports processed: {}", safe_reports);
    spdlog::info(
        "Number of safe reports processed with problem dampening: {}",
        safe_reports_with_problem_dampening
    );

    return 0;
}

int main(int argc, char** argv) {
    return App(day2).run(argc, argv);
}

void parse_reports(std::vector<Report>& reports, const std::string& input) {
    std::istringstream stream(input);
    std::string line;

    while (std::getline(stream, line)) {
        std::istringstream line_stream(line);
        Report report;
        int level;
        while (line_stream >> level) {
            report.levels.push_back(level);
        }
        if (!report.levels.empty()) {
            reports.push_back(report);
        }
    }
}
