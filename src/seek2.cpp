#include <iostream>
#include <string>
#include <stdexcept>
#include <fstream>
#include <functional> // Para std::hash

#include "record.hpp"
#include "BPlusTree.hpp" // Usa a BPlusTree de inteiros
#include "seek2.hpp"

// Função auxiliar para imprimir o artigo
void print_artigo(const Artigo& artigo) {
    // ... (mesma função dos outros programas)
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Uso: " << argv[0] << " <Título do artigo>" << std::endl;
        std::cerr << "Dica: Se o título contiver espaços, coloque-o entre aspas." << std::endl;
        return 1;
    }

    std::string search_title = argv[1];
    // (Opcional: lógica para juntar múltiplos argumentos se não usar aspas)

    const std::string secondary_index_path = "data/secondary_index.idx";
    const std::string data_file_path = "data/data_file.dat";

    try {
        // Calcula o hash do título de busca
        std::hash<std::string> title_hasher;
        int search_hash = static_cast<int>(title_hasher(search_title));

        // Inicializa e busca no índice secundário usando o HASH
        BPlusTree secondary_index(secondary_index_path);
        int blocks_read_index = 0;

        std::cout << "Buscando pelo título: \"" << search_title << "\" (Hash: " << search_hash << ")" << std::endl;
        f_ptr data_ptr = secondary_index.search(search_hash, blocks_read_index);

        bool truly_found = false;
        if (data_ptr != -1) {
            // Passo de verificação
            // Encontramos um registro com o hash correspondente. Agora, precisamos
            // verificar se é o artigo certo ou uma colisão.
            std::ifstream data_file(data_file_path, std::ios::binary);
            data_file.seekg(data_ptr);
            Artigo found_artigo;
            data_file.read(reinterpret_cast<char*>(&found_artigo), sizeof(Artigo));

            if (search_title == found_artigo.Titulo) {
                // Títulos correspondem! É o registro correto.
                truly_found = true;
                std::cout << "\nRegistro encontrado com sucesso!" << std::endl;
                print_artigo(found_artigo);
            } else {
                // Hash correspondeu, mas o título não, então foi uma colisão.
                // Com a B+ Tree atual, não podemos encontrar o original.
                std::cout << "\nOcorreu uma colisao de hash. O registro encontrado não corresponde ao título buscado." << std::endl;
            }
        }
        
        if (!truly_found) {
            std::cout << "\nRegistro com o título \"" << search_title << "\" não foi encontrado." << std::endl;
        }

        // Exibe as métricas
        std::cout << "\n--- Métricas da busca no índice secundário ---" << std::endl;
        std::cout << "Blocos lidos no arquivo de índice: " << blocks_read_index << std::endl;
        std::cout << "Total de blocos no arquivo de índice secundário: " << secondary_index.get_total_blocks() << std::endl;

    } catch (const std::runtime_error& e) {
        std::cerr << "ERRO FATAL: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}