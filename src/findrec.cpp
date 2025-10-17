#include <iostream>
#include <string>
#include <stdexcept>
#include <cstdlib>

#include "record.hpp"
#include "hashing.hpp"

// Função auxiliar para imprimir os campos de um artigo de forma legível
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
    // 1. Validação dos argumentos de linha de comando
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

    // ATENÇÃO: O número total de blocos deve ser o mesmo usado pelo programa 'upload'.
    long total_blocks = 1000; 
    const std::string data_file_path = "data/data_file.dat";

    try {
        // 2. Inicializa o arquivo de dados
        HashingFile data_file(data_file_path, total_blocks);
        
        int blocks_read = 0;

        std::cout << "Buscando pelo ID: " << search_id << std::endl;

        // 3. Executa a busca
        Artigo found_artigo = data_file.find_by_id(search_id, blocks_read);

        // 4. Verifica o resultado e exibe as informações
        // A função find_by_id retorna um artigo com ID 0 se não encontrar.
        if (found_artigo.ID == search_id && search_id != 0) {
            std::cout << "\nRegistro encontrado com sucesso!" << std::endl;
            print_artigo(found_artigo);

            std::cout << "--- Métricas da Busca ---" << std::endl;
            std::cout << "Blocos lidos para encontrar o registro: " << blocks_read << std::endl;
            std::cout << "Total de blocos no arquivo de dados: " << total_blocks << std::endl;
        } else {
            std::cout << "\nRegistro com ID " << search_id << " nao foi encontrado." << std::endl;
            std::cout << "--- Métricas da Busca ---" << std::endl;
            std::cout << "Blocos lidos durante a tentativa: " << blocks_read << std::endl;
            std::cout << "Total de blocos no arquivo de dados: " << total_blocks << std::endl;
        }

    } catch (const std::runtime_error& e) {
        std::cerr << "ERRO FATAL: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}