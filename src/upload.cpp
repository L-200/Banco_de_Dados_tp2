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
#include <algorithm> // Para find_first_not_of / find_last_not_of
#include <chrono>
#include <iomanip>
#include <filesystem>

// === Headers do projeto ===
#include "record.hpp"
#include "hashing.hpp"
#include "BPlusTree.hpp"
#include "BPlusTree_long.hpp"
#include "upload.hpp"
#include "log.hpp"

//quantidade de blocos
long blocks_qntd = 750000;

// Remove espaços em branco do início e fim da string (modifica in-place)
void trim(std::string& s) {
    s.erase(0, s.find_first_not_of(" \t\n\r\f\v"));
    s.erase(s.find_last_not_of(" \t\n\r\f\v") + 1);
}

// Tira os espaços extras do CSV
bool parse_csv_line(const std::string& line, Artigo& artigo) {
    if (line.empty()) return false;

    std::vector<std::string> fields;
    fields.reserve(8); // Reserva espaço para evitar realocações
    std::string current_field;
    bool in_quotes = false;
    bool field_was_quoted = false; // Flag para saber se o campo estava entre aspas

    for (size_t i = 0; i < line.length(); ++i) {
        char c = line[i];

        if (!in_quotes) {
            // FORA DAS ASPAS
            if (c == '"') {
                in_quotes = true;
                field_was_quoted = true; // Marca que este campo começou com aspas
                // Não adiciona a aspa inicial ao campo
            } else if (c == ';') {
                // Delimitador encontrado fora das aspas
                trim(current_field); // Limpa espaços do campo atual
                fields.push_back(current_field);
                current_field.clear();
                field_was_quoted = false; // Reseta a flag para o próximo campo
            } else {
                current_field += c; // Caractere normal
            }
        } else {
            // DENTRO DAS ASPAS
            if (c == '"') {
                // Verifica se é aspa dupla ("") para escape
                if (i + 1 < line.length() && line[i + 1] == '"') {
                    current_field += '"'; // Adiciona uma única aspa
                    i++; 
                } else {
                    // Aspa final do campo
                    in_quotes = false;
                }
            } else {
                current_field += c; // Caractere normal dentro das aspas
            }
        }
    }

    // Adiciona o último campo após o loop
    if (!field_was_quoted) {
        trim(current_field);
    }
    fields.push_back(current_field);

    // Verificação do número de campos
    if (fields.size() < 7) {
        LOG_WARN("Linha ignorada (campos < 7): " << line.substr(0, 100) << "...");
        return false;
    }

    // Tenta as conversões
    try {
        // 1. ID
        char* end_ptr = nullptr;
        long long temp_id = std::strtoll(fields[0].c_str(), &end_ptr, 10);
        if (end_ptr == fields[0].c_str() || *end_ptr != '\0' || temp_id > INT_MAX || temp_id < INT_MIN) {
            LOG_ERROR("Falha na conversao do ID: '" << fields[0] << "' Linha: " << line.substr(0,100) << "...");
            return false;
        }
        artigo.ID = static_cast<int>(temp_id);

        // 2. Título
        std::strncpy(artigo.Titulo, fields[1].c_str(), 300);
        artigo.Titulo[300] = '\0';
        // Rejeita títulos vazios
        if (artigo.Titulo[0] == '\0') {
            LOG_WARN("Titulo vazio encontrado no parser, artigo ID " << artigo.ID << " ignorado.");
            return false;
        }
        
        // 3. Ano
        artigo.Ano = std::atoi(fields[2].c_str());

        // 4. Autores
        std::strncpy(artigo.Autores, fields[3].c_str(), 150);
        artigo.Autores[150] = '\0';

        // 5. Citações
        artigo.Citacoes = std::atoi(fields[4].c_str());

        // 6. Atualização (simplificado)
        artigo.Atualizacao_timestamp = std::atol(fields[5].c_str());

        // 7. Snippet
        std::strncpy(artigo.Snippet, fields[6].c_str(), 1024);
        artigo.Snippet[1024] = '\0';

        return true;

    } catch (const std::exception& e) {
        LOG_ERROR("Excecao no parsing: Linha: " << line.substr(0,100) << "... Error: " << e.what());
        return false;
    } catch (...) {
        LOG_ERROR("Erro desconhecido no parsing: Linha: " << line.substr(0,100) << "...");
        return false;
    }
}


