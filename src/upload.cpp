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

// === Headers do seu projeto ===
#include "record.hpp"
#include "hashing.hpp"
#include "BPlusTree.hpp"
#include "BPlusTree_long.hpp"
#include "upload.hpp"


// === Função Auxiliar Simples para Trim ===
// Remove espaços em branco do início e fim da string (modifica in-place)
void trim(std::string& s) {
    s.erase(0, s.find_first_not_of(" \t\n\r\f\v"));
    s.erase(s.find_last_not_of(" \t\n\r\f\v") + 1);
}


// === Função de Parsing Otimizada ===
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
                    i++; // Pula a próxima aspa
                } else {
                    // Aspa final do campo
                    in_quotes = false;
                    // Não adiciona a aspa final ao campo
                }
            } else {
                current_field += c; // Caractere normal dentro das aspas
            }
        }
    }

    // Adiciona o último campo após o loop
    // Se o último campo estava entre aspas, ele já foi tratado.
    // Se não estava, precisa de trim. Se estava, não faz trim (pode ter espaços internos).
    if (!field_was_quoted) {
        trim(current_field);
    }
    fields.push_back(current_field);

    // Verificação do número de campos
    if (fields.size() < 7) {
        // std::cerr << "Linha ignorada (campos < 7): " << line << std::endl;
        return false;
    }

    // Tenta as conversões (mantendo a verificação robusta do ID)
    try {
        // 1. ID
        char* end_ptr = nullptr;
        long long temp_id = std::strtoll(fields[0].c_str(), &end_ptr, 10);
        if (end_ptr == fields[0].c_str() || *end_ptr != '\0' || temp_id > INT_MAX || temp_id < INT_MIN) {
             std::cerr << "ERRO DE CONVERSAO ID: '" << fields[0] << "' Linha: " << line.substr(0,100) << "..." << std::endl;
             return false;
        }
        artigo.ID = static_cast<int>(temp_id);

        // 2. Título
        std::strncpy(artigo.Titulo, fields[1].c_str(), 300);
        artigo.Titulo[300] = '\0';

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
        std::cerr << "ERRO DE EXCECAO NO PARSING: Linha: " << line.substr(0,100) << "... Error: " << e.what() << std::endl;
        return false;
    } catch (...) {
        std::cerr << "ERRO DESCONHECIDO NO PARSING: Linha: " << line.substr(0,100) << "..." << std::endl;
        return false;
    }
}


// === Função Principal do Programa `upload` (sem alterações na lógica principal) ===
int main(int argc, char* argv[]) {
    // ... (otimizações de I/O, verificação de args, abrir CSV) ...
    const std::string input_csv_path = argv[1];
    std::ifstream input_file(input_csv_path);
    if (!input_file.is_open()) { /* ... erro ... */ }

    long initial_blocks = 750000; // Valor ajustado para teste

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
            if (complete_record_line.empty()) {
                complete_record_line = line_buffer;
            } else {
                complete_record_line += "\n" + line_buffer;
            }

             size_t quote_count = 0;
             for(size_t i = 0; i < complete_record_line.length(); ++i) {
                  if (complete_record_line[i] == '"') {
                       if (i + 1 == complete_record_line.length() || complete_record_line[i+1] != '"') {
                            quote_count++;
                       } else { i++; }
                  }
             }

            if (quote_count % 2 == 0) {
                Artigo artigo; // Renomeado de new_artigo para evitar conflito com struct Artigo
                // Chama a NOVA função de parsing
                if (parse_csv_line(complete_record_line, artigo)) {
                    f_ptr data_ptr = data_file.insert(artigo);
                    if (data_ptr != -1) {
                        if (inserted_count % 5000 == 0) {
                            std::cout<< "Inserindo item numero :"<< inserted_count << "\n";
                        } 
                        primary_index.insert(artigo.ID, data_ptr);
                        long long titulo_hash = BPlusTree_long::hash_string_to_long(artigo.Titulo);
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