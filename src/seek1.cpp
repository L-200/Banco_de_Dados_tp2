#include <iostream>
#include <string>
#include <stdexcept>
#include <fstream>
#include <cstdlib> // Para std::stoi em C++11 ou posterior

#include "record.hpp"
#include "BPlusTree.hpp"

// Função auxiliar para imprimir os campos de um artigo
void print_artigo(const Artigo& artigo) {
    std::cout << "------------------------------------------" << std::endl;
    std::cout << "ID: " << artigo.ID << std::endl;
    std::cout << "Titulo: " << artigo.Titulo << std::endl;
    std::cout << "Ano: " << artigo.Ano << std::endl;
    std::cout << "Autores: " << artigo.Autores << std::endl;
    std::cout << "Citacoes: " << artigo.Citacoes << std::endl;
    std::cout << "Atualização: " << artigo.Atualizacao_timestamp << std::endl; // Assumindo timestamp
    std::cout << "Snippet: " << artigo.Snippet << std::endl;
    std::cout << "------------------------------------------" << std::endl;
}

int main(int argc, char* argv[]) {
    // 1. Validação dos argumentos
    if (argc != 2) {
        std::cerr << "Uso: " << argv[0] << " <ID_do_artigo>" << std::endl;
        return 1;
    }

    int search_id;
    try {
        search_id = std::stoi(argv[1]);
    } catch (const std::exception& e) {
        std::cerr << "ERRO: O ID informado ('" << argv[1] << "') nao e um numero valido." << std::endl;
        return 1;
    }

    // --- CORREÇÃO AQUI ---
    // Usar caminhos absolutos dentro do container Docker
    const std::string primary_index_path = "/data/primary_index.idx";
    const std::string data_file_path = "/data/data_file.dat";
    // ---------------------

    std::cout << "Buscando pelo ID no indice primario: " << search_id << std::endl;

    try {
        // 2. Inicializa o índice (que agora deve ABRIR o arquivo existente)
        BPlusTree primary_index(primary_index_path);
        int blocks_read_index = 0;

        // 3. Executa a busca no índice
        f_ptr data_ptr = primary_index.search(search_id, blocks_read_index);

        // 4. Verifica o resultado
        if (data_ptr != -1) {
            std::cout << "\nChave encontrada no indice! Ponteiro para dados: " << data_ptr << std::endl;
            std::cout << "Lendo registro do arquivo de dados..." << std::endl;

            // Abre o arquivo de dados para ler o registro
            std::ifstream data_file(data_file_path, std::ios::binary);
            if (!data_file) {
                 // Adicionado log mais específico
                 std::cerr << "ERRO FATAL: Nao foi possivel abrir o arquivo de dados '" << data_file_path << "' para leitura." << std::endl;
                throw std::runtime_error("Falha ao abrir arquivo de dados.");
            }

            // Posiciona no local exato
            data_file.seekg(data_ptr);
            if (!data_file) { // Verifica se seekg falhou
                 data_file.close();
                 std::cerr << "ERRO FATAL: Falha ao posicionar no arquivo de dados no offset " << data_ptr << std::endl;
                 throw std::runtime_error("Falha no seekg do arquivo de dados.");
            }


            Artigo found_artigo;
            // Lê o registro
            if (!data_file.read(reinterpret_cast<char*>(&found_artigo), sizeof(Artigo))) {
                 data_file.close();
                 std::cerr << "ERRO FATAL: Falha ao ler o registro do arquivo de dados no offset " << data_ptr << std::endl;
                 std::cerr << "  -> Verifique se o data_ptr esta correto e se o arquivo de dados nao esta corrompido." << std::endl;
                 throw std::runtime_error("Falha na leitura do arquivo de dados.");
            }
            data_file.close();

            std::cout << "\nRegistro encontrado com sucesso!" << std::endl;
            print_artigo(found_artigo);
        } else {
            std::cout << "\nRegistro com ID " << search_id << " nao foi encontrado no indice." << std::endl;
        }

        // 5. Exibe as métricas
        std::cout << "\n--- Metricas da Busca no Indice Primario ---" << std::endl;
        std::cout << "Blocos lidos no arquivo de indice: " << blocks_read_index << std::endl;
        // Adicionado try-catch em volta de get_total_blocks para segurança
        try {
             std::cout << "Total de blocos no arquivo de indice primario: " << primary_index.get_total_blocks() << std::endl;
        } catch (...) {
             std::cerr << "AVISO: Nao foi possivel obter o total de blocos do indice." << std::endl;
        }


    } catch (const std::runtime_error& e) {
        std::cerr << "ERRO FATAL durante a busca: " << e.what() << std::endl;
        return 1;
    } catch (...) {
         std::cerr << "ERRO FATAL desconhecido durante a busca." << std::endl;
        return 1;
    }

    return 0;
}