int main(int argc, char* argv[]) {

    auto start_time = std::chrono::high_resolution_clock::now();
    // Pegando o diretório de dados da variável de ambiente

    const char* data_dir_env = std::getenv("DATA_DIR");
    if (data_dir_env == nullptr) {
        LOG_ERROR("ERRO FATAL: Variavel de ambiente DATA_DIR nao definida.");
        LOG_INFO("Execute: export DATA_DIR=./data/db");
        return 1;
    }
    std::string data_dir(data_dir_env);

    // Validando o argumento de entrada (path do CSV)
    if (argc < 2) {
        LOG_ERROR("ERRO FATAL: Caminho para o .csv nao fornecido.");
        LOG_INFO("Uso: ./bin/upload <caminho_para_csv>");
        return 1;
    }
    const std::string input_csv_path = argv[1];
    std::ifstream input_file;

    try {

        std::filesystem::create_directories(data_dir);

        input_file.open(input_csv_path);
        if (!input_file.is_open()) {
            LOG_ERROR("Erro ao abrir aquivo: " + input_csv_path);
            return 1;
        }

        std::string data_file_path = data_dir + "/data_file.dat";
        std::string primary_index_path = data_dir + "/primary_index.idx";
        std::string secondary_index_path = data_dir + "/secondary_index.idx";

        HashingFile data_file(data_file_path, blocks_qntd);
        BPlusTree primary_index(primary_index_path);
        BPlusTree_long secondary_index(secondary_index_path);
        LOG_INFO("Estrutura inicializadas em: " + data_dir);

        std::string line_buffer;
        std::string complete_record_line;
        std::getline(input_file, line_buffer); 
        int inserted_count = 0;
        long physical_line_number = 1;

        while (std::getline(input_file, line_buffer)) {
            physical_line_number++;
            if (complete_record_line.empty()) {
                complete_record_line = line_buffer;
            } else {
                complete_record_line += "\n" + line_buffer;
            }

            size_t quote_count = 0;
            for (size_t i = 0; i < complete_record_line.length(); ++i) {
                if (complete_record_line[i] == '"') {
                    if (i + 1 == complete_record_line.length() || complete_record_line[i+1] != '"') {
                        quote_count++;
                    } else { i++; }
                }
            }

            if (quote_count % 2 == 0) {
                Artigo artigo; 
                // Chama a função de parsing
                if (parse_csv_line(complete_record_line, artigo)) {
                    // Rejeita títulos vazios
                    if (artigo.Titulo[0] == '\0') {
                        LOG_WARN("Aviso: Título vazio encontrado, artigo ignorado.\n");
                        continue; // Pula para a próxima linha
                    }

                    // Validação do Ano
                    if (artigo.Ano < 1000 || artigo.Ano > 2025) {
                        LOG_WARN("Aviso: Ano inválido para o artigo com ID: " << artigo.ID << "\n");
                        continue; 
                    }

                    f_ptr data_ptr = data_file.insert(artigo);
                    if (data_ptr != -1) {
                        if (inserted_count % 5000 == 0) {
                           LOG_INFO("Carregando dados... " << inserted_count << " artigos processados até agora.\n");
                        }
                        primary_index.insert(artigo.ID, data_ptr);
                        long long titulo_hash = BPlusTree_long::hash_string_to_long(artigo.Titulo);
                        secondary_index.insert(titulo_hash, data_ptr);
                        inserted_count++;
                    } else {
                        LOG_WARN("AVISO: Falha ao inserir artigo. ID: " << artigo.ID << "\n");
                    }
                } else {
                    LOG_WARN("Aviso: A linha " << physical_line_number << " foi ignorada. Título vazio ou inválido: " << complete_record_line.substr(0,100) << "...\n");
                }
                complete_record_line.clear();
            }
        }

        input_file.close();

        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = end_time - start_time;
        std::chrono::duration<double, std::milli> duration_ms_fp = duration;
        auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        LOG_INFO("Tempo de execucao do upload: "
                        << std::fixed << std::setprecision(3) // mostra 3 casas decimais
                        << duration_ms_fp.count() << " ms");

    } catch (const std::runtime_error& e) {
        LOG_ERROR("ERRO FATAL: " << e.what() << "\n");
        if (input_file.is_open()) input_file.close();
        return 1;
    } catch (...) {
        LOG_ERROR("ERRO FATAL DESCONHECIDO.\n");
        if (input_file.is_open()) input_file.close();
        return 1;
    }

    return 0;
}
