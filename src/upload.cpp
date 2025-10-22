// src/upload.cpp

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstring>     // strncpy
#include <cstdlib>     // atoi, atol
#include <ctime>       // time_t
#include <regex>
#include <functional>  // hash<string>

// Headers do projeto
#include "record.hpp"         // struct Artigo
#include "hashing.hpp"        // HashingFile
#include "BPlusTree.hpp"      // BPlusTree (int ID)
#include "BPlusTree_long.hpp" // BPlusTree_long (long long hash)
#include "upload.hpp"         // se houver declarações auxiliares

// ======================
// Hash do título
// ======================
long long hash_string_to_long(const char* str) {
    std::hash<std::string> hasher;
    return static_cast<long long>(hasher(str));
}

// ======================
// Função de parsing robusta
// ======================
bool parse_csv_line_fast(const std::string& line, Artigo& artigo) {
    if (line.empty()) return false;

    std::vector<std::string> tokens;
    tokens.reserve(8);

    std::string current;
    bool inside_quotes = false;

    for (char c : line) {
        if (c == '"') {
            if (inside_quotes && !current.empty() && current.back() == '"') {
                current += c; // aspas dupla escapada
            } else {
                inside_quotes = !inside_quotes;
            }
        } else if (c == ';' && !inside_quotes) {
            tokens.push_back(current);
            current.clear();
        } else {
            current += c;
        }
    }
    tokens.push_back(current);

    if (tokens.size() < 7) return false;

    auto trim_and_unescape = [](std::string s) -> std::string {
        if (s.size() >= 2 && s.front() == '"' && s.back() == '"')
            s = s.substr(1, s.size() - 2);
        s = std::regex_replace(s, std::regex("\"\""), "\"");
        s.erase(0, s.find_first_not_of(" \t\r\n"));
        s.erase(s.find_last_not_of(" \t\r\n") + 1);
        return s;
    };

    try {
        Artigo tmp;
        tmp.ID = std::stoi(trim_and_unescape(tokens[0]));

        std::string titulo = trim_and_unescape(tokens[1]);
        std::strncpy(tmp.Titulo, titulo.c_str(), 300);
        tmp.Titulo[300] = '\0';

        tmp.Ano = std::stoi(trim_and_unescape(tokens[2]));

        std::string autores = trim_and_unescape(tokens[3]);
        std::strncpy(tmp.Autores, autores.c_str(), 150);
        tmp.Autores[150] = '\0';

        tmp.Citacoes = std::stoi(trim_and_unescape(tokens[4]));

        std::string atualizacao = trim_and_unescape(tokens[5]);
        tmp.Atualizacao_timestamp = std::atol(atualizacao.c_str());

        std::string snippet = trim_and_unescape(tokens[6]);
        std::strncpy(tmp.Snippet, snippet.c_str(), 1024);
        tmp.Snippet[1024] = '\0';

        artigo = tmp;
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "[ERRO PARSING] Linha inválida: " << e.what() << "\n";
        return false;
    }
}

// ======================
// Função principal
// ======================
int main(int argc, char* argv[]) {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::cout.tie(nullptr);

    if (argc != 2) {
        std::cerr << "Uso: " << argv[0] << " <caminho_para_o_arquivo_csv>\n";
        return 1;
    }

    const std::string input_csv_path = argv[1];
    std::cout << "--- INÍCIO DA CARGA DE DADOS ---\n";
    std::cout << "Arquivo de entrada: " << input_csv_path << "\n";

    std::ifstream input_file(input_csv_path);
    if (!input_file.is_open()) {
        std::cerr << "ERRO: Não foi possível abrir o arquivo CSV: " << input_csv_path << "\n";
        return 1;
    }

    long initial_blocks = 650000; //numero maior do que precisa para evitar desemepnho ruim do upload (por conta das colisões)

    try {
        HashingFile data_file("/data/data_file.dat", initial_blocks);
        BPlusTree primary_index("/data/primary_index.idx");
        BPlusTree_long secondary_index("/data/secondary_index.idx");
        std::cout << "Estruturas inicializadas.\n";

        std::string line_buffer;
        std::string complete_record_line;
        std::getline(input_file, line_buffer); // cabeçalho
        int inserted_count = 0;
        long physical_line_number = 1;

        while (std::getline(input_file, line_buffer)) {
            physical_line_number++;

            if (complete_record_line.empty())
                complete_record_line = line_buffer;
            else
                complete_record_line += "\n" + line_buffer;

            // Conta aspas não escapadas
            size_t quote_count = 0;
            for (size_t i = 0; i < complete_record_line.size(); ++i) {
                if (complete_record_line[i] == '"') {
                    if (i + 1 < complete_record_line.size() && complete_record_line[i + 1] == '"') {
                        i++;
                    } else {
                        quote_count++;
                    }
                }
            }

            if (quote_count % 2 == 0) { // linha completa
                Artigo artigo;
                if (parse_csv_line_fast(complete_record_line, artigo)) {
                    f_ptr data_ptr = data_file.insert(artigo);
                    if (data_ptr != -1) {
                        primary_index.insert(artigo.ID, data_ptr);
                        long long titulo_hash = hash_string_to_long(artigo.Titulo);
                        secondary_index.insert(titulo_hash, data_ptr);
                        inserted_count++;
                    } else {
                        std::cerr << "AVISO: Falha ao inserir artigo. ID: " << artigo.ID << "\n";
                    }
                } else {
                    std::cerr << "AVISO: Linha ignorada (~" << physical_line_number 
                              << "): " << complete_record_line.substr(0,100) << "...\n";
                }
                complete_record_line.clear();
            }
        }

        input_file.close();

        std::cout << "--- CARGA DE DADOS CONCLUÍDA ---\n";
        std::cout << "Total de linhas físicas lidas: " << physical_line_number - 1 << "\n";
        std::cout << "Total de artigos inseridos: " << inserted_count << "\n";

    } catch (const std::runtime_error& e) {
        std::cerr << "ERRO FATAL: " << e.what() << "\n";
        if (input_file.is_open()) input_file.close();
        return 1;
    } catch (...) {
        std::cerr << "ERRO FATAL DESCONHECIDO.\n";
        if (input_file.is_open()) input_file.close();
        return 1;
    }

    return 0;
}