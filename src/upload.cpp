// src/upload.cpp

#include <iostream>       // Para std::cout, std::cerr, std::endl
#include <fstream>        // Para std::ifstream
#include <sstream>        // Para std::stringstream (usado no parser antigo, agora menos)
#include <string>         // Para std::string, std::getline
#include <vector>         // Para std::vector (usado no parser novo)
#include <stdexcept>      // Para std::runtime_error
#include <cstdlib>        // Para std::atoi, std::atol, std::strtoll, exit
#include <cstring>        // Para std::strncpy
#include <climits>        // Para INT_MAX, INT_MIN
#include <functional>     // Para std::hash (usado no hash do título)

// === Incluindo os headers do seu projeto ===
#include "record.hpp"         // Define a struct Artigo
#include "hashing.hpp"        // Define a classe HashingFile
#include "BPlusTree.hpp"      // Define a classe BPlusTree (para int ID)
#include "BPlusTree_long.hpp" // Define a classe BPlusTree_long (para long long hash do título)
#include "upload.hpp"         // Se você tiver um upload.hpp com a declaração de parse_csv_line

// === Função Auxiliar para Hash do Título ===
// Converte uma string (título) em um número de 64 bits (long long)
long long hash_string_to_long(const char* str) {
    std::hash<std::string> hasher;
    // static_cast garante que o resultado seja tratado como long long
    return static_cast<long long>(hasher(str));
}

// === Função Auxiliar para Analisar uma Linha do CSV ===
// Versão robusta que lida com ';', aspas, espaços e verifica conversão do ID.
bool parse_csv_line(const std::string& line, Artigo& artigo) {
    if (line.empty()) return false;

    std::vector<std::string> fields;
    std::string current_field;
    bool in_quotes = false;
    
    // Loop manual para analisar a linha, respeitando aspas
    for (size_t i = 0; i < line.length(); ++i) {
        char c = line[i];
        if (c == '"') {
            if (in_quotes && i + 1 < line.length() && line[i+1] == '"') {
                current_field += '"'; i++;
            } else {
                in_quotes = !in_quotes;
            }
        } else if (c == ';' && !in_quotes) {
            fields.push_back(current_field);
            current_field.clear();
        } else {
            current_field += c;
        }
    }
    fields.push_back(current_field); // Adiciona o último campo

    if (fields.size() < 7) {
        // std::cerr << "Linha ignorada (campos < 7): " << line << std::endl; // Opcional: Descomente para depurar
        return false;
    }

    // Limpa espaços em branco no início/fim de cada campo
    for (std::string& f : fields) {
        f.erase(0, f.find_first_not_of(" \t\n\r\f\v"));
        f.erase(f.find_last_not_of(" \t\n\r\f\v") + 1);
    }
    
    // Tenta as conversões
    try {
        // 1. ID (com verificação robusta)
        char* end_ptr = nullptr;
        long long temp_id = std::strtoll(fields[0].c_str(), &end_ptr, 10);
        if (end_ptr == fields[0].c_str() || *end_ptr != '\0' || temp_id > INT_MAX || temp_id < INT_MIN) {
             std::cerr << "ERRO DE CONVERSAO ID: '" << fields[0] << "' Linha: " << line << std::endl;
             return false;
        }
        artigo.ID = static_cast<int>(temp_id);

        // 2. Título (copia segura com strncpy)
        std::strncpy(artigo.Titulo, fields[1].c_str(), 300);
        artigo.Titulo[300] = '\0'; // Garante terminação nula

        // 3. Ano (atoi é suficiente aqui)
        artigo.Ano = std::atoi(fields[2].c_str());

        // 4. Autores (copia segura)
        std::strncpy(artigo.Autores, fields[3].c_str(), 150);
        artigo.Autores[150] = '\0';

        // 5. Citações (atoi é suficiente)
        artigo.Citacoes = std::atoi(fields[4].c_str());

        // 6. Atualização (simplificado - usar atol)
        // ATENÇÃO: Uma conversão real de data/hora seria mais complexa.
        artigo.Atualizacao_timestamp = std::atol(fields[5].c_str());

        // 7. Snippet (copia segura)
        std::strncpy(artigo.Snippet, fields[6].c_str(), 1024);
        artigo.Snippet[1024] = '\0';

        return true; // Parsing bem-sucedido
        
    } catch (const std::exception& e) {
        std::cerr << "ERRO DE EXCECAO NO PARSING: Linha: " << line << " Error: " << e.what() << std::endl;
        return false;
    } catch (...) {
        std::cerr << "ERRO DESCONHECIDO NO PARSING: Linha: " << line << std::endl;
        return false;
    }
}


