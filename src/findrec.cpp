#include <iostream>
#include <string>
#include <stdexcept>
#include <cstdlib> // Para std::stoi
#include <cstring> //para auxiliar no print

#include "record.hpp"
#include "hashing.hpp"

// Função auxiliar para imprimir os campos de um artigo de forma legível
void print_artigo(const Artigo& artigo) {
    std::cout << "------------------------------------------" << std::endl;
    std::cout << "ID: " << artigo.ID << std::endl;
    
    // Usa cout.write para imprimir com segurança os arrays de char
    std::cout << "Titulo: ";
    std::cout.write(artigo.Titulo, strnlen(artigo.Titulo, 300));
    std::cout << std::endl;

    std::cout << "Ano: " << artigo.Ano << std::endl;

    std::cout << "Autores: ";
    std::cout.write(artigo.Autores, strnlen(artigo.Autores, 150));
    std::cout << std::endl;

    std::cout << "Citacoes: " << artigo.Citacoes << std::endl;
    std::cout << "Atualizacao: " << artigo.Atualizacao_timestamp << std::endl;

    std::cout << "Snippet: ";
    for (size_t i = 0; i < strnlen(artigo.Snippet, 1024); ++i) {
        unsigned char c = artigo.Snippet[i];
        if (c >= 32 && c <= 126)  // caracteres ASCII imprimíveis
            std::cout << c;
        else
            std::cout << ' ';      // substitui os não-imprimíveis por espaço
    }
    std::cout << std::endl;

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
        std::cerr << "ERRO: O ID informado ('" << argv[1] << "') não é um número válido." << std::endl;
        return 1;
    }

    // --- CORREÇÃO AQUI ---
    // Usar caminho absoluto dentro do container Docker
    const std::string data_file_path = "/data/data_file.dat";
    // ---------------------

    // O número total de blocos DEVE ser idêntico ao usado no upload
    long total_blocks = 775000;

    std::cout << "Buscando pelo ID: " << search_id << std::endl;

    try {
        // 2. Inicializa o HashingFile (que deve ABRIR o arquivo existente)
        HashingFile data_file(data_file_path, total_blocks);

        int blocks_read = 0;

        // 3. Executa a busca
        Artigo found_artigo = data_file.find_by_id(search_id, blocks_read);

        // 4. Verifica o resultado
        // A função find_by_id retorna ID -1 se não encontrar
        if (found_artigo.ID != -1) {
            std::cout << "\nRegistro encontrado com sucesso!" << std::endl;
            print_artigo(found_artigo);
            std::cout.flush();

            std::cout << "\n--- Métricas da Busca ---" << std::endl;
            std::cout << "Blocos lidos para encontrar o registro: " << blocks_read << std::endl;
            std::cout << "Total de blocos no arquivo de dados: " << total_blocks << std::endl;
        } else {
            std::cout << "\nRegistro com ID " << search_id << " nao foi encontrado." << std::endl;
            std::cout << "--- Métricas da Busca ---" << std::endl;
            std::cout << "Blocos lidos durante a tentativa: " << blocks_read << std::endl;
            std::cout << "Total de blocos no arquivo de dados: " << total_blocks << std::endl;
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