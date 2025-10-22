// src/upload.cpp — VERSÃO OTIMIZADA

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include <climits>
#include <functional>
#include <string_view>
#include <algorithm>

// === Headers do Projeto ===
#include "record.hpp"
#include "hashing.hpp"
#include "BPlusTree.hpp"
#include "BPlusTree_long.hpp"
#include "upload.hpp"

// === Hash do Título com string_view (sem cópias) ===
inline long long hash_string_to_long(const char* str) {
    static std::hash<std::string_view> hasher;
    return static_cast<long long>(hasher(std::string_view(str)));
}

// === Parsing Rápido de CSV ===
// Assume separador ';' e respeita aspas — sem realocação de strings
bool parse_csv_line_fast(std::string& line, Artigo& artigo) {
    if (line.empty()) return false;

    const char* ptr = line.c_str();
    const char* end = ptr + line.size();
    const char* field_starts[7] = { ptr };
    const char* field_ends[7] = { nullptr };

    bool in_quotes = false;
    int field = 0;
    for (const char* p = ptr; p < end && field < 7; ++p) {
        if (*p == '"') {
            if (p + 1 < end && p[1] == '"') ++p; // aspas duplas escapadas
            else in_quotes = !in_quotes;
        } else if (*p == ';' && !in_quotes) {
            field_ends[field] = p;
            if (++field < 7) field_starts[field] = p + 1;
        }
    }
    if (field < 6) return false;
    field_ends[6] = end;

    // Função auxiliar para limpar espaços
    auto trim = [](std::string_view sv) {
        size_t start = sv.find_first_not_of(" \t\n\r\f\v");
        size_t end = sv.find_last_not_of(" \t\n\r\f\v");
        if (start == std::string_view::npos) return std::string_view{};
        return sv.substr(start, end - start + 1);
    };

    try {
        std::string_view id_str(field_starts[0], field_ends[0] - field_starts[0]);
        id_str = trim(id_str);
        char* endptr = nullptr;
        long long temp_id = std::strtoll(std::string(id_str).c_str(), &endptr, 10);
        if (endptr == id_str.data() || *endptr != '\0' || temp_id > INT_MAX || temp_id < INT_MIN)
            return false;
        artigo.ID = static_cast<int>(temp_id);

        auto copy_field = [&](char* dest, size_t max_len, int idx) {
            std::string_view sv = trim(std::string_view(field_starts[idx], field_ends[idx] - field_starts[idx]));
            size_t len = std::min(max_len, sv.size());
            std::memcpy(dest, sv.data(), len);
            dest[len] = '\0';
        };

        copy_field(artigo.Titulo, 300, 1);
        artigo.Ano = std::atoi(std::string(field_starts[2], field_ends[2] - field_starts[2]).c_str());
        copy_field(artigo.Autores, 150, 3);
        artigo.Citacoes = std::atoi(std::string(field_starts[4], field_ends[4] - field_starts[4]).c_str());
        artigo.Atualizacao_timestamp = std::atol(std::string(field_starts[5], field_ends[5] - field_starts[5]).c_str());
        copy_field(artigo.Snippet, 1024, 6);

        return true;
    } catch (...) {
        return false;
    }
}

// === Função Principal ===
int main(int argc, char* argv[]) {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::cout.tie(nullptr);

    if (argc != 2) {
        std::cerr << "Uso: " << argv[0] << " <arquivo.csv>\n";
        return 1;
    }

    const std::string input_csv_path = argv[1];
    std::cout << "--- INÍCIO DA CARGA DE DADOS ---\nArquivo: " << input_csv_path << "\n";

    // Bufferização pesada do CSV (1 MB)
    std::ifstream input_file(input_csv_path, std::ios::in | std::ios::binary);
    if (!input_file.is_open()) {
        std::cerr << "Erro: não foi possível abrir " << input_csv_path << "\n";
        return 1;
    }
    static char read_buffer[1 << 20];
    input_file.rdbuf()->pubsetbuf(read_buffer, sizeof(read_buffer));

    std::ofstream log_file("/data/upload_warnings.log", std::ios::app);

    long initial_blocks = 650000;

    try {
        HashingFile data_file("/data/data_file.dat", initial_blocks);
        BPlusTree primary_index("/data/primary_index.idx");
        BPlusTree_long secondary_index("/data/secondary_index.idx");
        std::cout << "Estruturas inicializadas.\n";

        std::string line_buffer, complete_record_line;
        std::getline(input_file, line_buffer); // pular cabeçalho

        int inserted_count = 0;
        long physical_line_number = 1;

        // buffers de inserção em lote (reduz I/O)
        std::vector<std::pair<int, f_ptr>> primary_batch;
        std::vector<std::pair<long long, f_ptr>> secondary_batch;
        primary_batch.reserve(1000);
        secondary_batch.reserve(1000);

        while (std::getline(input_file, line_buffer)) {
            physical_line_number++;
            if (complete_record_line.empty()) complete_record_line = line_buffer;
            else complete_record_line += "\n" + line_buffer;

            size_t quote_count = 0;
            for (size_t i = 0; i < complete_record_line.size(); ++i)
                if (complete_record_line[i] == '"' && (i + 1 == complete_record_line.size() || complete_record_line[i + 1] != '"'))
                    quote_count++;

            if (quote_count % 2 == 0) {
                Artigo new_artigo;
                if (parse_csv_line_fast(complete_record_line, new_artigo)) {
                    f_ptr data_ptr = data_file.insert(new_artigo);
                    if (data_ptr != -1) {
                        long long titulo_hash = hash_string_to_long(new_artigo.Titulo);
                        primary_batch.emplace_back(new_artigo.ID, data_ptr);
                        secondary_batch.emplace_back(titulo_hash, data_ptr);
                        inserted_count++;
                    }
                } else {
                    log_file << "Linha ignorada (~" << physical_line_number
                             << "): " << complete_record_line.substr(0, 80) << "...\n";
                }
                complete_record_line.clear();

                // Flush em lote
                if (primary_batch.size() >= 1000) {
                    for (auto& [id, ptr] : primary_batch) primary_index.insert(id, ptr);
                    for (auto& [hash, ptr] : secondary_batch) secondary_index.insert(hash, ptr);
                    primary_batch.clear();
                    secondary_batch.clear();
                }
            }
        }

        // flush final
        for (auto& [id, ptr] : primary_batch) primary_index.insert(id, ptr);
        for (auto& [hash, ptr] : secondary_batch) secondary_index.insert(hash, ptr);

        input_file.close();
        log_file.close();

        std::cout << "--- CARGA CONCLUÍDA ---\n";
        std::cout << "Linhas lidas: " << physical_line_number - 1 << "\n";
        std::cout << "Artigos inseridos: " << inserted_count << "\n";

    } catch (const std::exception& e) {
        std::cerr << "Erro fatal: " << e.what() << "\n";
        return 1;
    }

    return 0;
}