// === Função Principal do Programa `upload` ===
int main(int argc, char* argv[]) {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::cout.tie(nullptr);

    
    // 1. Verifica se o caminho do CSV foi passado como argumento
    if (argc != 2) {
        std::cerr << "Uso: " << argv[0] << " <caminho_para_o_arquivo_csv>" << std::endl;
        std::cerr << "Exemplo dentro do Docker: ./bin/upload /data/artigo.csv" << std::endl;
        return 1; // Termina com erro
    }
    const std::string input_csv_path = argv[1]; // Pega o caminho do CSV do argumento

    std::cout << "--- INÍCIO DA CARGA DE DADOS ---" << std::endl;
    std::cout << "Arquivo de entrada: " << input_csv_path << std::endl;

    // 2. Tenta abrir o arquivo CSV
    std::ifstream input_file(input_csv_path);
    if (!input_file.is_open()) {
        std::cerr << "ERRO: Não foi possível abrir o arquivo CSV: " << input_csv_path << std::endl;
        return 1; // Termina com erro
    }
    
    // 3. Define o número de blocos para o arquivo de Hashing
    //    Ajuste este valor conforme necessário (ex: 1.3 * num_artigos / RECORDS_PER_BLOCK)
    long initial_blocks = 775000; // Ajustado para o seu caso

    // 4. Bloco try...catch para lidar com erros durante a inicialização ou carga
    try {
        // --- Inicializa as Estruturas de Dados ---
        
        // A. Arquivo de Dados Principal (Hashing)
        //    Usa caminho absoluto /data/ DENTRO do container.
        HashingFile data_file("/data/data_file.dat", initial_blocks);
        std::cout << "Arquivo de dados inicializado: /data/data_file.dat" << std::endl;
        
        // B. Índice Primário (B+Tree para ID int)
        //    Usa caminho absoluto /data/
        BPlusTree primary_index("/data/primary_index.idx");
        std::cout << "\nÍndice primário inicializado: /data/primary_index.idx" << std::endl;

        // C. Índice Secundário (B+Tree para hash long long do Título)
        //    Usa caminho absoluto /data/
        BPlusTree_long secondary_index("/data/secondary_index.idx");
        std::cout << "\nÍndice secundário inicializado: /data/secondary_index.idx" << std::endl;

        // --- Loop Principal de Leitura e Inserção ---
        
        std::string line;
        
        // A. Pula a primeira linha (cabeçalho) do CSV
        std::getline(input_file, line); 

        // B. Loop para ler cada linha de dados do CSV
        int inserted_count = 0;
        int line_count = 1; // Começa na linha 2 (após o cabeçalho)
        while (std::getline(input_file, line)) {
            line_count++;
            Artigo new_artigo; // Cria um objeto Artigo vazio
            
            // Tenta analisar a linha atual do CSV
            if (parse_csv_line(line, new_artigo)) {
                // Se o parsing foi bem-sucedido:
                
                // 1. Insere o registro completo no arquivo de dados e obtém seu endereço (offset)
                f_ptr data_ptr = data_file.insert(new_artigo); 

                // Verifica se a inserção no hashing falhou (arquivo cheio)
                if (data_ptr == -1) {
                    std::cerr << "AVISO: Falha ao inserir o artigo com ID " << new_artigo.ID << " (provavelmente arquivo de dados cheio)." << std::endl;
                    continue; // Pula para a próxima linha do CSV
                }

                // 2. Insere a chave (ID int) e o endereço no índice primário
                primary_index.insert(new_artigo.ID, data_ptr);
                
                // 3. Calcula o hash do título
                long long titulo_hash = hash_string_to_long(new_artigo.Titulo);
                
                // 4. Insere a chave (hash long long) e o endereço no índice secundário
                secondary_index.insert(titulo_hash, data_ptr);
                
                inserted_count++; // Conta mais um artigo inserido com sucesso
            } else {
                 std::cerr << "AVISO: Linha " << line_count << " ignorada devido a erro de parsing." << std::endl;
            }
        }
        
        // Fecha o arquivo CSV após terminar o loop
        input_file.close();

        // Imprime um resumo da operação
        std::cout << "--- CARGA DE DADOS CONCLUÍDA ---" << std::endl;
        std::cout << "Total de linhas lidas (sem cabeçalho): " << line_count - 1 << std::endl;
        std::cout << "Total de artigos inseridos com sucesso: " << inserted_count << std::endl;

    } catch (const std::runtime_error& e) {
        // Captura erros graves (ex: falha ao criar arquivos)
        std::cerr << "ERRO FATAL DURANTE A CARGA: " << e.what() << std::endl;
        if(input_file.is_open()) input_file.close(); // Garante que o arquivo CSV seja fechado
        return 1; // Termina com erro
    } catch (...) {
        // Captura qualquer outro erro inesperado
        std::cerr << "ERRO FATAL DESCONHECIDO DURANTE A CARGA." << std::endl;
         if(input_file.is_open()) input_file.close();
        return 1;
    }
    
    // Se tudo correu bem, termina com sucesso
    return 0;
}