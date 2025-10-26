#include <iostream>
#include <string>
#include <stdexcept>
#include <cstdlib> // Para std::stoi
#include <cstring> //para auxiliar no print
#include <chrono>
#include <iomanip>

#include "record.hpp"
#include "hashing.hpp"
#include "log.hpp"
#include "findrec.hpp"

//quantidade de blocos
long blocks_qntd = 750000;

// Função auxiliar para imprimir os campos de um artigo de forma legível
//não precisa de log
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

    auto start_time = std::chrono::high_resolution_clock::now();
    // 1. Validação dos argumentos
    if (argc != 2) {
        LOG_ERROR("Uso: " << argv[0] << " <ID_do_artigo>");
        return 1;
    }

    int search_id;
    try {
        search_id = std::stoi(argv[1]);
    } catch (const std::exception& e) {
        LOG_ERROR("ERRO: O ID informado ('" << argv[1] << "') não é um número válido.");
        return 1;
    }
    
    const std::string data_file_path = "/data/data_file.dat";

   LOG_INFO("Buscando pelo ID: " << search_id);

    try {
        // 2. Inicializa o HashingFile (que deve ABRIR o arquivo existente)
        HashingFile data_file(data_file_path, blocks_qntd);

        int blocks_read = 0;

        // 3. Executa a busca
        Artigo found_artigo = data_file.find_by_id(search_id, blocks_read);

        // 4. Verifica o resultado
        // A função find_by_id retorna ID -1 se não encontrar
        if (found_artigo.ID != -1) {
           LOG_INFO("\nRegistro encontrado com sucesso!");
            print_artigo(found_artigo);
            std::cout.flush();

           LOG_INFO("\n--- Métricas da Busca ---");
           LOG_INFO("Blocos lidos para encontrar o registro: " << blocks_read);
           LOG_INFO("Total de blocos no arquivo de dados: " << blocks_qntd);
        } else {
           LOG_INFO("\nRegistro com ID " << search_id << " nao foi encontrado.");
           LOG_INFO("--- Métricas da Busca ---");
           LOG_INFO("Blocos lidos durante a tentativa: " << blocks_read);
           LOG_INFO("Total de blocos no arquivo de dados: " << blocks_qntd);
        }

        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = end_time - start_time;
        std::chrono::duration<double, std::milli> duration_ms_fp = duration;
        auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        LOG_INFO("Tempo de execucao do findrec: "
                        << std::fixed << std::setprecision(3) // mostra 3 casas decimais
                        << duration_ms_fp.count() << " ms");

    } catch (const std::runtime_error& e) {
        LOG_ERROR("ERRO FATAL durante a busca: " << e.what());
        return 1;
    } catch (...) {
         LOG_ERROR("ERRO FATAL desconhecido durante a busca.");
        return 1;
    }

    return 0;
}