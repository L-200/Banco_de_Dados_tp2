#include <iostream>
#include <string>
#include <stdexcept>
#include <fstream>
#include <cstdlib>

#include "record.hpp"
#include "BPlusTree.hpp"
#include "seek1.hpp"

// Função auxiliar para imprimir os campos de um artigo
void print_artigo(const Artigo& artigo) {
    std::cout << "------------------------------------------" << std::endl;
    std::cout << "ID: " << artigo.ID << std::endl;
    std::cout << "Titulo: " << artigo.Titulo << std::endl;
    std::cout << "Ano: " << artigo.Ano << std::endl;
    std::cout << "Autores: " << artigo.Autores << std::endl;
    std::cout << "Citacoes: " << artigo.Citacoes << std::endl;
    std::cout << "Atualizacao: " << artigo.Atualizacao_timestamp << std::endl;
    std::cout << "Snippet: " << artigo.Snippet << std::endl;
    std::cout << "------------------------------------------" << std::endl;
}

int main(int argc, char* argv[]) {
    // Validação dos argumentos da linha de comando
    if (argc != 2) {
        std::cerr << "Uso: " << argv[0] << " <ID_do_artigo>" << std::endl;
        return 1;
    }

    int search_id;
    try {
        search_id = std::stoi(argv[1]);
    } catch (const std::exception& e) {
        std::cerr << "ERRO: O ID informado ('" << argv[1] << "') não é um número válido." << std::endl;
        return 1;
    }

    const std::string primary_index_path = "data/primary_index.idx";
    const std::string data_file_path = "data/data_file.dat";

    try {
        // Inicializa o índice primário (B+ Tree)
        BPlusTree primary_index(primary_index_path);
        int blocks_read_index = 0;

        std::cout << "Buscando pelo ID no índice primário: " << search_id << std::endl;

        // Executa a busca no índice
        // A função `search` da B+ Tree retorna o ponteiro (byte offset) para o registro no arquivo de dados.
        f_ptr data_ptr = primary_index.search(search_id, blocks_read_index);

        // Verifica o resultado da busca no índice
        if (data_ptr != -1) { // -1 indica que a chave não foi encontrada
            // Se encontrou, abre o arquivo de dados para ler o registro
            std::ifstream data_file(data_file_path, std::ios::binary);
            if (!data_file) {
                throw std::runtime_error("ERRO: Não foi possível abrir o arquivo de dados " + data_file_path);
            }

            // Posiciona o cursor de leitura no local exato indicado pelo ponteiro
            data_file.seekg(data_ptr);

            Artigo found_artigo;
            data_file.read(reinterpret_cast<char*>(&found_artigo), sizeof(Artigo));
            data_file.close();

            std::cout << "\nRegistro encontrado com sucesso!" << std::endl;
            print_artigo(found_artigo);
        } else {
            std::cout << "\nRegistro com ID " << search_id << " não foi encontrado no índice." << std::endl;
        }

        // Exibe as métricas de acordo com a especificação do trabalho
        std::cout << "\n--- Métricas da busca no índice primário ---" << std::endl;
        std::cout << "Blocos lidos no arquivo de índice: " << blocks_read_index << std::endl;
        std::cout << "Total de blocos no arquivo de índice primário: " << primary_index.get_total_blocks() << std::endl;

    } catch (const std::runtime_error& e) {
        std::cerr << "ERRO FATAL: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}