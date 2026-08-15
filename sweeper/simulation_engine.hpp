#pragma once
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <cassert>
#include <memory>
#include <filesystem>

#include "sweeper/config.hpp"
#include "sweeper/node_data.hpp"
#include "smt-switch/solver.h"
#include "smt-switch/smt.h"
#include "simulation.h"

namespace sweeper {
void match_term_constraint_pattern(const smt::TermVec & constraints,
                                   std::unordered_map<smt::Term, std::string> & constraint_input_map);

// 供 simulation() 内部处理 extract(constraint)
inline std::string apply_extract_constraint(const std::string & base_value,
                                            int high, int low,
                                            const std::string & extract_value)
{
    std::string result = base_value;
    int total_width = static_cast<int>(base_value.size());
    int extract_width = high - low + 1;
    std::string padded_extract = extract_value;
    if (extract_value.size() < static_cast<size_t>(extract_width)) {
        padded_extract = std::string(extract_width - extract_value.size(), '0') + extract_value;
    }
    for (int i = 0; i < extract_width; ++i) {
        int pos = total_width - 1 - low - i;
        if (pos >= 0 && pos < total_width)
            result[pos] = padded_extract[extract_width - 1 - i];
    }
    return result;
}

// 按行加载已经 dump 的输入向量
template <typename TermIterable>
inline void load_simulation_input(const std::string & path,
                                  const TermIterable & input_terms,
                                  std::unordered_map<smt::Term, NodeData> & node_data_map)
{
    std::ifstream infile(path);
    if (!infile.is_open()) {
        std::cerr << "[ERROR] Cannot open simulation input file: " << path << std::endl;
        return;
    }
    std::string line;
    while (std::getline(infile, line)) {
        if (line.empty() || line[0] == '[') continue;
        size_t pos = line.find(" = ");
        if (pos == std::string::npos) continue;
        std::string term_str = line.substr(0, pos);
        std::string val_str  = line.substr(pos + 3);
        for (const auto & term : input_terms) {
            if (term->to_string() == term_str) {
                node_data_map[term].get_simulation_data()
                    .push_back(btor_bv_const(val_str.c_str(), term->get_sort()->get_width()));
            }
        }
    }
}


template <typename TermIterable>
inline void simulation(const TermIterable & input_terms,
                       int num_iterations,
                       std::unordered_map<smt::Term, NodeData> & node_data_map,
                       std::string & dump_file_path,
                       std::string & load_file_path,
                       const smt::TermVec & constraints = {})
{
    using namespace smt;
    using sweeper::GmpRandStateGuard;

    GmpRandStateGuard rand_guard;
    std::unordered_map<Term, std::string> constraint_input_map;
    match_term_constraint_pattern(constraints, constraint_input_map);//FIXME

    std::ofstream dumpfile;
    if (!load_file_path.empty()) {
        // 直接从文件读取并填充
        std::unordered_map<std::string, Term> term_lookup;
        for (const auto & term : input_terms)
            term_lookup[term->to_string()] = term;

        std::ifstream infile(load_file_path);
        if (!infile.is_open()) {
            std::cerr << "[ERROR] Cannot open simulation input file: " << load_file_path << std::endl;
            return;
        }
        std::string line;
        while (std::getline(infile, line)) {
            if (line.empty() || line[0] == '[') continue;
            size_t pos = line.find(" = ");
            if (pos == std::string::npos) continue;
            std::string term_str = line.substr(0, pos);
            std::string val_str  = line.substr(pos + 3);

            auto it = term_lookup.find(term_str);
            if (it != term_lookup.end()) {
                node_data_map[it->second].get_simulation_data()
                    .push_back(btor_bv_const(val_str.c_str(), it->second->get_sort()->get_width()));
            }
        }
        std::cout << "[Simulation] Loaded input simulation values from: " << load_file_path << std::endl;
        return;
    }

    if (!dump_file_path.empty()) {
        dumpfile.open(dump_file_path, std::ios::app);
        if (!dumpfile.is_open()) {
            std::cerr << "[ERROR] Cannot open dump file: " << dump_file_path << std::endl;
            return;
        }
        dumpfile << "[BEGIN NEW SIMULATION BATCH]\n";
    }

    for (int i = 0; i < num_iterations; ++i) {
        if (dumpfile.is_open()) dumpfile << "[SIMULATION ITERATION " << i << "]\n";
        for (const auto & term : input_terms) {
            std::string value_str;
            // 命中约束：支持 Extract 子段固定
            if (auto it = constraint_input_map.find(term); it != constraint_input_map.end()) {
                value_str = it->second;
                if (value_str.rfind("EXTRACT_", 0) == 0) {
                    std::string base(term->get_sort()->get_width(), '0');
                    size_t eq_pos = value_str.find('=');
                    std::string tag = value_str.substr(0, eq_pos);
                    std::string extract_val = value_str.substr(eq_pos + 1);
                    size_t hpos = tag.find('_', 8);
                    int high = std::stoi(tag.substr(8, hpos - 8));
                    int low  = std::stoi(tag.substr(hpos + 1));
                    value_str = apply_extract_constraint(base, high, low, extract_val);
                }
            } else {
                // 随机生成
                const auto width = term->get_sort()->get_width();
                mpz_t input_mpz;
                rand_guard.random_input(input_mpz, width);
                std::unique_ptr<char, void (*)(void *)> input_str(mpz_get_str(nullptr, 2, input_mpz), free);
                mpz_clear(input_mpz);
                value_str = input_str.get();
            }

            if (value_str.size() < term->get_sort()->get_width()) {
                size_t pad_len = term->get_sort()->get_width() - value_str.size();
                value_str = std::string(pad_len, '0') + value_str;
            }

            node_data_map[term].get_simulation_data()
                .push_back(btor_bv_const(value_str.c_str(), term->get_sort()->get_width()));
            if (dumpfile.is_open()) dumpfile << term->to_string() << " = " << value_str << "\n";
        }
        if (dumpfile.is_open()) dumpfile << "\n";
    }

    if (dumpfile.is_open()) {
        dumpfile << "[END SIMULATION BATCH]\n";
        dumpfile.close();
        std::cout << "[Simulation] Dumped input simulation values to: " << dump_file_path << std::endl;
    }
}

} // namespace sweeper
