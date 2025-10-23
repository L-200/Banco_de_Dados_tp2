#include <iostream>
#include <string>
#include <stdexcept>
#include <fstream>
#include <sstream>       // para reconstruir o título com espaços
#include <functional>    // para std::hash

#include "record.hpp"
#include "BPlusTree_long.hpp" // índice secundário por hash de título

// -----------------------------------------------------------
// Função auxiliar: imprime o conteúdo de um artigo formatado
// -----------------------------------------------------------
void print_artigo(const Artigo& artigo) {
    std::cout << "------------------------------------------\n";
    std::cout << "ID: " << artigo.ID << "\n";
    std::cout << "Titulo: " << artigo.Titulo << "\n";
    std::cout << "Ano: " << artigo.Ano << "\n";
    std::cout << "Autores: " << artigo.Autores << "\n";
    std::cout << "Citacoes: " << artigo.Citacoes << "\n";
    std::cout << "Atualizacao: " << artigo.Atualizacao_timestamp << "\n";
    std::cout << "Snippet: " << artigo.Snippet << "\n";
    std::cout << "------------------------------------------\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Uso: " << argv[0] << " <Titulo do artigo>\n";
        std::cerr << "Dica: Se o titulo contiver espacos, coloque-o entre aspas ou digite sem aspas que o programa trata.\n";
        return 1;
    }

    // Reconstrói o título completo a partir dos argumentos
    std::ostringstream oss;
    for (int i = 1; i < argc; ++i) {
        if (i > 1) oss << " ";
        oss << argv[i];
    }
    std::string search_title = oss.str();

    const std::string secondary_index_path = "/data/secondary_index.idx";
    const std::string data_file_path = "/data/data_file.dat";

    try {
        // -----------------------------------------------------------
        // 1. Calcula o hash do título (chave para o índice secundário)
        // -----------------------------------------------------------
        long long search_hash = BPlusTree_long::hash_string_to_long(search_title.c_str());
        std::cout << "Buscando pelo Titulo: \"" << search_title << "\"\n";
        std::cout << "Hash gerado: " << search_hash << "\n\n";

        // -----------------------------------------------------------
        // 2. Abre a árvore B+ do índice secundário e busca a chave
        // -----------------------------------------------------------
        BPlusTree_long secondary_index(secondary_index_path);
        int blocks_read_index = 0;

        f_ptr data_ptr = secondary_index.search(search_hash, blocks_read_index);

        bool truly_found = false;

        // -----------------------------------------------------------
        // 3. Se encontrou posição no índice, abre o arquivo de dados
        //    e verifica se é realmente o artigo (evita colisão de hash)
        // -----------------------------------------------------------
        if (data_ptr != -1) {
            std::ifstream data_file(data_file_path, std::ios::binary);
            if (!data_file.is_open()) {
                throw std::runtime_error("Nao foi possivel abrir o arquivo de dados: " + data_file_path);
            }

            data_file.seekg(data_ptr);
            Artigo found_artigo;

            if (!data_file.read(reinterpret_cast<char*>(&found_artigo), sizeof(Artigo))) {
                throw std::runtime_error("Erro ao ler registro no offset " + std::to_string(data_ptr));
            }

            if (search_title == found_artigo.Titulo) {
                truly_found = true;
                std::cout << " Registro encontrado com sucesso!\n";
                print_artigo(found_artigo);
            } else {
                std::cout << "  Colisao de hash detectada.\n";
                std::cout << "O registro recuperado nao corresponde ao titulo buscado.\n";
            }
        }

        // -----------------------------------------------------------
        // 4. Caso não encontrado ou colisão
        // -----------------------------------------------------------
        if (!truly_found) {
            std::cout << " Registro com o titulo \"" << search_title << "\" nao foi encontrado.\n";
        }

        // -----------------------------------------------------------
        // 5. Métricas de desempenho da busca
        // -----------------------------------------------------------
        std::cout << "\n--- Métricas da Busca no Índice Secundário ---\n";
        std::cout << "Blocos lidos no arquivo de índice: " << blocks_read_index << "\n";
        std::cout << "Total de blocos no arquivo de índice secundário: "
                  << secondary_index.get_total_blocks() << "\n";

    } catch (const std::runtime_error& e) {
        std::cerr << "ERRO FATAL: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}